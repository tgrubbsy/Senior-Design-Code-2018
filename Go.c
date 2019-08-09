// Copyright 2018 Taylor Grubbs

/*This file is part of the The Player Illuminated Negativity Killer Source Code.

The Player Illuminated Negativity Killer Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The Player Illuminated Negativity Killer Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with The Player Illuminated Negativity Killer Source Code.  If not, see <http://www.gnu.org/licenses/>.*/

//###############################################################
//# Go.c, created by Taylor Grubbs
//# The classic chinese game of Go!
//###############################################################
#pragma once
#include "StandardGameIncludes.h"

#ifdef DEBUGCHECKS
#include "DebugFunctions.h"
#endif

// Ensures that all game functions are unique and won't generate linker errors if we switch strategies.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!
#define GAMEFUNC(FUNCNAME) GOO_##FUNCNAME

#define LIGHTSBOARDSIZE 8

#define P1COLOR GC_RED
#define P1TER GC_RED + GC_DARK
#define P2COLOR GC_BLUE
#define P2TER GC_BLUE + GC_DARK
#define OFFCOLOR GC_GRAY
#define ONCOLOR GC_WHITE
#define CAPTURED GC_YELLOW

#define P1SCORE TM_LCD_SCORE_PLAYER1
#define P2SCORE TM_LCD_SCORE_PLAYER2

#define BOARDSIZE 9

// Menu IDs
// Setup
#define MSLOT_S_STARTGAME 0
#define MSLOT_S_TT 1

// Game
#define MSLOT_G_RESTART 0
#define MSLOT_G_RECONFIGURE 1

// Gameplay Variables
static unsigned char m_bIsSetup; // 1 if setup phase, 0 if game phase.
static unsigned char m_iBoardSize;
static short timerEnable = 0;

static int visited[9][9];
static int lastPlay[2];
static short passes = 0;
static int P1Score = 0;
static int P2Score = 0;
static int turnCount = 0;

static void checkSurrounding(int x, int y, int enemy);
static short checkString(int x, int y, int prevx, int prevy, int enemy, int exitCond, short noCapture, short specialCheck);

// Game Specific Functions!  ALL OF THESE SHOULD BE DECLARED STATIC TO LIMIT THEM TO THE FILE SCOPE!
static unsigned short InitSetupPhase(unsigned short freshConfiguration)
{
	m_bIsSetup = 1;

	if (freshConfiguration) // Do initial setup stuff here that should only happen on fresh reloads.
	{
		// Create our "home" configuration.
		SetBoardSize(BOARDSIZE, BOARDSIZE);
		IlluminateBoard(OFFCOLOR);

		SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSETUP);

		timerEnable = 0;
	}
	else // Do any cleanup here to freeze the gamestate of a previous playthrough.
	{
		if (timerEnable == 1) {
			SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER1, 1);
			SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER2, 1);
		}
	}

	// Init our setup phase menu.
	ClearAllMenuOptions();

	// Use different text depending on if we've been playing this game already or not.
	if (freshConfiguration)
		RegisterMenuOption(SPT_OPTIONS_STARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);
	else
		RegisterMenuOption(SPT_OPTIONS_RESTARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);

	RegisterMenuOptionWithStringParameter(SPT_OPTIONS_TURNTIMERS, IMAGEID_NONE, timerEnable ? SPT_UTIL_ON : SPT_UTIL_OFF, MSLOT_S_TT);

	// Nothing that can cause errors here, really, so always return 0.
	return 0;
}

static void InitGamePhase()
{
	m_bIsSetup = 0;

	// Play our start que
	PlaySoundPreset(SOUNDID_GAMESTART);

	// Init our LCD display values
	SetLCDGameDisplayFormat(GDCONFIG_TWOPLAYERS); // Just one player for tileflip
	SetLCDScoreDisplayValue(TM_LCD_SCORE_PLAYER1, 0);
	SetLCDScoreDisplayValue(TM_LCD_SCORE_PLAYER2, 0);

	SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSTART);

	// Create our starting LED configuration.
	SetBoardSize(BOARDSIZE, BOARDSIZE);
	IlluminateBoard(OFFCOLOR);

	if (timerEnable) {
		SetLCDTimerValue(TM_LCD_TIMER_PLAYER1, 3600); // Timer starts at 3600 seconds
		SetLCDTimerValue(TM_LCD_TIMER_PLAYER2, 3600); // Timer starts at 3600 seconds
		SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER1, 3);
	}

	//initializing the visited array for traversal
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++)
			visited[i][j] = 0;
	}
	passes = 0;
	P1Score = 0;
	P2Score = 0;
	turnCount = 0;

	// Init our game phase menu.
	ClearAllMenuOptions();
	RegisterMenuOption(SPT_OPTIONS_RESTART, IMAGEID_NONE, MSLOT_G_RESTART);
	RegisterMenuOption(SPT_OPTIONS_RECONFIGURE, IMAGEID_NONE, MSLOT_G_RECONFIGURE);
}

static void endGame() {

	//P1 Territory
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (GetButtonColorAtPos(i, j) == P1COLOR)
				checkSurrounding(i, j, P2COLOR);
		}
	}

	//initializing the visited array for traversal
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++)
			visited[i][j] = 0;
	}

	//P2 Territory
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (GetButtonColorAtPos(i, j) == P2COLOR)
				checkSurrounding(i, j, P1COLOR);
		}
	}

	if (P1Score > P2Score) {
		SetLCDGameMessage(SPT_GAMEMESSAGE_REDVICTORY);
	}
	else if (P2Score > P1Score) {
		SetLCDGameMessage(SPT_GAMEMESSAGE_BLUEVICTORY);
	}
	else {
		SetLCDGameMessage(SPT_GAMEMESSAGE_TIEGAME);
	}
	InitSetupPhase(0);
}

static void checkSurrounding(int x, int y, int enemy) {

	if (GetButtonColorAtPos(x - 1, y) == OFFCOLOR)
		checkString(x - 1, y, 0, 0, OFFCOLOR, enemy, 0, 0);
	if (GetButtonColorAtPos(x, y + 1) == OFFCOLOR)
		checkString(x, y + 1, 0, 0, OFFCOLOR, enemy, 0, 0);
	if (GetButtonColorAtPos(x + 1, y) == OFFCOLOR)
		checkString(x + 1, y, 0, 0, OFFCOLOR, enemy, 0, 0);
	if (GetButtonColorAtPos(x, y - 1) == OFFCOLOR)
		checkString(x, y - 1, 0, 0, OFFCOLOR, enemy, 0, 0);

	return;
}

static short koRule(int x, int y, int enemy) {
	
	int color;

	switch (enemy) {
	case P1COLOR: color = P2COLOR;
	case P2COLOR: color = P1COLOR;
	}

	if (checkString(x, y, 0, 0, color, OFFCOLOR, 1, 0))
		return 1;
	else return 0;

}

static short checkString(int x, int y, int prevx, int prevy, int enemy, int exitCond, short noCapture, short specialCheck) {

	unsigned char recip;
	int score, terColor, color;

	if (exitCond == OFFCOLOR) {
		switch (enemy) {
		case P2COLOR: recip = P1SCORE; color = P1COLOR;  score = P1Score;  terColor = (GC_RED + GC_DARK);  break;
		case P1COLOR: recip = P2SCORE; color = P2COLOR;  score = P2Score;  terColor = P2TER;  break;
		}
	}
	else {
		switch (exitCond) {
		case P2COLOR: recip = P1SCORE; color = P1COLOR; score = P1Score;  terColor = (GC_RED + GC_DARK); break;
		case P1COLOR: recip = P2SCORE; color = P2COLOR; score = P2Score;  terColor = P2TER; break;
		}
	}

	short tempVisited[9][9];
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			tempVisited[i][j] = 0;
		}
	}

	if (exitCond == OFFCOLOR) {
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				visited[i][j] = 0;
			}
		}
	}

	if (noCapture)
		IlluminateButton(x, y, enemy);
	if (specialCheck)
		IlluminateButton(prevx, prevy, color);

	if (visited[x][y] == 1)
		return 0;

	int queue[81][2];
	int rear = -1;
	int front = 0;
	int queueItemCount = 0;

	//visited[x][y] = 1;
	tempVisited[x][y] = 1;
	rear++;
	queue[rear][0] = x;
	queue[rear][1] = y;
	queueItemCount++;

	while (queueItemCount != 0) {

		int tempPiece[2];
		tempPiece[0] = queue[front][0];
		tempPiece[1] = queue[front][1];
		front++;
		queueItemCount--;

		if (y > 0) {
			if (GetButtonColorAtPos(tempPiece[0], tempPiece[1] - 1) == exitCond) {
				if (noCapture)
					IlluminateButton(x, y, OFFCOLOR);
				else if (specialCheck)
					IlluminateButton(prevx, prevy, OFFCOLOR);
				return 0;
			}
			else if (GetButtonColorAtPos(tempPiece[0], tempPiece[1] - 1) == enemy && (/*visited[tempPiece[0]][tempPiece[1] - 1] == 0 || */tempVisited[tempPiece[0]][tempPiece[1] - 1] == 0)) {
				//visited[tempPiece[0]][tempPiece[1] - 1] = 1;
				tempVisited[tempPiece[0]][tempPiece[1] - 1] = 1;
				rear++;
				queue[rear][0] = tempPiece[0];
				queue[rear][1] = tempPiece[1] - 1;
				queueItemCount++;
			}
		}
		if (x < 8) {
			if (GetButtonColorAtPos(tempPiece[0] + 1, tempPiece[1]) == exitCond) {
				if (noCapture)
					IlluminateButton(x, y, OFFCOLOR);
				else if (specialCheck)
					IlluminateButton(prevx, prevy, OFFCOLOR);
				return 0;
			}
			else if (GetButtonColorAtPos(tempPiece[0] + 1, tempPiece[1]) == enemy && (/*visited[tempPiece[0] + 1][tempPiece[1]] == 0 || */tempVisited[tempPiece[0] + 1][tempPiece[1]] == 0)) {
				//visited[tempPiece[0] + 1][tempPiece[1]] = 1;
				tempVisited[tempPiece[0] + 1][tempPiece[1]] = 1;
				rear++;
				queue[rear][0] = tempPiece[0] + 1;
				queue[rear][1] = tempPiece[1];
				queueItemCount++;
			}
		}
		if (y < 8) {
			if (GetButtonColorAtPos(tempPiece[0], tempPiece[1] + 1) == exitCond) {
				if (noCapture)
					IlluminateButton(x, y, OFFCOLOR);
				else if (specialCheck)
					IlluminateButton(prevx, prevy, OFFCOLOR);
				return 0;
			}
			else if (GetButtonColorAtPos(tempPiece[0], tempPiece[1] + 1) == enemy && (/*visited[tempPiece[0]][tempPiece[1] + 1] == 0 || */tempVisited[tempPiece[0]][tempPiece[1] + 1] == 0)) {
				//visited[tempPiece[0]][tempPiece[1] + 1] = 1;
				tempVisited[tempPiece[0]][tempPiece[1] + 1] = 1;
				rear++;
				queue[rear][0] = tempPiece[0];
				queue[rear][1] = tempPiece[1] + 1;
				queueItemCount++;
			}
		}
		if (x > 0) {
			if (GetButtonColorAtPos(tempPiece[0] - 1, tempPiece[1]) == exitCond) {
				if (noCapture)
					IlluminateButton(x, y, OFFCOLOR);
				else if (specialCheck)
					IlluminateButton(prevx, prevy, OFFCOLOR);
				return 0;
			}
			else if (GetButtonColorAtPos(tempPiece[0] - 1, tempPiece[1]) == enemy && (/*visited[tempPiece[0] - 1][tempPiece[1]] == 0 || */tempVisited[tempPiece[0] - 1][tempPiece[1]] == 0)) {
				//visited[tempPiece[0] - 1][tempPiece[1]] = 1;
				tempVisited[tempPiece[0] - 1][tempPiece[1]] = 1;
				rear++;
				queue[rear][0] = tempPiece[0] - 1;
				queue[rear][1] = tempPiece[1];
				queueItemCount++;
			}
		}
	}

	if (noCapture != 1) {

		if (exitCond == OFFCOLOR) {
			//if we made it this far, the whole string is captured
			for (int i = 0; i < 9; i++) {
				for (int j = 0; j < 9; j++) {
					if (tempVisited[i][j] == 1) {
						if (i == lastPlay[0] && j == lastPlay[1]) {
							if (koRule(prevx, prevy, enemy)) {
								IlluminateButton(prevx, prevy, OFFCOLOR);
								PlaySoundPreset(SOUNDID_DENY);
								return 0;
							}
						}
						SetLCDScoreDisplayValue(recip, ++score);
						IlluminateButton(i, j, OFFCOLOR);
					}
				}
			}

			for (int i = 0; i < 9; i++) {
				for (int j = 0; j < 9; j++)
					visited[i][j] = 0;
			}

			switch (enemy) {
			case P2COLOR: P1Score = score;  break;
			case P1COLOR: P2Score = score;  break;
			}

		}
		else {
			for (int i = 0; i < 9; i++) {
				for (int j = 0; j < 9; j++) {
					if (tempVisited[i][j] == 1) {
						visited[i][j] = 1;
						IlluminateButton(i, j, terColor);
						SetLCDScoreDisplayValue(recip, ++score);
					}
				}
			}

			switch (exitCond) {
			case P2COLOR: P1Score = score; break;
			case P1COLOR: P2Score = score; break;
			}
		}
	}
	else
		IlluminateButton(x, y, OFFCOLOR);

	return 1;
}

static short selfCapture(int x, int y, int color, int enemy) {

	short flag = 0;

	if (y > 0) {
		if (GetButtonColorAtPos(x, y - 1) == OFFCOLOR)
			flag = 1;
		else if (GetButtonColorAtPos(x, y - 1) == enemy)
			checkString(x, y-1, x, y, enemy, OFFCOLOR, 0, 1);
	}
	if (x < 8) {
		if (GetButtonColorAtPos(x + 1, y) == OFFCOLOR)
			flag = 1;
		else if (GetButtonColorAtPos(x+1, y) == enemy)
			checkString(x+1, y, x, y, enemy, OFFCOLOR, 0, 1);
	}
	if (y < 8) {
		if (GetButtonColorAtPos(x, y + 1) == OFFCOLOR)
			flag = 1;
		else if (GetButtonColorAtPos(x, y + 1) == enemy)
			checkString(x, y+1, x, y, enemy, OFFCOLOR, 0, 1);
	}
	if (x > 0) {
		if (GetButtonColorAtPos(x - 1, y) == OFFCOLOR)
			flag = 1;
		else if (GetButtonColorAtPos(x-1, y) == enemy)
			checkString(x-1, y, x, y, enemy, OFFCOLOR, 0, 1);
	}

	if (flag != 1) {
		if (checkString(x, y, 0, 0, color, OFFCOLOR, 1, 0) == 1)
			return 1;
		else return 0;
	}
	else return 0;
}

static void MakeMove(int x, int y, short pass) {

	
	int playerTurn = turnCount % 2;
	int color = P1COLOR;
	int enemy = P2COLOR;
	char timerVal = TM_LCD_TIMER_PLAYER1;
	char enemyTimer = TM_LCD_TIMER_PLAYER2;

	if (pass == 1) {
		turnCount++;
		return;
	}

	switch (playerTurn) {
	case 0: color = P1COLOR; enemy = P2COLOR; timerVal = TM_LCD_TIMER_PLAYER1;  enemyTimer = TM_LCD_TIMER_PLAYER2;  break;
	case 1: color = P2COLOR; enemy = P1COLOR; timerVal = TM_LCD_TIMER_PLAYER2;  enemyTimer = TM_LCD_TIMER_PLAYER1; break;
	}

	if (GetButtonColorAtPos(x, y) == OFFCOLOR) {
		if (selfCapture(x, y, color, enemy) == 1) {
			PlaySoundPreset(SOUNDID_DENY);
			return;
		}
		else {
			IlluminateButton(x, y, color);
			lastPlay[0] = x;
			lastPlay[1] = y;
		}
	}
	else {
		PlaySoundPreset(SOUNDID_DENY);
		return;
	}

	turnCount++;
	passes = 0;
	if (timerEnable) {
		SetLCDTimerCountMode(timerVal, 1);
		SetLCDTimerCountMode(enemyTimer, 3);
	}
}

// Standard Callbacks

GF_PREFIX int GAMEFUNC(OnGameLoaded)()
{
	return InitSetupPhase(1);
}


GF_PREFIX void GAMEFUNC(OnButtonPressed)(int x, int y)
{
	if (m_bIsSetup)
		PlaySoundPreset(SOUNDID_DENY);
	else
		MakeMove(x, y, 0);
}

GF_PREFIX void GAMEFUNC(OnLCDButtonPressed)(int id)
{
	if (id == LCDB_EXTRA1) {
		if (m_bIsSetup) {
			PlaySoundPreset(SOUNDID_DENY);
			return;
		}
		passes++;
		MakeMove(0, 0, 1);
	}
	if (passes >= 2) {
		endGame();
	}
	return;
}

GF_PREFIX void GAMEFUNC(OnExit)(int reason)
{
	// Not supported yet!
}


GF_PREFIX void GAMEFUNC(OnTimerFinished)(int id)
{

}

GF_PREFIX void GAMEFUNC(OnLCDTimerHitZero)(int id)
{
	switch (id) {
	case TM_LCD_TIMER_PLAYER1:
		SetLCDGameMessage(SPT_GAMEMESSAGE_BLUEVICTORY);
		InitSetupPhase(0);
		break;
	case TM_LCD_TIMER_PLAYER2:
		SetLCDGameMessage(SPT_GAMEMESSAGE_REDVICTORY);
		InitSetupPhase(0);
		break;
	}
}

GF_PREFIX void GAMEFUNC(OnMenuOptionSelected)(int id)
{
	if (m_bIsSetup)
	{
		switch (id)
		{
			case MSLOT_S_STARTGAME: InitGamePhase(); break;
			case MSLOT_S_TT:
				timerEnable = timerEnable ? 0 : 1;

				RegisterMenuOptionWithStringParameter(SPT_OPTIONS_TURNTIMERS, IMAGEID_NONE, timerEnable ? SPT_UTIL_ON : SPT_UTIL_OFF, MSLOT_S_TT);
			break;
		}
	}
	else
	{
		switch (id)
		{
		case MSLOT_G_RESTART: InitGamePhase(); break;
		case MSLOT_G_RECONFIGURE: SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSETUP); InitSetupPhase(0); break;
		}
	}
}

// Fires when the system has not received an input for a "long time"
GF_PREFIX void GAMEFUNC(OnIdle)()
{
	PrintDebugMessage("Called OnIdle!\n");
	SetColorMode(1);
}

// Fires when the system has received an input while idle.
GF_PREFIX void GAMEFUNC(OnWake)(int reason)
{
	PrintDebugMessage("Called OnWake!\n");
	SetColorMode(0);
}


// ----------------------------------------------------------------------------
// Game Loader Function!  Be sure to give unique name (don't use GAMEFUNC) 
// and call in game.c's InitGamesList function with a unique id!  To load 
// immediately, for testing purposes, change GAME_DEFAULT_ID in game.h 
// to the ID you registered the game as.
// Function pointers should be fine as-is but the title and help text needs to
// be updated for each game.
// ----------------------------------------------------------------------------

void Go_RegisterGame(int id)
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_GO;
	BoardGameInfoList[id].descriptionPreset = SPT_GD_GO;
	BoardGameInfoList[id].gameIconID = IMAGEID_GAMEICON_TILEFLIP;
	BoardGameInfoList[id].p_OnGameLoaded = &GAMEFUNC(OnGameLoaded);
	BoardGameInfoList[id].p_OnButtonPressed = &GAMEFUNC(OnButtonPressed);
	BoardGameInfoList[id].p_OnLCDButtonPressed = &GAMEFUNC(OnLCDButtonPressed);
	BoardGameInfoList[id].p_OnTimerFinished = &GAMEFUNC(OnTimerFinished);
	BoardGameInfoList[id].p_OnLCDTimerHitZero = &GAMEFUNC(OnLCDTimerHitZero);
	BoardGameInfoList[id].p_OnIdle = &GAMEFUNC(OnIdle);
	BoardGameInfoList[id].p_OnWake = &GAMEFUNC(OnWake);
	BoardGameInfoList[id].p_OnExit = &GAMEFUNC(OnExit);
	BoardGameInfoList[id].p_OnMenuOptionSelected = &GAMEFUNC(OnMenuOptionSelected);
}