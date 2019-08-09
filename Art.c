// Copyright 2018 Noah Thering

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
//# Art, created by Noah Thering
//# Express your creativity!
//###############################################################

#pragma once
#include "StandardGameIncludes.h"

// Ensures that all game functions are unique and won't generate linker errors if we switch strategies.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!
#define GAMEFUNC(FUNCNAME) ART_##FUNCNAME

#ifdef DEBUGCHECKS
//#define DEBUGTILEFLIP
#endif

#define DEFAULT_COLOR GC_COLOR_OFF

static const unsigned char m_iColorCycleList[] = { SPT_ART_BOOLEAN, SPT_ART_RGB, SPT_ART_GENERIC, SPT_ART_ALL };
static const unsigned char m_iColorCycleListLength = sizeof(m_iColorCycleList) / sizeof(unsigned char);

static const unsigned char m_iColorCycleBoolean[] = { DEFAULT_COLOR, GC_WHITE };
static const unsigned char m_iColorCycleBooleanLength = sizeof(m_iColorCycleBoolean) / sizeof(unsigned char);

static const unsigned char m_iColorCycleRGB[] = { DEFAULT_COLOR, GC_RED, GC_GREEN, GC_BLUE };
static const unsigned char m_iColorCycleRGBLength = sizeof(m_iColorCycleRGB) / sizeof(unsigned char);

static const unsigned char m_iColorCycleMain[] = { DEFAULT_COLOR, GC_WHITE, GC_RED, GC_PURPLE, GC_BLUE, GC_CYAN, GC_GREEN, GC_YELLOW };
static const unsigned char m_iColorCycleMainLength = sizeof(m_iColorCycleMain) / sizeof(unsigned char);

#define ALLCOLORS_CYCLE (m_iColorCycleListLength - 1)

// Configuration Options:
static unsigned char m_iColorCycle;

// Gameplay Variables
static unsigned char m_bIsSetup; // 1 if setup phase, 0 if game phase.
static const unsigned char *m_iColorCyclePointer;
static unsigned char m_iColorCycleLength;

// Menu IDs
// Setup
#define MSLOT_S_STARTGAME 0
#define MSLOT_S_COLORCYCLE 1

// Game
#define MSLOT_G_RESTART 0
#define MSLOT_G_RECONFIGURE 1
#define MSLOT_G_COLORCYCLE 2

// Menu Display Arrays
const static unsigned short m_iRandomnessArray[] = { SPT_UTIL_OFF, SPT_UTIL_LOW, SPT_UTIL_MED, SPT_UTIL_HIGH };
const static unsigned short m_iRandomnessArrayLen = sizeof(m_iRandomnessArray) / sizeof(unsigned short);

// Game Specific Functions!  ALL OF THESE SHOULD BE DECLARED STATIC TO LIMIT THEM TO THE FILE SCOPE!
static void LoadColorCycleWithID(unsigned char id);

static unsigned short InitSetupPhase( unsigned short freshConfiguration )
{
	m_bIsSetup = 1;

	if ( freshConfiguration ) // Do initial setup stuff here that should only happen on fresh reloads.
	{
		// Create our "home" configuration.
		SetBoardSize( BOARDSIZEX, BOARDSIZEY );
		IlluminateBoard( DEFAULT_COLOR );

		SetLCDGameMessage( SPT_ART_SETUP );
	}
	else // Do any cleanup here to freeze the gamestate of a previous playthrough.
	{

	}

	// Init our setup phase menu.
	ClearAllMenuOptions();

	// Use different text depending on if we've been playing this game already or not.
	if ( freshConfiguration )
		RegisterMenuOption(SPT_OPTIONS_STARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);
	else
		RegisterMenuOption(SPT_OPTIONS_RESTARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);

	RegisterMenuOptionWithStringParameter( SPT_OPTIONS_COLORCYCLE, IMAGEID_NONE, m_iColorCycleList[m_iColorCycle], MSLOT_S_COLORCYCLE );

	// Nothing that can cause errors here, really, so always return 0.
	return 0;
}

static void InitGamePhase()
{
	m_bIsSetup = 0;

	// Play our start que
	PlaySoundPreset( SOUNDID_GAMESTART );

	// Init our LCD display values
	SetLCDGameDisplayFormat( GDCONFIG_NONE ); // No players in art, only masters.
	SetLCDGameMessage( SPT_ART_START );

	// Create our starting LED configuration.
	SetBoardSize( BOARDSIZEX, BOARDSIZEY );
	IlluminateBoard( DEFAULT_COLOR );

	// Init our game phase menu.
	ClearAllMenuOptions();
	RegisterMenuOption(SPT_OPTIONS_CLEARBOARD, IMAGEID_NONE, MSLOT_G_RESTART);
	RegisterMenuOption(SPT_OPTIONS_RECONFIGURE, IMAGEID_NONE, MSLOT_G_RECONFIGURE);
	RegisterMenuOptionWithStringParameter( SPT_OPTIONS_COLORCYCLE, IMAGEID_NONE, m_iColorCycleList[m_iColorCycle], MSLOT_G_COLORCYCLE );

	LoadColorCycleWithID(m_iColorCycle);
}

static void ShiftGrid()
{
	unsigned char holderRow[BOARDSIZEX];

	for (int g = 0; g < BOARDSIZEX; g++)
	{
		holderRow[g] = GetButtonColorAtPos(g, 0);
	}

	for (int i = 0; i < BOARDSIZEY - 1; i++)
	{
		for (int g = 0; g < BOARDSIZEX; g++)
		{
			IlluminateButton( g, i, GetButtonColorAtPos(g, i + 1) );
		}
	}

	for (int g = 0; g < BOARDSIZEX; g++)
	{
		IlluminateButton(g, BOARDSIZEY - 1, holderRow[g]);
	}
}

static void LoadColorCycleWithID(unsigned char id)
{
	m_iColorCycle = id;

	switch ( id )
	{
		case 0: // Boolean
			m_iColorCyclePointer = m_iColorCycleBoolean;
			m_iColorCycleLength = m_iColorCycleBooleanLength;
			break;
		case 1: // RGB
			m_iColorCyclePointer = m_iColorCycleRGB;
			m_iColorCycleLength = m_iColorCycleRGBLength;
			break;
		case 2: // Generic
			m_iColorCyclePointer = m_iColorCycleMain;
			m_iColorCycleLength = m_iColorCycleMainLength;
			break;
		default: // Make sure we always have -some- pointer.
			m_iColorCyclePointer = m_iColorCycleBoolean;
			m_iColorCycleLength = m_iColorCycleBooleanLength;
			break;
	}
}


static void AdvanceColorSquare(unsigned char x, unsigned char y)
{
	unsigned char currentColor = GetButtonColorAtPos(x, y);
	unsigned char newColor = DEFAULT_COLOR;

	if (m_iColorCycle == ALLCOLORS_CYCLE)
	{
		newColor = (currentColor + 1) % GC_COLOR_MAX;
	}
	else
	{
		int i = 0;
		for (; i < m_iColorCycleLength; i++)
		{
			if (m_iColorCyclePointer[i] == currentColor)
				break; // Found current color in current color cycle, carry it over to the next part of the function.
		}
	
		// It's okay if we didn't find our color and i ends up at the max for the cycle, it will be fit into it by the modulus.
		newColor = m_iColorCyclePointer[(i + 1) % m_iColorCycleLength];
	}

	IlluminateButton(x, y, newColor);
}

GF_PREFIX int GAMEFUNC(OnGameLoaded)()
{
	// Start in the setup phase.
	return InitSetupPhase( 1 ); // Always use a fresh configuration on initial game load.
}

GF_PREFIX void GAMEFUNC(OnButtonPressed)( int x, int y )
{
	if ( m_bIsSetup )
		PlaySoundPreset( SOUNDID_DENY );
	else
		AdvanceColorSquare(x, y);
}

GF_PREFIX void GAMEFUNC(OnLCDButtonPressed)( int id )
{
	if ( m_bIsSetup )
		PlaySoundPreset( SOUNDID_DENY );
	else
		ShiftGrid();
}

GF_PREFIX void GAMEFUNC(OnExit)( int reason )
{
	// Nothing to clean up on exit.
}


GF_PREFIX void GAMEFUNC(OnTimerFinished)( int id )
{
	// No internal timers used in tileflip.
}

GF_PREFIX void GAMEFUNC(OnLCDTimerHitZero)( int id )
{
	// No match timers used in tileflip.
}

GF_PREFIX void GAMEFUNC(OnMenuOptionSelected)( int id )
{
	if (m_bIsSetup)
	{
		switch (id)
		{
			case MSLOT_S_STARTGAME: InitGamePhase(); break;
			case MSLOT_S_COLORCYCLE:
				m_iColorCycle = (m_iColorCycle + 1) % m_iColorCycleListLength;

				RegisterMenuOptionWithStringParameter( SPT_OPTIONS_COLORCYCLE, IMAGEID_NONE, m_iColorCycleList[m_iColorCycle], MSLOT_S_COLORCYCLE );
				break;
		}
	}
	else
	{
		switch (id)
		{
			case MSLOT_G_RESTART: InitGamePhase(); break;
			case MSLOT_G_RECONFIGURE: SetLCDGameMessage( SPT_GAMEMESSAGE_GENERICSETUP ); InitSetupPhase(0); break;
			case MSLOT_G_COLORCYCLE: 
				m_iColorCycle = (m_iColorCycle + 1) % m_iColorCycleListLength;
				LoadColorCycleWithID(m_iColorCycle);

				RegisterMenuOptionWithStringParameter(SPT_OPTIONS_COLORCYCLE, IMAGEID_NONE, m_iColorCycleList[m_iColorCycle], MSLOT_G_COLORCYCLE); 
				break;
		}
	}
}

// Fires when the system has not received an input for a "long time"
GF_PREFIX void GAMEFUNC(OnIdle)()
{
	SetColorMode(1);
}

// Fires when the system has received an input while idle.
GF_PREFIX void GAMEFUNC(OnWake)( int reason )
{
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

void Art_RegisterGame( int id )
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_ART;
	BoardGameInfoList[id].descriptionPreset = SPT_GD_ART;
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