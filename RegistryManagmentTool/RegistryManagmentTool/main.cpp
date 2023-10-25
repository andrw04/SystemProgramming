#include "main.h"


void AddRegistryKey() {
	std::cout << "Enter key name: " << std::endl;
	std::wstring keyName;
	std::wcin >> keyName;

	std::wstring subKey = L"SOFTWARE\\" + keyName;

	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		std::cout << "Key successfully created.\n";
		RegCloseKey(hKey);
	}
	else {
		std::cout << "The registry key was not created.\n";
	}
}

void RemoveRegistryKey()
{
	std::cout << "Enter key name: ";
	std::wstring keyName;
	std::wcin >> keyName;

	std::wstring subKey = L"SOFTWARE\\" + keyName;

	// Открываем раздел реестра HKEY_CURRENT_USER
	HKEY hKey;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		std::cerr << "Unable to access registry key." << std::endl;
		return;
	}

	// Удаляем ключ из реестра
	if (RegDeleteKeyW(hKey, keyName.c_str()) != ERROR_SUCCESS)
	{
		std::cerr << "Key is not deleted" << std::endl;
		RegCloseKey(hKey);
		return;
	}

	std::cout << "Key successfully deleted." << std::endl;

	// Закрываем открытый реестровый ключ
	RegCloseKey(hKey);
}

void AddValue()
{
	HKEY hKey;

	std::cout << "Enter key name" << std::endl;
	std::wstring keyName;
	std::wcin >> keyName;

	std::wstring subKey = L"SOFTWARE\\" + keyName;

	// Открытие указанного ключа
	if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
		// Добавление параметра и его значения

		std::cout << "Enter value name: " << std::endl;
		std::wstring valueName;
		std::wcin >> valueName;

		std::cout << "Enter value: " << std::endl;
		std::wstring value;
		std::wcin >> value;

		if (RegSetValueEx(hKey, valueName.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), (wcslen(value.c_str()) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS) {
			std::cout << "Value set successfully."  << "\n";
		}
		else {
			std::cout << "Error creating value.\n";
		}
		RegCloseKey(hKey);
	}
	else {
		std::cout << "Unable to access registry key.\n";
	}

	return;
}

void RemoveValue() {
	HKEY hKey;

	std::cout << "Enter key name: " << std::endl;
	std::wstring keyName;
	std::wcin >> keyName;

	std::wstring subKey = L"SOFTWARE\\" + keyName;

	// Открытие указанного ключа
	if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
		std::cout << "Enter value name:" << std::endl;
		std::wstring valueName;
		std::wcin >> valueName;

		// Удаление параметра
		if (RegDeleteValue(hKey, valueName.c_str()) == ERROR_SUCCESS) {
			std::cout << "Value deleted successfully." << "\n";
		}
		else {
			std::cout << "Error deleting value.\n";
		}
		RegCloseKey(hKey);
	}
	else {
		std::cout << "Unable to access registry key.\n";
	}
}

void ListAllKeys() {
	HKEY hKey;
	DWORD dwIndex = 0;
	TCHAR szKeyName[MAX_PATH];
	DWORD dwKeyNameSize = MAX_PATH;

	// Открытие указанного ключа
	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		std::cout << "List of all keys in Software:\n";

		// Перебор всех подключей
		while (RegEnumKeyEx(hKey, dwIndex, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			std::wcout << szKeyName << "\n";
			dwIndex++;
			dwKeyNameSize = MAX_PATH;  // Сброс размера имени ключа
		}

		RegCloseKey(hKey);
	}
	else {
		std::cout << "Unable to access registry key.\n";
	}
}

void ListValueNamesAndValuesByKey() {
	HKEY hKey;
	DWORD dwIndex = 0;
	TCHAR szValueName[MAX_PATH];
	DWORD dwValueNameSize = MAX_PATH;
	BYTE szValueData[MAX_PATH];
	DWORD dwValueDataSize = MAX_PATH;
	DWORD dwType = 0;

	std::cout << "Enter key name" << std::endl;
	std::wstring keyName;
	std::wcin >> keyName;

	std::wstring subKey = L"SOFTWARE\\" + keyName;


	// Открытие указанного ключа
	if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		std::cout << "List of all values in key:" << ":\n";

		// Перебор всех параметров ключа
		while (RegEnumValue(hKey, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, szValueData, &dwValueDataSize) == ERROR_SUCCESS) {
			std::wcout << szValueName << ": ";

			switch (dwType) {
			case REG_SZ:
				std::wcout << (wchar_t*)szValueData << "\n";
				break;
			case REG_DWORD:
				std::wcout << *(DWORD*)szValueData << "\n";
				break;
			default:
				std::wcout << "Unsupported data type.\n";
			}

			dwIndex++;
			dwValueNameSize = MAX_PATH;  // Сброс размера имени параметра
			dwValueDataSize = MAX_PATH;  // Сброс размера данных параметра
		}

		RegCloseKey(hKey);
	}
	else {
		std::cout << "Unable to access registry key.\n";
	}
}


INT main() {
	BOOL manage_var = TRUE;
	while (manage_var) {
		printf("1 - Create Key.\n");
		printf("2 - Remove Key.\n");
		printf("3 - Create/edit Value.\n");
		printf("4 - Remove Value.\n");
		printf("5 - List all keys.\n");
		printf("6 - List all values in key.\n");

		int choice;
		if (scanf_s("%d", &choice) != 1) {
			printf("Wrong input. Try one more time.\n");
			break;
		}

		switch (choice) {
		case 1:
			AddRegistryKey();
			break;
		case 2:
			RemoveRegistryKey();
			break;
		case 3:
			AddValue();
			break;
		case 4:
			RemoveValue();
			break;
		case 5:
			ListAllKeys();
			break;
		case 6:
			ListValueNamesAndValuesByKey();
			break;
		default:
			manage_var = FALSE;
			break;
		}
	}
	getchar();
	return 0;
}