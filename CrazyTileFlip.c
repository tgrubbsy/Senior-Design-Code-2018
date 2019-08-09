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
//# Crazy Tile Flip, created by Noah Thering
//# ~crazy~ variant of Tile Flip to test game linking behavior.  DEFUNCT.
//###############################################################

#pragma once
#include "StandardGameIncludes.h"

// Ensures that all game functions are unique and won't generate linker errors if we switch strategies.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!
#define GAMEFUNC(FUNCNAME) CTF_##FUNCNAME

#define TILESBOARDSIZE 5

#define ONCOLOR GC_PINK
#define OFFCOLOR GC_GRAY
#define VICTORYCOLOR GC_YELLOW

// Game Specific Functions!  ALL OF THESE SHOULD BE DECLARED STATIC TO LIMIT THEM TO THE FILE SCOPE!

static short VictoryCheck()
{
	for ( int i = 0; i < TILESBOARDSIZE; i++ )
	{
		for ( int g = 0; g < TILESBOARDSIZE; g++ )
		{
			if ( GetButtonColorAtPos(g, i) == OFFCOLOR )
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
	// If we only flip squares to on, then there's a chance we've won the game!
	// Otherwise we don't need to peform a somewhat expensive check for victory.
	short allPositive = 1;

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
		if (xPos >= TILESBOARDSIZE || yPos >= TILESBOARDSIZE || xPos < 0 || yPos < 0)
			continue;

		// Flip the square!
		int color2Use = ONCOLOR;
		if ( GetButtonColorAtPos(xPos, yPos) == ONCOLOR )
		{
			// We're flipping to an off state so be sure to record that.
			allPositive = 0;
			color2Use = OFFCOLOR;
		}

		IlluminateButton(xPos, yPos, color2Use);
	}

	// Once we've won, turn all the squares yellow!
	// Once this state is acheived the board will stay this way until reset because
	// any click can only turn squares white which will instantly make the board
	// turn all yellow again since there will still be no gray squares.
	if (allPositive && VictoryCheck())
	{
		for ( int i = 0; i < TILESBOARDSIZE; i++ )
		{
			for ( int g = 0; g < TILESBOARDSIZE; g++ )
			{
				IlluminateButton(g, i, VICTORYCOLOR);
			}
		}
	}
}

// Standard Callbacks

GF_PREFIX int GAMEFUNC(OnGameLoaded)()
{
	SetBoardSize(TILESBOARDSIZE,TILESBOARDSIZE);

	for (int i = 0; i < TILESBOARDSIZE; i++)
	{
		for (int g = 0; g < TILESBOARDSIZE; g++)
		{
			IlluminateButton( i, g, OFFCOLOR );
		}
	}

	int moveCount = GenerateRandomInt(0, 10);

	// Make a randomly decided amount of random moves to generate a unique starting state that's also solvable.
	for (int i = 0; i < moveCount; i++)
		MakeMove(GenerateRandomInt(0, TILESBOARDSIZE), GenerateRandomInt(0, TILESBOARDSIZE));

	// Start a repeating timer that makes 2 moves every 10 seconds.
	StartTimer(0, 10000, 1);

	SetLCDGameMessage( SPT_TIF_TILEFLIPED );

	SetLCDTimerCountMode( TM_LCD_TIMER_PLAYER1, 2 );

	SetLCDScoreDisplayValue( TM_LCD_SCORE_PLAYER1, 1000 );

	return 0;
}

GF_PREFIX void GAMEFUNC(OnButtonPressed)( int x, int y )
{
	MakeMove(x, y);
}

GF_PREFIX void GAMEFUNC(OnLCDButtonPressed)( int id )
{
	return;
}

GF_PREFIX void GAMEFUNC(OnExit)( int reason )
{
	// Not supported yet!
}


GF_PREFIX void GAMEFUNC(OnTimerFinished)( int id )
{
	for (int i = 0; i < 2; i++)
		MakeMove(GenerateRandomInt(0, TILESBOARDSIZE), GenerateRandomInt(0, TILESBOARDSIZE));

	char buffer[128];

	G_sprintf(buffer, sizeof(buffer), "Detected Timer %d!\n", id);
	PrintDebugMessage(buffer);

	G_sprintf(buffer, sizeof(buffer), "Current millis is %lu\n", GetCurrentMillisecond());
	PrintDebugMessage(buffer);
}

GF_PREFIX void GAMEFUNC(OnLCDTimerHitZero)( int id )
{
}

GF_PREFIX void GAMEFUNC(OnMenuOptionSelected)( int id )
{
	
}

// Fires when the system has not received an input for a "long time"
GF_PREFIX void GAMEFUNC(OnIdle)()
{
	PrintDebugMessage("Called OnIdle!\n");
	SetColorMode(1);
}

// Fires when the system has received an input while idle.
GF_PREFIX void GAMEFUNC(OnWake)( int reason )
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

void CrazyTileFlip_RegisterGame( int id )
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_CRAZYTILEFLIP;
	BoardGameInfoList[id].descriptionPreset = SPT_GD_CRAZYTILEFLIP;
	BoardGameInfoList[id].gameIconID = IMAGEID_GAMEICON_CRAZYTILEFLIP;
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