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
//# Tile Flip, created by Noah Thering
//# Game where you try and flip all the tiles to white!
//###############################################################

#pragma once
#include "StandardGameIncludes.h"

// Ensures that all game functions are unique and won't generate linker errors if we switch strategies.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!
#define GAMEFUNC(FUNCNAME) TIF_##FUNCNAME

#ifdef DEBUGCHECKS
//#define DEBUGTILEFLIP
#endif

#define TILESBOARDSIZE_DEFAULT 5

#define ONCOLOR GC_WHITE
#define OFFCOLOR GC_GRAY
#define VICTORYCOLOR GC_YELLOW

#define STARTINGSCOREVALUE 1000
#define SCOREMOVEPENALTY 1

// Configuration Options:
static unsigned char m_iBoardSize;
static unsigned char m_iStartRandomness;
static unsigned char m_bRandomMoves;
static unsigned char m_bRandomMovePattern;

// Gameplay Variables
static unsigned char m_bIsSetup; // 1 if setup phase, 0 if game phase.
static unsigned char m_iPlayerMoves; // How many moves the player has made so far.

// Timer IDs
#define GT_RANDOMMOVES 0

// Menu IDs
// Setup
#define MSLOT_S_STARTGAME 0
#define MSLOT_S_BOARDSIZE 1
#define MSLOT_S_RANDOMNESS 2
#define MSLOT_S_RANDOMMOVES 3

// Game
#define MSLOT_G_RESTART 0
#define MSLOT_G_RECONFIGURE 1

// Menu Display Arrays
const static unsigned short m_iRandomnessArray[] = { SPT_UTIL_OFF, SPT_UTIL_LOW, SPT_UTIL_MED, SPT_UTIL_HIGH };
const static unsigned short m_iRandomnessArrayLen = sizeof(m_iRandomnessArray) / sizeof(unsigned short);

// Game Specific Functions!  ALL OF THESE SHOULD BE DECLARED STATIC TO LIMIT THEM TO THE FILE SCOPE!
static void MakeMove(int x, int y);

static unsigned short InitSetupPhase( unsigned short freshConfiguration )
{
	m_bIsSetup = 1;

	if ( freshConfiguration ) // Do initial setup stuff here that should only happen on fresh reloads.
	{
		// Create our "home" configuration.
		SetBoardSize( TILESBOARDSIZE_DEFAULT, TILESBOARDSIZE_DEFAULT );
		IlluminateBoard( OFFCOLOR );

		SetLCDGameMessage( SPT_GAMEMESSAGE_GENERICSETUP );

		// Only set our config variables if we want a fresh configuration, otherwise use the old ones for easy tweaking.
		m_iBoardSize = TILESBOARDSIZE_DEFAULT;
		m_iStartRandomness = 0;
		m_bRandomMoves = 0;

		m_iPlayerMoves = 0;
	}
	else // Do any cleanup here to freeze the gamestate of a previous playthrough.
	{
		// We're using a timer so be sure to pause it.
		SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER1, 1);

		KillTimer(GT_RANDOMMOVES);
	}

	// Init our setup phase menu.
	ClearAllMenuOptions();

	// Use different text depending on if we've been playing this game already or not.
	if ( freshConfiguration )
		RegisterMenuOption(SPT_OPTIONS_STARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);
	else
		RegisterMenuOption(SPT_OPTIONS_RESTARTGAME, IMAGEID_NONE, MSLOT_S_STARTGAME);

	RegisterMenuOptionWithIntParameter(SPT_OPTIONS_BOARDSIZE, IMAGEID_NONE, m_iBoardSize, MSLOT_S_BOARDSIZE);
	RegisterMenuOptionWithStringParameter(SPT_OPTIONS_RANDOMSTART, IMAGEID_NONE, m_iRandomnessArray[m_iStartRandomness / 10], MSLOT_S_RANDOMNESS);
	RegisterMenuOptionWithStringParameter(SPT_OPTIONS_RANDOM_MOVES, IMAGEID_NONE, m_bRandomMoves ? SPT_UTIL_ON : SPT_UTIL_OFF, MSLOT_S_RANDOMMOVES);

	// Nothing that can cause errors here, really, so always return 0.
	return 0;
}

static void InitGamePhase()
{
	m_bIsSetup = 0;

	// Play our start que
	PlaySoundPreset( SOUNDID_GAMESTART );

	// Init our LCD display values
	SetLCDGameDisplayFormat(GDCONFIG_ONEPLAYER); // Just one player for tileflip
	SetLCDTimerValue(TM_LCD_TIMER_PLAYER1, 0); // Timer starts at 0 seconds
	SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER1, 2); // Timer counts up

	SetLCDGameMessage( SPT_GAMEMESSAGE_GENERICSTART );

	// Create our starting LED configuration.
	SetBoardSize( m_iBoardSize, m_iBoardSize );
	IlluminateBoard( OFFCOLOR );

	// Init our game phase menu.
	ClearAllMenuOptions();
	RegisterMenuOption(SPT_OPTIONS_RESTART, IMAGEID_NONE, MSLOT_G_RESTART);
	RegisterMenuOption(SPT_OPTIONS_RECONFIGURE, IMAGEID_NONE, MSLOT_G_RECONFIGURE);

	// Make a randomly decided amount of random moves to generate a unique starting state that's also solvable.
	int moveCount = GenerateRandomInt( m_iStartRandomness, m_iStartRandomness * 2 + 1 );

#ifdef DEBUGTILEFLIP
	char buffer[32];
	G_sprintf(buffer, 32, "Making %d moves!\n", moveCount);

	PrintDebugMessage(buffer);
#endif

	for (int i = 0; i < moveCount; i++)
		MakeMove(GenerateRandomInt(0, m_iBoardSize), GenerateRandomInt(0, m_iBoardSize));

	// Do this last so our score isn't affected by the generator moves.
	m_iPlayerMoves = 0;
	SetLCDScoreDisplayValue(TM_LCD_SCORE_PLAYER1, 0); // We always start at 0 moves

	if ( m_bRandomMoves )
	{
		// Start a repeating timer that makes 2 moves every 10 seconds.
		StartTimer(GT_RANDOMMOVES, 10000, 1);
	}
}

static short VictoryCheck()
{
	for ( int i = 0; i < m_iBoardSize; i++ )
	{
		for ( int g = 0; g < m_iBoardSize; g++ )
		{
			if ( GetBaseButtonColorAtPos(g, i) == OFFCOLOR )
			{
				return 0;
			}
		}
	}

	// If we've made it all the way to here, we've done it!  Not a single tile is off.
	return 1;
}

static void MakeMove(int x, int y)
{
	PlaySound(900, 100);

	SetLCDGameMessage( SPT_TIF_TILEFLIPED );

	// If we only flip squares to on, then there's a chance we've won the game!
	// Otherwise we don't need to peform a somewhat expensive check for victory.
	short allPositive = 1;

	m_iPlayerMoves++;
	SetLCDScoreDisplayValue(TM_LCD_SCORE_PLAYER1, m_iPlayerMoves);

	for (int i = 0; i < 5; i++)
	{
		int xPos = x;
		int yPos = y;

		switch (i)
		{
			case 0: xPos += 1; break;
			case 1: yPos += 1; break;
			case 2: xPos -= 1; break;
			case 3: yPos -= 1; break;
			default: break;
		}

		// Make sure this is an actual square we can flip.
		if (xPos >= m_iBoardSize || yPos >= m_iBoardSize || xPos < 0 || yPos < 0)
			continue;

		// Flip the square!
		unsigned char color2Use = ONCOLOR;
		if ( GetBaseButtonColorAtPos(xPos, yPos) == ONCOLOR )
		{
			// We're flipping to an off state so be sure to record that.
			allPositive = 0;
			color2Use = OFFCOLOR;
		}

		IlluminateButton( xPos, yPos, color2Use );
	}

	// Once we've won, turn all the squares yellow!
	// Once this state is acheived the board will stay this way until reset because
	// any click can only turn squares white which will instantly make the board
	// turn all yellow again since there will still be no gray squares.
	if (allPositive && VictoryCheck())
	{
		if (m_iBoardSize == 9)
			PlaySoundPreset( SOUNDID_MAJOR_VICTORY );
		else
			PlaySoundPreset( SOUNDID_VICTORY );
		
		SetLCDGameMessage( SPT_TIF_VICTORY ); // Display victory message.
		SetLCDTimerCountMode( TM_LCD_TIMER_PLAYER1, 1 ); // Pause the timer so the player knows how long they took.
		InitSetupPhase( 0 ); // Don't wipe the board config so the player can observe the fruits of their labor.
	}
}

// Standard Callbacks
static unsigned short testCounter = 0;

GF_PREFIX int GAMEFUNC(OnGameLoaded)()
{
	// Can't load game because board is too small.
	if ( (TILESBOARDSIZE_DEFAULT > BOARDSIZEX) || (TILESBOARDSIZE_DEFAULT > BOARDSIZEY) )
		return 1;

	// Start in the setup phase.
	return InitSetupPhase( 1 ); // Always use a fresh configuration on initial game load.
}

GF_PREFIX void GAMEFUNC(OnButtonPressed)( int x, int y )
{
	if ( m_bIsSetup )
		PlaySoundPreset( SOUNDID_DENY );
	else
		MakeMove(x, y);
}

GF_PREFIX void GAMEFUNC(OnLCDButtonPressed)( int id )
{
	PlaySoundPreset( SOUNDID_DENY ); // No Extra Button Support
}

GF_PREFIX void GAMEFUNC(OnExit)( int reason )
{
	// Nothing to clean up on exit.
}


GF_PREFIX void GAMEFUNC(OnTimerFinished)( int id )
{
	if ( id == GT_RANDOMMOVES )
	{
		unsigned char moveCount = (unsigned char)GenerateRandomInt(0, 6);

		for (int i = 0; i < moveCount; i++)
			MakeMove(GenerateRandomInt(0, m_iBoardSize), GenerateRandomInt(0, m_iBoardSize));
	}
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
			case MSLOT_S_BOARDSIZE:  
				// MENU_VALUE = (MENU_VALUE - (A - 1)) % (MAX VALUE - (A - 1)) + A; where A is your minimum value for this kind of cycle.
				m_iBoardSize = (m_iBoardSize - 2) % (BOARDSIZEX - 2) + 3; // 3 is our minimum board size.

				RegisterMenuOptionWithIntParameter(SPT_OPTIONS_BOARDSIZE, IMAGEID_NONE, m_iBoardSize, MSLOT_S_BOARDSIZE); // Update board size display.
				break;
			case MSLOT_S_RANDOMNESS: 
				m_iStartRandomness = (m_iStartRandomness + 10) % (m_iRandomnessArrayLen * 10); // Cycle through randomness values on increments of 10.
				RegisterMenuOptionWithStringParameter(SPT_OPTIONS_RANDOMSTART, IMAGEID_NONE, m_iRandomnessArray[m_iStartRandomness / 10], MSLOT_S_RANDOMNESS);
				break;
			case MSLOT_S_RANDOMMOVES: 
				m_bRandomMoves = m_bRandomMoves ? 0 : 1;

				RegisterMenuOptionWithStringParameter(SPT_OPTIONS_RANDOM_MOVES, IMAGEID_NONE, m_bRandomMoves ? SPT_UTIL_ON : SPT_UTIL_OFF, MSLOT_S_RANDOMMOVES);
				break;
		}
	}
	else
	{
		switch (id)
		{
			case MSLOT_G_RESTART: InitGamePhase(); break;
			case MSLOT_G_RECONFIGURE: SetLCDGameMessage( SPT_GAMEMESSAGE_GENERICSETUP ); InitSetupPhase(0); break;
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

void TileFlip_RegisterGame( int id )
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_TILEFLIP;
	BoardGameInfoList[id].descriptionPreset = SPT_GD_TILEFLIP;
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