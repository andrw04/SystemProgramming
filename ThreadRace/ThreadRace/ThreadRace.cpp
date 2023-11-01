#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#include <map>
#include <utility> // pair
#include <string>
#include <conio.h>
using namespace std;


bool exitRace = false;
HANDLE mutex;

std::map<int, std::pair<int, int>> cars{
	{0, std::pair<int, int>(2, 18)},
	{1, std::pair<int, int>(14, 18)},
	{2, std::pair<int, int>(26, 18)}
};


string car[7] = { "    _    ",
				  " 0=[_]=0 ",
				  "   /T\   ",
				  "  |(o)|  ",
				  "[]=\\_/=[]",
				  "  __V__  ",
				  " '-----' "
};

void gotoxy(int x, int y) // to print in any place
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() { // hide cursor
	CONSOLE_CURSOR_INFO cursor;
	cursor.dwSize = 100;
	cursor.bVisible = false;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
}


DWORD WINAPI translate(void* data) {
	int carIndex = *((int*)data);

	while (!exitRace)
	{
		WaitForSingleObject(mutex, INFINITE);

		// Отрисовка трассы
		for (int i = 0; i < 25; i++) {
			gotoxy(0, i);
			cout << "#                                   #";
			gotoxy(12, i);
			cout << "|           |";
		}

		// отрисовка машинок
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 7; j++)
			{
				gotoxy(cars[i].first, j + cars[i].second);
				cout << car[j];
			}
		}

		// перемещаем текущую машинку
		if (cars[carIndex].second < 25 && cars[carIndex].second > 0)
			cars[carIndex].second--;

		// завершаем гонку
		if (cars[carIndex].second == 0)
		{
			system("cls");
			exitRace = true;
			cout << "Car number " << carIndex + 1 << " wins!" << endl;
		}

		ReleaseMutex(mutex);

		Sleep(50);
	}


	
	return 0;
}

int main() {
	cout << "Press space to start race..." << endl;

	while (true)
	{
		if (_kbhit())
		{
			char c = _getch();
			if (c == 32)
				break;
			else
				return 1;
		}
	}

	cout << "1..." << endl;
	Sleep(1000);
	cout << "2..." << endl;
	Sleep(1000);
	cout << "3..." << endl;
	cout << "Go go go!" << endl;
	Sleep(500);
	

	hideCursor();

	HANDLE hThread1, hThread2, hThread3;
	mutex = CreateMutex(NULL, FALSE, NULL);

	int car1 = 0, car2 = 1, car3 = 2;

	hThread1 = CreateThread(NULL, 0, translate, &car1, 0, NULL);
	hThread2 = CreateThread(NULL, 0, translate, &car2, 0, NULL);
	hThread3 = CreateThread(NULL, 0, translate, &car3, 0, NULL);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	WaitForSingleObject(hThread3, INFINITE);

	CloseHandle(mutex);
	return 0;
}

