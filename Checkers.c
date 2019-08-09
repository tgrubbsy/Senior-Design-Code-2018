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
//# The classic game of checkers!
//###############################################################

#pragma once
#include "StandardGameIncludes.h"

// Ensures that all game functions are unique and won't generate linker errors if we switch strategies.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!
#define GAMEFUNC(FUNCNAME) CHK_##FUNCNAME

#define LIGHTSBOARDSIZE 8

#define P1COLOR GC_RED
#define P2COLOR GC_BLUE
#define P1KINGCOLOR GC_ORANGE
#define P2KINGCOLOR GC_PURPLE
#define JUMPCOLOR GC_PINK
#define OFFCOLOR GC_GRAY
#define MOVECOLOR GC_YELLOW

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
static int DisplayMoves(int x, int y, int jump, int canJump, int checkMode);

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

	//P1 Board
	IlluminateButton(0, 7, P1COLOR);
	IlluminateButton(2, 7, P1COLOR);
	IlluminateButton(4, 7, P1COLOR);
	IlluminateButton(6, 7, P1COLOR);
	IlluminateButton(0, 5, P1COLOR);
	IlluminateButton(2, 5, P1COLOR);
	IlluminateButton(4, 5, P1COLOR);
	IlluminateButton(6, 5, P1COLOR);
	IlluminateButton(1, 6, P1COLOR);
	IlluminateButton(3, 6, P1COLOR);
	IlluminateButton(5, 6, P1COLOR);
	IlluminateButton(7, 6, P1COLOR);

	//P2 Board
	IlluminateButton(0, 1, P2COLOR);
	IlluminateButton(2, 1, P2COLOR);
	IlluminateButton(4, 1, P2COLOR);
	IlluminateButton(6, 1, P2COLOR);
	IlluminateButton(1, 0, P2COLOR);
	IlluminateButton(3, 0, P2COLOR);
	IlluminateButton(5, 0, P2COLOR);
	IlluminateButton(7, 0, P2COLOR);
	IlluminateButton(1, 2, P2COLOR);
	IlluminateButton(3, 2, P2COLOR);
	IlluminateButton(5, 2, P2COLOR);
	IlluminateButton(7, 2, P2COLOR);

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

static void EndGame(int winningColor)
{

	char victory;

	switch (winningColor) {
	case P1COLOR: victory = SPT_GAMEMESSAGE_REDVICTORY;
	case P1KINGCOLOR: victory = SPT_GAMEMESSAGE_REDVICTORY; break;
	case P2COLOR: victory = SPT_GAMEMESSAGE_BLUEVICTORY;
	case P2KINGCOLOR: victory = SPT_GAMEMESSAGE_BLUEVICTORY; break;
	}

	SetLCDGameMessage(victory);
	InitSetupPhase(0);
}

//Logic for checking if a jump is available on the board
static int CheckJumps(int color, int kingColor)
{

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (GetButtonColorAtPos(i, j) == color || GetButtonColorAtPos(i, j) == kingColor) {
				if (DisplayMoves(i, j, 0, 0, 1) == 1) {
					return 1;
				}
			}
		}
	}

	return 0;
}

static int DisplayMoves(int x, int y, int jump, int canJump, int checkMode)
{
	int color = GetButtonColorAtPos(x, y), displayColor = MOVECOLOR;
	int leftY, rightY, leftJumpY, rightJumpY, king = 0, enemy, enemyKing;
	int flag = 0;

	switch (color) {
	case P1COLOR: leftY = -1; rightY = -1; leftJumpY = -2; rightJumpY = -2; enemy = P2COLOR; enemyKing = P2KINGCOLOR; break;
	case P2COLOR: leftY = 1; rightY = 1; leftJumpY = 2; rightJumpY = 2; enemy = P1COLOR; enemyKing = P1KINGCOLOR; break;
	case P1KINGCOLOR: king = 1; enemy = P2COLOR; enemyKing = P2KINGCOLOR; break;
	case P2KINGCOLOR: king = 1; enemy = P1COLOR; enemyKing = P1KINGCOLOR; break;
	}

	if (jump == 1)
		displayColor = GC_PINK;

	//king moves
	if (king == 1) {

		//jumps
		if ((GetButtonColorAtPos(x - 1, y - 1) == enemy || GetButtonColorAtPos(x - 1, y - 1) == enemyKing) && GetButtonColorAtPos(x - 2, y - 2) == OFFCOLOR) {
			if (checkMode != 1)
				IlluminateButton(x - 2, y - 2, displayColor);
			flag = 1;
		}
		if ((GetButtonColorAtPos(x + 1, y - 1) == enemy || GetButtonColorAtPos(x + 1, y - 1) == enemyKing) && GetButtonColorAtPos(x + 2, y - 2) == OFFCOLOR) {
			if (checkMode != 1)
				IlluminateButton(x + 2, y - 2, displayColor);
			flag = 1;
		}
		if ((GetButtonColorAtPos(x - 1, y + 1) == enemy || GetButtonColorAtPos(x - 1, y + 1) == enemyKing) && GetButtonColorAtPos(x - 2, y + 2) == OFFCOLOR) {
			if (checkMode != 1)
				IlluminateButton(x - 2, y + 2, displayColor);
			flag = 1;
		}
		if ((GetButtonColorAtPos(x + 1, y + 1) == enemy || GetButtonColorAtPos(x + 1, y + 1) == enemyKing) && GetButtonColorAtPos(x + 2, y + 2) == OFFCOLOR) {
			if (checkMode != 1)
				IlluminateButton(x + 2, y + 2, displayColor);
			flag = 1;
		}

		if (checkMode == 1)
			return flag;

		//normal moves
		if (GetButtonColorAtPos(x - 1, y - 1) == OFFCOLOR && jump != 1 && canJump != 1)
			IlluminateButton(x - 1, y - 1, displayColor);
		if (GetButtonColorAtPos(x + 1, y - 1) == OFFCOLOR && jump != 1 && canJump != 1)
			IlluminateButton(x + 1, y - 1, displayColor);
		if (GetButtonColorAtPos(x - 1, y + 1) == OFFCOLOR && jump != 1 && canJump != 1)
			IlluminateButton(x - 1, y + 1, displayColor);
		if (GetButtonColorAtPos(x + 1, y + 1) == OFFCOLOR && jump != 1 && canJump != 1)
			IlluminateButton(x + 1, y + 1, displayColor);
	}

	//regular piece moves
	else {

		//jumps
		if ((GetButtonColorAtPos(x - 1, y + leftY) == enemy || GetButtonColorAtPos(x - 1, y + leftY) == enemyKing) && GetButtonColorAtPos(x - 2, y + leftJumpY) == OFFCOLOR) {
			if (checkMode != 1)
				IlluminateButton(x - 2, y + leftJumpY, displayColor);
			flag = 1;
		}
		if ((GetButtonColorAtPos(x + 1, y + rightY) == enemy || GetButtonColorAtPos(x + 1, y + rightY) == enemyKing) && GetButtonColorAtPos(x + 2, y + rightJumpY) == OFFCOLOR) {
			if (checkMode != 1)
				IlluminateButton(x + 2, y + rightJumpY, displayColor);
			flag = 1;
		}

		if (checkMode == 1)
			return flag;

		//normal moves
		if (GetButtonColorAtPos(x - 1, y + leftY) == OFFCOLOR && jump != 1 && canJump != 1)
			IlluminateButton(x - 1, y + leftY, displayColor);
		if (GetButtonColorAtPos(x + 1, y + rightY) == OFFCOLOR && jump != 1 && canJump != 1)
			IlluminateButton(x + 1, y + rightY, displayColor);
	}

	return flag;
}

static void EraseMoves(int x, int y) {

	if (GetButtonColorAtPos(x - 1, y - 1) == MOVECOLOR)
		IlluminateButton(x - 1, y - 1, OFFCOLOR);
	if (GetButtonColorAtPos(x + 1, y - 1) == MOVECOLOR)
		IlluminateButton(x + 1, y - 1, OFFCOLOR);
	if (GetButtonColorAtPos(x - 1, y + 1) == MOVECOLOR)
		IlluminateButton(x - 1, y + 1, OFFCOLOR);
	if (GetButtonColorAtPos(x + 1, y + 1) == MOVECOLOR)
		IlluminateButton(x + 1, y + 1, OFFCOLOR);
	if (GetButtonColorAtPos(x - 2, y - 2) == MOVECOLOR || GetButtonColorAtPos(x - 2, y - 2) == JUMPCOLOR)
		IlluminateButton(x - 2, y - 2, OFFCOLOR);
	if (GetButtonColorAtPos(x + 2, y - 2) == MOVECOLOR || GetButtonColorAtPos(x + 2, y - 2) == JUMPCOLOR)
		IlluminateButton(x + 2, y - 2, OFFCOLOR);
	if (GetButtonColorAtPos(x - 2, y + 2) == MOVECOLOR || GetButtonColorAtPos(x - 2, y + 2) == JUMPCOLOR)
		IlluminateButton(x - 2, y + 2, OFFCOLOR);
	if (GetButtonColorAtPos(x + 2, y + 2) == MOVECOLOR || GetButtonColorAtPos(x + 2, y + 2) == JUMPCOLOR)
		IlluminateButton(x + 2, y + 2, OFFCOLOR);
}

static short MovePiece(int x, int y, int prevX, int prevY) {

	static int p1Pieces = 12;
	static int p2Pieces = 12;
	int tempColor;
	int color = GetButtonColorAtPos(prevX, prevY);

	EraseMoves(prevX, prevY);
	IlluminateButton(x, y, color);
	IlluminateButton(prevX, prevY, OFFCOLOR);

	if (color != P1KINGCOLOR && color != P2KINGCOLOR) {
		if (y == 0 || y == 7) {
			int kingColor, end;

			switch (color) {
			case P1COLOR: kingColor = P1KINGCOLOR; end = 0; break;
			case P2COLOR: kingColor = P2KINGCOLOR; end = 7; break;
			}

			if (y == end)
				IlluminateButton(x, y, kingColor);
		}
	}

	//if move was a jump, delete the jumped piece and return 1, else return 0
	if (x == prevX - 2 || x == prevX + 2) {
		if (x == prevX - 2) {
			if (y == prevY - 2) {
				tempColor = GetButtonColorAtPos(prevX - 1, prevY - 1);
				IlluminateButton(prevX - 1, prevY - 1, OFFCOLOR);
				if (tempColor == P2COLOR || tempColor == P2KINGCOLOR) {
					PrintDebugMessage("p2 piece removed\n");
					p2Pieces--;
				}
				else if (tempColor == P1COLOR || tempColor == P1KINGCOLOR) {
					PrintDebugMessage("p1 piece removed\n");
					p1Pieces--;
				}
			}
			else {
				tempColor = GetButtonColorAtPos(prevX - 1, prevY + 1);
				IlluminateButton(prevX - 1, prevY + 1, OFFCOLOR);
				if (tempColor == P2COLOR || tempColor == P2KINGCOLOR) {
					PrintDebugMessage("p2 piece removed\n");
					p2Pieces--;
				}
				else if (tempColor == P1COLOR || tempColor == P1KINGCOLOR) {
					PrintDebugMessage("p1 piece removed\n");
					p1Pieces--;
				}
			}
		}
		else {
			if (y == prevY - 2) {
				tempColor = GetButtonColorAtPos(prevX + 1, prevY - 1);
				IlluminateButton(prevX + 1, prevY - 1, OFFCOLOR);
				if (tempColor == P2COLOR || tempColor == P2KINGCOLOR) {
					PrintDebugMessage("p2 piece removed\n");
					p2Pieces--;
				}
				else if (tempColor == P1COLOR || tempColor == P1KINGCOLOR) {
					PrintDebugMessage("p1 piece removed\n");
					p1Pieces--;
				}
			}
			else {
				tempColor = GetButtonColorAtPos(prevX + 1, prevY + 1);
				IlluminateButton(prevX + 1, prevY + 1, OFFCOLOR);
				if (tempColor == P2COLOR || tempColor == P2KINGCOLOR) {
					PrintDebugMessage("p2 piece removed\n");
					p2Pieces--;
				}
				else if (tempColor == P1COLOR || tempColor == P1KINGCOLOR) {
					PrintDebugMessage("p1 piece removed\n");
					p1Pieces--;
				}
			}

		}

		if (p1Pieces == 0 || p2Pieces == 0)
			EndGame(color);
		return 1;
	}
	else {
		return 0;
	}

}

static void MakeMove(int x, int y) {

	static int jumpString = 0;
	static int previous[2];
	static short movesDisplayed = 0;
	int playerTurn = turnCount % 2;
	int color, kingColor;

	char timerVal, enemyTimer;

	switch (playerTurn) {
	case 0: color = P1COLOR; kingColor = P1KINGCOLOR; timerVal = TM_LCD_TIMER_PLAYER1; enemyTimer = TM_LCD_TIMER_PLAYER2; break;
	case 1: color = P2COLOR; kingColor = P2KINGCOLOR; timerVal = TM_LCD_TIMER_PLAYER2; enemyTimer = TM_LCD_TIMER_PLAYER1; break;
	}

	if ((GetButtonColorAtPos(x, y) == color || GetButtonColorAtPos(x, y) == kingColor) && jumpString != 1) {

		if (movesDisplayed == 0) {
			DisplayMoves(x, y, 0, CheckJumps(color, kingColor), 0);
			movesDisplayed = 1;
		}
		else {
			EraseMoves(previous[0], previous[1]);
			DisplayMoves(x, y, 0, CheckJumps(color, kingColor), 0);
		}
		previous[0] = x;
		previous[1] = y;
		return;
	}

	if (GetButtonColorAtPos(x, y) == MOVECOLOR) {

		if (MovePiece(x, y, previous[0], previous[1]) == 1) {
			if (DisplayMoves(x, y, 1, 0, 0)) {
				jumpString = 1;
				previous[0] = x;
				previous[1] = y;
				return;
			}
		}

		turnCount++;
		if (timerEnable) {
			SetLCDTimerCountMode(timerVal, 1);
			SetLCDTimerCountMode(enemyTimer, 3);
		}
		movesDisplayed = 0;
		return;
	}

	if (GetButtonColorAtPos(x, y) == JUMPCOLOR) {

		MovePiece(x, y, previous[0], previous[1]);

		if (DisplayMoves(x, y, 1, 0, 0)) {
			previous[0] = x;
			previous[1] = y;
		}
		else {
			jumpString = 0;
			turnCount++;
			if (timerEnable) {
				SetLCDTimerCountMode(timerVal, 1);
				SetLCDTimerCountMode(enemyTimer, 3);
			}
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

void Checkers_RegisterGame(int id)
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_CHECKERS;
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