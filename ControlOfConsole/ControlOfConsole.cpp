// ControlOfConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <wchar.h>

#define PI 3.14159265f


int nSPF = 33; //seconds per frame

const int nScreenHeight(80);
const int nScreenWidth(200);
const int nGameMapHeight(16);
const int nGameMapWidth(16);



//global functions
int toMapIndex(int x, int y) {
	return y * nGameMapWidth + x;
}

int toScreenIndex(int x, int y) {
	return y * nScreenWidth + x;
}

void clearScreen(wchar_t *screen) {
	for (int i = 0; i < nScreenHeight; i++)
	{
		for (int j = 0; j < nScreenWidth; j++)
		{
			screen[i*nScreenWidth + j] = ' ';
		}
	}
}


//classes
class Player {
private:
	float fPlayerX;
	float fPlayerY;
	float fPlayerA;
public:
	float fGetPlayerX();
	float fGetPlayerY();
	float fGetPlayerA();
	void addAngle(float angle);
	void turnH(int dir);
	void move(int dir);

	Player(float posX, float posY, float initA);
};

class rayVector {
private:
	float fPosX;
	float fPosY;
	float fDPosX;
	float fDPosY;
	float fPathLength;
	std::wstring *pGameMap;
public:
	void update();
	bool hitWall();
	float getPathLength();
	rayVector(Player player, float fStepLength, float fAngle, std::wstring *gameMap);
};

//class functions

//player
Player::Player(float posX, float posY, float initA) {
	fPlayerX = posX;
	fPlayerY = posY;
	fPlayerA = initA;
}

float Player::fGetPlayerX() {
	return fPlayerX;
}

float Player::fGetPlayerY() {
	return fPlayerY;
}

float Player::fGetPlayerA() {
	return fPlayerA;
}

void Player::addAngle(float angle) {
	fPlayerA += angle;
}

void Player::turnH(int turn) {
	if (turn == 1) {//right
		fPlayerA += PI / 90.0f;
	}
	else {
		fPlayerA += - PI / 90.0f;
	}
}

void Player::move(int dir) {
	if (dir == 1) {
		fPlayerX += 0.05*std::cosf(fPlayerA);
		fPlayerY += 0.05*std::sinf(fPlayerA);
	}
	else {
		fPlayerX += -0.05*std::cosf(fPlayerA);
		fPlayerY += -0.05*std::sinf(fPlayerA);
	}
}

//ray
rayVector::rayVector(Player player, float fStepLength, float fAngle, std::wstring *gameMap) {
	fPosX = player.fGetPlayerX();
	fPosY = player.fGetPlayerY();
	fDPosX = fStepLength * std::cosf(fAngle); //radian
	fDPosY = fStepLength * std::sinf(fAngle);
	fPathLength = 0;
	pGameMap = gameMap; 
}

void rayVector::update() {
	fPosX += fDPosX;
	fPosY += fDPosY;
	fPathLength += std::sqrt(fDPosX*fDPosX + fDPosY * fDPosY);
}

bool rayVector::hitWall() {
	int roundX = std::round(fPosX);
	int roundY = std::round(fPosY);
	std::wstring gameMap = *pGameMap;
	if (gameMap[toMapIndex(roundX, roundY)] == '#') { //the L might cause problems.
		return true;
	}
	else return false;
}

float rayVector::getPathLength() {
	return fPathLength;
}




int main()
{
	wchar_t *screen = new wchar_t[nScreenHeight*nScreenWidth];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	float fStepLenght = 0.01f;
	float fVPOV = PI / 2.0f;
	float fHPOV = PI / 2.0f;
	std::wstring gameMap; //currently 16x16, wchar_t at index i can be called by gameMap[p]

	gameMap += L"################";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#....######....#";
	gameMap += L"#....#....#....#";
	gameMap += L"#....#....#....#";
	gameMap += L"#....#....#....#";
	gameMap += L"#....#....#....#";
	gameMap += L"#....#....#....#";
	gameMap += L"#....#....#....#";
	gameMap += L"#....#....#....#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"#..............#";
	gameMap += L"################";

	Player player = Player(8.0f, 8.0f, 0.0f);
	float playerHeight = 1.80f;
	float wallHeight = 3.00f;

	//game loop
	while (true)
	{
		
		clearScreen(screen);
		
		if (_kbhit()) {
			wchar_t key = _getwch();
			if (key == L'd') {
				player.turnH(1);
			}
			else if (key == L'a'){
				player.turnH(0);
			}
			else if (key == L'w') {
				player.move(1);
			}
			else if (key == L's') {
				player.move(0);
			}
		}
		//send rays and paint wall, angle in radian

		float sendA = player.fGetPlayerA() - fHPOV / 2;
		float stepA = fHPOV / nScreenWidth;
		for (int i = 0; i < nScreenWidth; i++) {
			
			rayVector ray = rayVector(player, fStepLenght, sendA, &gameMap);

			//travel the ray untill it hits a wall;
			while (ray.getPathLength() < 16.0f) {
				if (!ray.hitWall()) {
					ray.update();
				}
				else {
					break;
				}
			}

			if (ray.hitWall()) {
				//upper half 
				
				wchar_t texture;
				float pathLength = ray.getPathLength() * std::cosf(sendA - player.fGetPlayerA()) + 0.5;
				if (pathLength >= 0.0f && pathLength < 4.0f) texture = L'\25A0';
				else if (pathLength >= 4.0f && pathLength < 8.0f) texture = L'=';
				else if (pathLength >= 8.0f && pathLength < 12.0f) texture = L'-';
				else if (pathLength >= 12.0f && pathLength <= 16.0f) texture = L'.';
					
				//int screenHeightWall

				float fWallScreenHeight = nScreenHeight * (wallHeight - playerHeight) / (2 * pathLength * std::tanf(fVPOV / 2));
				int nWallScreenHeight = std::round(fWallScreenHeight);
				if (nWallScreenHeight > nScreenHeight / 2) nWallScreenHeight = nScreenHeight / 2;
				for (int j = 0; j < nWallScreenHeight; j++) {
					screen[toScreenIndex(i, std::round(((float) nScreenHeight) / 2.0 - j))] = texture;
				}

				fWallScreenHeight = nScreenHeight * (playerHeight) / (2 * pathLength * std::tanf(fVPOV / 2));
				nWallScreenHeight = std::round(fWallScreenHeight);
				if (nWallScreenHeight > nScreenHeight / 2) nWallScreenHeight = nScreenHeight / 2;
				for (int j = 0; j < nWallScreenHeight; j++) {
					screen[toScreenIndex(i, std::round(((float)nScreenHeight) / 2.0 + j))] = texture;
				}
				//second half
			}
			sendA = sendA + stepA;
		}
		

		screen[nScreenHeight*nScreenWidth - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight*nScreenWidth, { 0,0 }, &dwBytesWritten);
		//Sleep(nSPF);


	}
	
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
