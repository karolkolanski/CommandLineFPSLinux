/*
	OneLoneCoder.com - Command Line First Person Shooter (FPS) Engine
	"Why were games not done like this is 1990?" - @Javidx9

	License
	~~~~~~~
	Copyright (C) 2018  Javidx9
	This program comes with ABSOLUTELY NO WARRANTY.
	This is free software, and you are welcome to redistribute it
	under certain conditions; See license for details. 
	Original works located at:
	https://www.github.com/onelonecoder
	https://www.onelonecoder.com
	https://www.youtube.com/javidx9

	GNU GPLv3
	https://github.com/OneLoneCoder/videos/blob/master/LICENSE

	From Javidx9 :)
	~~~~~~~~~~~~~~~
	Hello! Ultimately I don't care what you use this for. It's intended to be 
	educational, and perhaps to the oddly minded - a little bit of fun. 
	Please hack this, change it and use it in any way you see fit. You acknowledge 
	that I am not responsible for anything bad that happens as a result of 
	your actions. However this code is protected by GNU GPLv3, see the license in the
	github repo. This means you must attribute me if you use it. You can view this
	license here: https://github.com/OneLoneCoder/videos/blob/master/LICENSE
	Cheers!

	Background
	~~~~~~~~~~
	Whilst waiting for TheMexicanRunner to start the finale of his NesMania project,
	his Twitch stream had a counter counting down for a couple of hours until it started.
	With some time on my hands, I thought it might be fun to see what the graphical
	capabilities of the console are. Turns out, not very much, but hey, it's nice to think
	Wolfenstein could have existed a few years earlier, and in just ~200 lines of code.

	IMPORTANT!!!!
	~~~~~~~~~~~~~
	READ ME BEFORE RUNNING!!! This program expects the console dimensions to be set to 
	120 Columns by 40 Rows. I recommend a small font "Consolas" at size 16. You can do this
	by running the program, and right clicking on the console title bar, and specifying 
	the properties. You can also choose to default to them in the future.
	
	Controls: A = Turn Left, D = Turn Right, W = Walk Forwards, S = Walk Backwards

	Future Modifications
	~~~~~~~~~~~~~~~~~~~~
	1) Shade block segments based on angle from player, i.e. less light reflected off
	walls at side of player. Walls straight on are brightest.
	2) Find an interesting and optimised ray-tracing method. I'm sure one must exist
	to more optimally search the map space
	3) Add bullets!
	4) Add bad guys!

	Author
	~~~~~~
	Twitter: @javidx9
	
	Implmentation on LINUX using ncurses based on Zarca modification of javidx9 work
*/

#include <iostream>
#include <utility>
#include<vector>
#include <algorithm>
#include <chrono>
#include<string>
#include <math.h>
#include<thread>
#include<unistd.h>
#include<stdlib.h>
#include <stdio.h>
#include <ncursesw/ncurses.h>
#include <locale.h>

using namespace std;
#define SHOWMAP

int nScreenWidth = 130;			// Console Screen Size X (columns) in shell MACOS
int nScreenHeight = 34;			// Console Screen Size Y (rows)    in shell MACOS 

int nMapWidth = 16;				// World Dimensions
int nMapHeight = 16;

float fPlayerX = 14.7f;			// Player Start Position
float fPlayerY = 5.09f;
float fPlayerA = 0.0f;			// Player Start Rotation
float fFOV = 3.14159f / 4.0f;	// Field of View
float fDepth = 16.0f;			// Maximum rendering distance
float fSpeed = 150.0f;			// Walking Speed
bool pressed = false;
wstring map; // the mini map 
char user_key_value = '\0'; // user input
std::thread * background_job; 

// by value not by reference to avoid race condition better
void move_player(char key_value, float fElapsedTime){
	if (!pressed){
		return;
	}
	// ACK here
	else{
		pressed = false;
	}
	// Handle CCW Rotation
	if (key_value == 'a')
		fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;

	// Handle CW Rotation
	if (key_value == 'd')
		fPlayerA += (fSpeed * 0.75f) * fElapsedTime;
	
	// Handle Forwards movement & collision
	if (key_value == 'w')
	{
		fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
		fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
		if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
		}			
	}
	// Handle backwards movement & collision
	if (key_value == 's')
	{
		fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;;
		fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;;
		if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
		{
			fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;;
			fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;;
		}
	}

}

void get_key(){
	char key_value = '\0';
	for(;;){
		/*
		std::cin >> key_value;
		if(!cin){
			continue;
		}
		*/
		user_key_value = getchar();
		//user_key_value = key_value;
		//TODO automic class
		pressed = true;
		
	}
}
void GetAsyncKeyState(){
    setlocale(LC_ALL, "");
	// working in a async way
	background_job =  new std::thread(get_key);
}

int main()
{
	// preprare for latter actions
	// preprocess();
	initscr();
	WINDOW * win = newwin(nScreenHeight, nScreenWidth, 0, 0);
    refresh();
    box(win, 0, 0);
    wrefresh(win);

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth*nScreenHeight+1];
	screen[nScreenWidth * nScreenHeight] = '\0';

	// Create Map of world space # = wall block, . = space
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";
	
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();
	// detect user's input in the background
	GetAsyncKeyState();
	
	for(;;)
	{
		// We'll need time differential per frame to calculate modification
		// to movement speeds, to ensure consistant movement, as ray-tracing
		// is non-deterministic
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		move_player(user_key_value, fElapsedTime);

		for (int x = 0; x < nScreenWidth; x++)
		{
			// For each column, calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			// Find distance to wall
			float fStepSize = 0.1f;		  // Increment size for ray casting, decrease to increase										
			float fDistanceToWall = 0.0f; //                                      resolution

			bool bHitWall = false;		// Set when ray hits wall block
			bool bBoundary = false;		// Set when ray hits boundary between two wall blocks

			float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			// Incrementally cast ray from player, along ray angle, testing for 
			// intersection with a block
			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += fStepSize;
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
				
				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;			// Just set distance to maximum depth
					fDistanceToWall = fDepth;
				}
				else
				{
					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
					{
						// Ray has hit wall
						bHitWall = true;

						// To highlight tile boundaries, cast a ray from each corner
						// of the tile, to the player. The more coincident this ray
						// is to the rendering ray, the closer we are to a tile 
						// boundary, which we'll shade to add detail to the walls
						vector<pair<float, float>> p;

						// Test each corner of hit tile, storing the distance from
						// the player, and the calculated dot product of the two rays
						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								// Angle of corner to eye
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx*vx + vy*vy); 
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

						// Sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });
						
						// First two/three are closest (we will never see all four)
						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						//if (acos(p.at(1).second) < fBound) bBoundary = true;
						//if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}
		
			// Calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			// Shader walls based on distance
			wchar_t nShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f)			nShade = L'█';	// Very close	
			else if (fDistanceToWall < fDepth / 3.0f)		nShade = L'▓';
			else if (fDistanceToWall < fDepth / 2.0f)		nShade = L'▒';
			else if (fDistanceToWall < fDepth)				nShade = L'░';
			else											nShade = ' ';		// Too far away

			if (bBoundary)		nShade = ' '; // Black it out
			
			for (int y = 0; y < nScreenHeight; y++)
			{
				// Each Row
				if(y <= nCeiling)
					screen[y*nScreenWidth + x] = ' ';
				else if(y > nCeiling && y <= nFloor)
					screen[y*nScreenWidth + x] = nShade;
				else // Floor
				{				
					// Shade floor based on distance
					float b = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[y*nScreenWidth + x] = nShade;
				}
			}
		}

		// Display Stats
		//	swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f/fElapsedTime);		
		// Display Map
		#ifdef SHOWMAP
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapHeight; ny++)
			{
				screen[(ny+1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX+1) * nScreenWidth + (int)fPlayerY] = 'P';
		#endif
		// Display Frame
		mvwaddwstr(win, 0, 0, screen);
		wrefresh(win);
	}
	return 0;
}
