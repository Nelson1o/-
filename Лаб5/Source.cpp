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
	// Объявляем структуру EXPLICIT_ACCESS для двух групп
	EXPLICIT_ACCESS ea[2];
	// Предварительно обнуляем
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

	//Получение идентификатора группы Everyone
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_CREATOR_SID_AUTHORITY;
	PSID pCurrentSID = NULL;

	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_CREATOR_OWNER_RID, 0, 0, 0, 0, 0, 0, 0, &pCurrentSID)) {
		error("AllocateAndInitializeSid Error");
	}
	// Заполнение структуры EXPLICIT_ACCESS для группы Everyone
	ea[0].grfAccessPermissions = KEY_READ;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pCurrentSID;

	//Получение идентификатора группы Администратор
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	PSID pAdminSID = NULL;

	if (!AllocateAndInitializeSid(&SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSID)) {
		FreeSid(pCurrentSID);
		error("AllocateAndInitializeSid Error");
	}
	// Заполнение структуры EXPLICIT_ACCESS для группы Администратор
	ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = (LPTSTR)pAdminSID;
	// Генерируется новый список ACL
	PACL pACL = NULL;
	DWORD dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
	if (dwRes != ERROR_SUCCESS) {
		FreeSid(pCurrentSID);
		FreeSid(pAdminSID);
		error("SetEntriesInAcl Error");
	}
	// Выделяем память под дескриптор безопасности
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL) {
		FreeSid(pCurrentSID);
		FreeSid(pAdminSID);
		LocalFree(pACL);
		error("LocalAlloc Error");
	}
	// Инициализируем дескриптор безопасности 
	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
		FreeSid(pCurrentSID);
		FreeSid(pAdminSID);
		LocalFree(pACL);
		LocalFree(pSD);
		error("InitializeSecurityDescriptor Error");
	}

	if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
		FreeSid(pCurrentSID);
		FreeSid(pAdminSID);
		LocalFree(pACL);
		LocalFree(pSD);
		error("SetSecurityDescriptorDacl Error");
	}
	// Теперь, когда дескриптор безопасности создан,
	// его можно использовать при создание файла, заполнив структуру SECURITY_ATTRIBUTES
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	HKEY hkSub;
	DWORD dwDisposition;
	// Создание реестра
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
