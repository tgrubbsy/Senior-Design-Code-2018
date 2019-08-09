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
//# Checkers.c, created by Taylor Grubbs
//# The classic game of chinese checkers!
//###############################################################
#pragma once
#include "StandardGameIncludes.h"

// Ensures that all game functions are unique and won't generate linker errors if we switch strategies.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!
#define GAMEFUNC(FUNCNAME) CNC_##FUNCNAME

#define LIGHTSBOARDSIZE 8

#define P1COLOR GC_RED
#define P2COLOR GC_BLUE
#define JUMPCOLOR GC_YELLOW + GC_DARK
#define OFFCOLOR GC_GRAY
#define ONCOLOR GC_WHITE
#define MOVECOLOR GC_YELLOW
#define P1CORNERCOLOR GC_WHITE
#define P2CORNERCOLOR GC_WHITE

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

static int turnCount = 0;

// Game Specific Functions!  ALL OF THESE SHOULD BE DECLARED STATIC TO LIMIT THEM TO THE FILE SCOPE!

static unsigned short InitSetupPhase(unsigned short freshConfiguration)
{
	m_bIsSetup = 1;

	if (freshConfiguration) // Do initial setup stuff here that should only happen on fresh reloads.
	{
		// Create our "home" configuration.
		SetBoardSize(LIGHTSBOARDSIZE, LIGHTSBOARDSIZE);
		IlluminateBoard(OFFCOLOR);

		timerEnable = 0;

		SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSETUP);
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
	SetBoardSize(LIGHTSBOARDSIZE, LIGHTSBOARDSIZE);
	IlluminateBoard(OFFCOLOR);

	//P1Board
	IlluminateButton(3, 7, P1COLOR);
	IlluminateButton(4, 7, P1COLOR);
	IlluminateButton(5, 7, P1COLOR);
	IlluminateButton(6, 7, P1COLOR);
	IlluminateButton(7, 7, P1COLOR);
	IlluminateButton(4, 6, P1COLOR);
	IlluminateButton(5, 6, P1COLOR);
	IlluminateButton(6, 6, P1COLOR);
	IlluminateButton(7, 6, P1COLOR);
	IlluminateButton(5, 5, P1COLOR);
	IlluminateButton(6, 5, P1COLOR);
	IlluminateButton(7, 5, P1COLOR);
	IlluminateButton(6, 4, P1COLOR);
	IlluminateButton(7, 4, P1COLOR);
	IlluminateButton(7, 3, P1COLOR);

	//P2Board
	IlluminateButton(0, 0, P2COLOR);
	IlluminateButton(1, 0, P2COLOR);
	IlluminateButton(2, 0, P2COLOR);
	IlluminateButton(3, 0, P2COLOR);
	IlluminateButton(4, 0, P2COLOR);
	IlluminateButton(0, 1, P2COLOR);
	IlluminateButton(1, 1, P2COLOR);
	IlluminateButton(2, 1, P2COLOR);
	IlluminateButton(3, 1, P2COLOR);
	IlluminateButton(0, 2, P2COLOR);
	IlluminateButton(1, 2, P2COLOR);
	IlluminateButton(2, 2, P2COLOR);
	IlluminateButton(0, 3, P2COLOR);
	IlluminateButton(1, 3, P2COLOR);
	IlluminateButton(0, 4, P2COLOR);

	if (timerEnable) {
		SetLCDTimerValue(TM_LCD_TIMER_PLAYER1, 1200); // Timer starts at 1200 seconds
		SetLCDTimerValue(TM_LCD_TIMER_PLAYER2, 1200); // Timer starts at 1200 seconds
		SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER1, 3);
	}

	turnCount = 0;

	// Init our game phase menu.
	ClearAllMenuOptions();
	RegisterMenuOption(SPT_OPTIONS_RESTART, IMAGEID_NONE, MSLOT_G_RESTART);
	RegisterMenuOption(SPT_OPTIONS_RECONFIGURE, IMAGEID_NONE, MSLOT_G_RECONFIGURE);
}

//Multipurpose function
//With eT set to 0, this function returns 1 if the piece is currently in a corner
//With eT set to 1, this function returns 1 if the piece is currently in enemy territory
static short CheckCorners(int x, int y, int color, short eT) {

	//Array of all the corner coords
	int P1Corner[15][2] = {
		{ 7, 3 },
		{ 7, 4 },
		{ 7, 5 },
		{ 7, 6 },
		{ 7, 7 },
		{ 6, 4 },
		{ 6, 5 },
		{ 6, 6 },
		{ 6, 7 },
		{ 5, 5 },
		{ 5, 6 },
		{ 5, 7 },
		{ 4, 6 },
		{ 4, 7 },
		{ 3, 7 }
	};

	int P2Corner[15][2] = {
		{ 0, 0 },
		{ 0, 1 },
		{ 0, 2 },
		{ 0, 3 },
		{ 0, 4 },
		{ 1, 0 },
		{ 1, 1 },
		{ 1, 2 },
		{ 1, 3 },
		{ 2, 0 },
		{ 2, 1 },
		{ 2, 2 },
		{ 3, 0 },
		{ 3, 1 },
		{ 4, 0 }
	};

	//Setting color to enemy color when eT is 1
	if (eT == 1) {
		switch (color) {
		case P1COLOR: color = P2COLOR; break;
		case P2COLOR: color = P1COLOR; break;
		}
	}

	//Checking if the piece is in a corner/enemy corner
	if (color == P1COLOR || eT == 0) {
		for (int i = 0; i < 15; i++) {
			if (x == P1Corner[i][0] && y == P1Corner[i][1]) {
				return 1;
			}
		}
	}
	if (color == P2COLOR || eT == 0) {
		for (int i = 0; i < 15; i++) {
			if (x == P2Corner[i][0] && y == P2Corner[i][1]) {
				return 1;
			}
		}
	}

	return 0;
}

//Displays the moves for a given (x, y)
//When jumped is set to 1, it will only display jumps (in the jump color). It signifies a multiple jump scenerio.
static short DisplayMoves(int x, int y, int prevX, int prevY, short jumped)
{
	int color = GetButtonColorAtPos(x, y), displayColor = MOVECOLOR;
	int playerTurn = turnCount % 2;
	short flag = 0, eT = 0;
	char timerVal, enemyTimer;

	switch (playerTurn) {
	case 0: color = P1COLOR; timerVal = TM_LCD_TIMER_PLAYER1; enemyTimer = TM_LCD_TIMER_PLAYER2; break;
	case 1: color = P2COLOR; timerVal = TM_LCD_TIMER_PLAYER1; enemyTimer = TM_LCD_TIMER_PLAYER2; break;
	}

	//checking to see if the piece is in enemy territory
	if (CheckCorners(x, y, color, 1))
		eT = 1;

	//change the display color if we are in a multiple jump scenerio
	if (jumped == 1)
		displayColor = JUMPCOLOR;

	//small double for loop for displaying all the moves
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {

			if ((GetButtonColorAtPos(x + i, y + j) != P1COLOR && GetButtonColorAtPos(x + i, y + j) != P2COLOR) && jumped != 1) {
				//if we are in enemey territory, only display moves within that territory
				if (eT == 1) {
					if (CheckCorners(x + i, y + j, color, 1))
						IlluminateButton(x + i, y + j, displayColor);
				}
				else IlluminateButton(x + i, y + j, displayColor);
			}

			else if ((GetButtonColorAtPos(x + i, y + j) == P1COLOR || GetButtonColorAtPos(x + i, y + j) == P2COLOR) && (GetButtonColorAtPos(x + (i * 2), y + (j * 2)) != P1COLOR && GetButtonColorAtPos(x + (i * 2), y + (j * 2)) != P2COLOR)) {
				//if we are in enemey territory, only display moves within that territory
				if (x + (i * 2) == prevX && y + (j * 2) == prevY)
					continue;
				if (eT == 1) {
					if (CheckCorners(x + (i * 2), y + (j * 2), color, 1)) {
						IlluminateButton(x + (i * 2), y + (j * 2), displayColor);
						flag = 1;
					}
				}
				else {
					IlluminateButton(x + (i * 2), y + (j * 2), displayColor);
					flag = 1;
				}
			}
		}
	}

	/*if (flag == 0 && jumped == 1){

	turnCount++;
	if (timerEnable) {
	SetLCDTimerCountMode(timerVal, 1);
	SetLCDTimerCountMode(enemyTimer, 3);
	}

	}*/


	//returns 1 if a jump is available
	return flag;
}

//Erases all displayed moves of a given (x, y)
static void EraseMoves(int x, int y, int color) {

	int cornerColor;
	switch (color) {
	case P1COLOR: cornerColor = P1CORNERCOLOR; break;
	case P2COLOR: cornerColor = P2CORNERCOLOR; break;
	}


	//similar loop to DisplayMoves but does the opposite
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (GetButtonColorAtPos(x + i, y + j) == MOVECOLOR || GetButtonColorAtPos(x + i, y + j) == JUMPCOLOR) {
				if (CheckCorners(x + i, y + j, color, 0)) {
					IlluminateButton(x + i, y + j, cornerColor);
				}
				else IlluminateButton(x + i, y + j, OFFCOLOR);
			}
			else if (GetButtonColorAtPos(x + (i * 2), y + (j * 2)) == MOVECOLOR || GetButtonColorAtPos(x + (i * 2), y + (j * 2)) == JUMPCOLOR) {
				if (CheckCorners(x + (i * 2), y + (j * 2), color, 0)) {
					IlluminateButton(x + (i * 2), y + (j * 2), cornerColor);
				}
				else IlluminateButton(x + (i * 2), y + (j * 2), OFFCOLOR);
			}
		}
	}
}

//Moves a piece from (prevX, prevY) to (x, y)
static short MovePiece(int x, int y, int prevX, int prevY, int color) {

	static int P1Points = 0, P2Points = 0;
	int cornerColor;

	char victory;

	switch (color) {
	case P1COLOR: cornerColor = P1CORNERCOLOR; victory = SPT_GAMEMESSAGE_REDVICTORY; break;
	case P2COLOR: cornerColor = P2CORNERCOLOR; victory = SPT_GAMEMESSAGE_BLUEVICTORY; break;
	}

	//erase the displayed moves, move piece to new location, and erase the piece from the old location
	EraseMoves(prevX, prevY, color);
	IlluminateButton(x, y, color);
	if (CheckCorners(prevX, prevY, color, 0))
		IlluminateButton(prevX, prevY, cornerColor);
	else
		IlluminateButton(prevX, prevY, OFFCOLOR);

	//if we moved a piece into the enemy corner, add a point. 
	if (CheckCorners(x, y, color, 1) == 1 && CheckCorners(prevX, prevY, color, 1) != 1) {
		if (color == P1COLOR)
			P1Points++;
		else
			P2Points++;
	}

	//if we have 15 points, that means all pieces are in the enemy corner. we win!
	if (P1Points >= 15 || P2Points >= 15) {
		SetLCDGameMessage(victory);
		InitSetupPhase(0);
	}

	//if the move was a jump, return 1, else return 0
	if (x == prevX - 2 || x == prevX + 2 || y == prevY - 2 || y == prevY + 2)
		return 1;
	else return 0;
}

static void MakeMove(int x, int y) {

	static int jumpString = 0;
	static int previous[2];
	static short movesDisplayed = 0;
	int playerTurn = turnCount % 2;
	int color;

	char timerVal, enemyTimer;

	//sets color based on turn
	switch (playerTurn) {
	case 0: color = P1COLOR; timerVal = TM_LCD_TIMER_PLAYER1; enemyTimer = TM_LCD_TIMER_PLAYER2; break;
	case 1: color = P2COLOR; timerVal = TM_LCD_TIMER_PLAYER1; enemyTimer = TM_LCD_TIMER_PLAYER2; break;
	}

	//selecting a piece to move
	if (GetButtonColorAtPos(x, y) == color && jumpString != 1) {

		//displays moves
		if (movesDisplayed == 0) {
			DisplayMoves(x, y, previous[0], previous[1], 0);
			movesDisplayed = 1;
		}
		//other moves already displayed, delete those and display moves for the newly selected piece
		else {
			EraseMoves(previous[0], previous[1], color);
			DisplayMoves(x, y, previous[0], previous[1], 0);
		}
		previous[0] = x;
		previous[1] = y;
		return;
	}

	//conditional for ending a multiple jump scenario preemptively
	if (x == previous[0] && y == previous[1] && jumpString == 1) {

		EraseMoves(x, y, color);
		jumpString = 0;
		turnCount++;
		if (timerEnable) {
			SetLCDTimerCountMode(timerVal, 1);
			SetLCDTimerCountMode(enemyTimer, 3);
		}
		return;
	}

	//selecting where you want your piece to move
	if (GetButtonColorAtPos(x, y) == MOVECOLOR) {

		//moves piece, if it was a jump, initiate a multiple jump scenario
		if (MovePiece(x, y, previous[0], previous[1], color) == 1) {
			if (DisplayMoves(x, y, previous[0], previous[1], 1)) {
				jumpString = 1;
				previous[0] = x;
				previous[1] = y;
				return;
			}
		}

		//not a jump, carry on
		turnCount++;
		if (timerEnable) {
			SetLCDTimerCountMode(timerVal, 1);
			SetLCDTimerCountMode(enemyTimer, 3);
		}
		movesDisplayed = 0;
		return;
	}


	//continuing through multiple jump scenario
	if (GetButtonColorAtPos(x, y) == JUMPCOLOR) {

		MovePiece(x, y, previous[0], previous[1], color);

		//jump string continues
		if (DisplayMoves(x, y, previous[0], previous[1], 1)) {
			previous[0] = x;
			previous[1] = y;
			return;
		}
		//jump string has ended, carry on
		else {
			jumpString = 0;
			turnCount++;
			if (timerEnable) {
				SetLCDTimerCountMode(timerVal, 1);
				SetLCDTimerCountMode(enemyTimer, 3);
			}
			return;
		}
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
		MakeMove(x, y);
}

GF_PREFIX void GAMEFUNC(OnLCDButtonPressed)(int id)
{
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

void ChineseCheckers_RegisterGame(int id)
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_CHINESECHECKERS;
	BoardGameInfoList[id].descriptionPreset = SPT_GD_CHECKERS;
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