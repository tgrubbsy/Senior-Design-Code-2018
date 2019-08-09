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
//# StraightEdge.c, created by Taylor Grubbs
//# Make a line of your color to win!
//###############################################################
#pragma once
#include "StandardGameIncludes.h"

// Ensures that all game functions are unique and won't generate linker errors.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!  Be sure to use this macro for
// every function you create to avoid possible linker errors.
#define GAMEFUNC(FUNCNAME) /*three letter code*/CNF_##FUNCNAME

#define LIGHTSBOARDCOLSIZE 7
#define LIGHTSBOARDROWSIZE 6

#define ONCOLOR GC_WHITE
#define OFFCOLOR GC_GRAY
#define P1COLOR GC_RED
#define P2COLOR GC_BLUE

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

static int turnCount = 0;

// Game Specific Functions!  ALL OF THESE SHOULD BE DECLARED STATIC TO LIMIT THEM TO THE FILE SCOPE!

static unsigned short InitSetupPhase(unsigned short freshConfiguration)
{
	m_bIsSetup = 1;

	if (freshConfiguration) // Do initial setup stuff here that should only happen on fresh reloads.
	{
		// Create our "home" configuration.
		SetBoardSize(LIGHTSBOARDCOLSIZE, LIGHTSBOARDROWSIZE);
		IlluminateBoard(OFFCOLOR);

		SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSETUP);
	}
	else // Do any cleanup here to freeze the gamestate of a previous playthrough.
	{
	}

	// Init our setup phase menu.
	ClearAllMenuOptions();

	// Use different text depending on if we've been playing this game already or not.
	if (freshConfiguration)
		RegisterMenuOption(SPT_OPTIONS_STARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);
	else
		RegisterMenuOption(SPT_OPTIONS_RESTARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);

	// Nothing that can cause errors here, really, so always return 0.
	return 0;
}

static void InitGamePhase()
{
	m_bIsSetup = 0;

	// Play our start que
	PlaySoundPreset(SOUNDID_GAMESTART);

	// Init our LCD display values

	SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSTART);

	// Create our starting LED configuration.
	SetBoardSize(LIGHTSBOARDCOLSIZE, LIGHTSBOARDROWSIZE);
	IlluminateBoard(OFFCOLOR);

	turnCount = 0;

	// Init our game phase menu.
	ClearAllMenuOptions();
	RegisterMenuOption(SPT_OPTIONS_RESTART, IMAGEID_NONE, MSLOT_G_RESTART);
	RegisterMenuOption(SPT_OPTIONS_RECONFIGURE, IMAGEID_NONE, MSLOT_G_RECONFIGURE);
}

static void EndGame(int color) {

	switch (color) {
	case P1COLOR: SetLCDGameMessage(SPT_GAMEMESSAGE_REDVICTORY); break;
	case P2COLOR: SetLCDGameMessage(SPT_GAMEMESSAGE_BLUEVICTORY); break;
	}

	InitSetupPhase(0);
}

static short VictoryCheck(int x, int y, int color)
{
	int combo = 0;

	//checking for horizontal combo 

	//searching left
	int k = 0;
	for (int i = x; i >= 0; i--) {

		if (GetButtonColorAtPos(i, y) == color && (k < 4)) {
			combo++;
			k++;
			continue;
		}

		break;
	}

	if (combo >= 4) {
		return 1;
	}

	combo--;
	//searching right
	k = 0;
	for (int i = x; i < 7; i++) {

		if (GetButtonColorAtPos(i, y) == color && (k < 4)) {
			combo++;
			k++;
			continue;
		}

		break;
	}

	if (combo >= 4) {
		return 1;
	}
	combo = 0;

	//checking for vertical combo 

	//searching down (completing a vertical combo will ensure the most recently pressed tile is the one on top)
	k = 0;
	for (int i = y; i < 6; i++) {

		if (GetButtonColorAtPos(x, i) == color && (k < 4)) {
			combo++;
			k++;
			continue;
		}

		break;
	}

	if (combo >= 4) {
		return 1;
	}
	combo = 0;

	//checking for upperleft->lowerright diagonal combo

	//searching upper left
	int j = y;
	k = 0;
	for (int i = x; i >= 0; i--) {


		if (j < 0) {
			break;
		}

		if (GetButtonColorAtPos(i, j) == color && (k < 4)) {
			combo++;
			k++;
			j--;
			continue;
		}
		break;
	}

	if (combo >= 4) {
		return 1;
	}
	combo--;

	//searching lower right
	j = y;
	k = 0;
	for (int i = x; i < 7; i++) {

		if (j > 6) {
			break;
		}

		if (GetButtonColorAtPos(i, j) == color && (k < 4)) {
			combo++;
			k++;
			j++;
			continue;
		}
		break;
	}

	if (combo >= 4) {
		return 1;
	}
	combo = 0;

	//checking for lowerleft->upperright diagonal combo

	//searching lower left
	j = y;
	k = 0;
	for (int i = x; i >= 0; i--) {


		if (j > 6) {
			break;
		}

		if (GetButtonColorAtPos(i, j) == color && (k < 4)) {
			combo++;
			k++;
			j++;
			continue;
		}
		break;
	}

	if (combo >= 4) {
		return 1;
	}
	combo--;

	//searching upper right
	j = y;
	k = 0;
	for (int i = x; i < 7; i++) {

		if (j > 0) {
			break;
		}

		if (GetButtonColorAtPos(i, j) == color && (k < 4)) {
			combo++;
			k++;
			j--;
			continue;
		}
		break;
	}

	if (combo >= 4) {
		return 1;
	}

	return 0;
}

static void MakeMove(int x, int y) {

	int playerTurn = turnCount % 2;
	int color;

	switch (playerTurn) {
	case 0: color = P1COLOR; break;
	case 1: color = P2COLOR; break;
	}

	if (GetButtonColorAtPos(x, y) != P1COLOR && GetButtonColorAtPos(x, y) != P2COLOR) {
		for (int i = 5; i >= 0; i--) {

			if ((GetButtonColorAtPos(x, i) == P1COLOR) || (GetButtonColorAtPos(x, i) == P2COLOR))
				continue;

			IlluminateButton(x, i, color);
			y = i;
			turnCount++;
			break;
		}
	}

	if (turnCount > 6) {
		if (VictoryCheck(x, y, color) == 1)
			EndGame(color);
		else if (turnCount == 42) {
			SetLCDGameMessage(SPT_GAMEMESSAGE_TIEGAME);
			InitSetupPhase(0);
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

}

GF_PREFIX void GAMEFUNC(OnMenuOptionSelected)(int id)
{
	if (m_bIsSetup)
	{
		switch (id)
		{
		case MSLOT_S_STARTGAME: InitGamePhase(); break;
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

void StraightEdge_RegisterGame(int id)
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_STRAIGHTEDGE;
	BoardGameInfoList[id].descriptionPreset = SPT_GD_STRAIGHTEDGE;
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