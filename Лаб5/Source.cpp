#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <stdio.h>
#include <conio.h>


void error(const char *error) {
	printf("%s\n", error);

	_getch();

	exit(1);
}

int main(int argc, char* argv[])
{
	// ��������� ��������� EXPLICIT_ACCESS ��� ���� �����
	EXPLICIT_ACCESS ea[2];
	// �������������� ��������
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

	//��������� �������������� ������ Everyone
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	PSID pEveryoneSID = NULL;

	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID)) {
		error("AllocateAndInitializeSid Error");
	}
	// ���������� ��������� EXPLICIT_ACCESS ��� ������ Everyone
	ea[0].grfAccessPermissions = KEY_READ;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

	//��������� �������������� ������ �������������
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	PSID pAdminSID = NULL;

	if (!AllocateAndInitializeSid(&SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSID)) {
		FreeSid(pEveryoneSID);
		error("AllocateAndInitializeSid Error");
	}
	// ���������� ��������� EXPLICIT_ACCESS ��� ������ �������������
	ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = (LPTSTR)pAdminSID;
	// ������������ ����� ������ ACL
	PACL pACL = NULL;
	DWORD dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
	if (dwRes != ERROR_SUCCESS) {
		FreeSid(pEveryoneSID);
		FreeSid(pAdminSID);
		error("SetEntriesInAcl Error");
	}
	// �������� ������ ��� ���������� ������������
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL) {
		FreeSid(pEveryoneSID);
		FreeSid(pAdminSID);
		LocalFree(pACL);
		error("LocalAlloc Error");
	}
	// �������������� ���������� ������������ 
	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
		FreeSid(pEveryoneSID);
		FreeSid(pAdminSID);
		LocalFree(pACL);
		LocalFree(pSD);
		error("InitializeSecurityDescriptor Error");
	}

	if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
		FreeSid(pEveryoneSID);
		FreeSid(pAdminSID);
		LocalFree(pACL);
		LocalFree(pSD);
		error("SetSecurityDescriptorDacl Error");
	}
	// ������, ����� ���������� ������������ ������,
	// ��� ����� ������������ ��� �������� �����, �������� ��������� SECURITY_ATTRIBUTES
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	HKEY hkSub;
	DWORD dwDisposition;
	// �������� �������
	LONG lRes = RegCreateKeyEx(HKEY_CURRENT_USER, ("Software\\pi"), 0, NULL, 0, KEY_READ | KEY_WRITE, &sa, &hkSub, NULL);

	printf("RegCreateKeyEx result: ");
	switch (lRes) {
	case ERROR_SUCCESS:
		printf("OK!\n");
		break;
	default:
		printf("ERROR!\n");
	}

	printf("\nFinished!");

	_getch();

	return 0;
}
