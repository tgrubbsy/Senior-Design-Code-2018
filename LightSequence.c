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
//# LightSequence.c, created by Noah Thering
//# Play a sequence of lights that the player must mimic!
//###############################################################
#pragma once
#include "StandardGameIncludes.h"

// Ensures that all game functions are unique and won't generate linker errors if we switch strategies.
// Assign a gamecode by changing the text before the ## symbols.  Use format [Three Letters]_
// EVERY GAME MUST HAVE A UNIQUE GAMECODE ASSIGNED!!!
#define GAMEFUNC(FUNCNAME) LSQ_##FUNCNAME
#define DEFAULTDIFFICULTY 0

// Timer IDs
#define GT_SHOWSEQUENCE 0
#define GT_DISCOLORBUTTON 1
#define GT_INTERMTIMER 2

// Menu IDs
// Setup
#define MSLOT_S_STARTGAME 0
#define MSLOT_S_DIFFICULTY 1
#define MSLOT_S_ALLOWREPEAT 2
#define MSLOT_S_DELAY 3

// Game
#define MSLOT_G_RESTART 0
#define MSLOT_G_RECONFIGURE 1

// Settings
static unsigned char m_iDifficulty;
static unsigned char m_bAllowSequenceRepeat;
static unsigned short m_iSequenceDelay;

#define CURRENT_BOARDSIZE (m_iDifficulty + 2)

#define DIFFICULTY_DEFAULT 1
#define SEQREPEAT_DEFAULT 0
#define SEQDELAY_DEFAULT 1000
#define SEQDELAY_INTERVAL 250
#define SEQDELAY_MAX 3000
#define OFFCOLOR GC_LIGHT_GRAY

#define COLOR_OFFSET 5

// Menu Display Arrays
const static unsigned char m_iDifficultyArray[] = { SPT_UTIL_LOW, SPT_UTIL_HIGH };
const static unsigned char m_iDifficultyArrayLen = sizeof(m_iDifficultyArray) / sizeof(unsigned char);

// Gameplay Variables
static unsigned long m_iRandomSequenceSeed;
static unsigned char m_bIsBuildingSequence;
static unsigned short m_iSequenceLength;
static unsigned short m_iSequenceCount;

static unsigned char m_bIsSetup;

static void ShowSequence();
static void DisplayChosenColor(unsigned char colorID);
static unsigned char XForColor(unsigned char colorID);
static unsigned char YForColor(unsigned char colorID);


// Game Specific Functions!  ALL OF THESE SHOULD BE DECLARED STATIC TO LIMIT THEM TO THE FILE SCOPE!
static unsigned short InitSetupPhase(unsigned short freshConfiguration)
{
	m_bIsSetup = 1;

	if (freshConfiguration) // Do initial setup stuff here that should only happen on fresh reloads.
	{
		// Create our "home" configuration.
		SetBoardSize(DEFAULTDIFFICULTY + 2, DEFAULTDIFFICULTY + 2);
		IlluminateBoard(OFFCOLOR);

		SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSETUP);

		// Only set our config variables if we want a fresh configuration, otherwise use the old ones for easy tweaking.
		m_iDifficulty = DEFAULTDIFFICULTY;
		m_bAllowSequenceRepeat = SEQREPEAT_DEFAULT;
		m_iSequenceDelay = SEQDELAY_DEFAULT;

		// Get a new random sequence seed.
		SeedRandomIntGeneratorRandomly();
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


	RegisterMenuOptionWithStringParameter(SPT_OPTIONS_DIFFICULTY, IMAGEID_NONE, m_iDifficultyArray[m_iDifficulty], MSLOT_S_DIFFICULTY);
	RegisterMenuOptionWithStringParameter(SPT_OPTIONS_ALLOWREPEATS, IMAGEID_NONE, m_bAllowSequenceRepeat ? SPT_UTIL_ON : SPT_UTIL_OFF, MSLOT_S_ALLOWREPEAT);
	RegisterMenuOptionWithIntParameter(SPT_OPTIONS_WAITDURATION, IMAGEID_NONE, m_iSequenceDelay, MSLOT_S_DELAY); // Update board size display.
																  // Make sure we don't show a timer.
	SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER1, 0);

	// Nothing that can cause errors here, really, so always return 0.
	return 0;
}

static void InitGamePhase()
{
	m_bIsSetup = 0;

	// Generate a new seed for our sequence.
	m_iRandomSequenceSeed = GenerateRandomInt(0, 0x7FFFFFFF);

	m_bIsBuildingSequence = 0;
	m_iSequenceLength = 3;

	// Play our start que
	PlaySoundPreset(SOUNDID_GAMESTART);

	// Init our LCD display values
	SetLCDTimerCountMode(TM_LCD_TIMER_PLAYER1, 0); // Timer is disabled
	SetLCDGameDisplayFormat(GDCONFIG_ONEPLAYER); // Just one player for tileflip
	SetLCDScoreDisplayValue(TM_LCD_SCORE_PLAYER1, 3); // We always start at 0 moves
	SetLCDTimerValue(TM_LCD_TIMER_PLAYER1, 0); // Timer starts at 0 seconds

	SetLCDGameMessage(SPT_GAMEMESSAGE_GENERICSTART);

	// Create our starting LED configuration.
	SetBoardSize(CURRENT_BOARDSIZE, CURRENT_BOARDSIZE);
	IlluminateBoard(OFFCOLOR);

	// Init our game phase menu.
	ClearAllMenuOptions();
	RegisterMenuOption(SPT_OPTIONS_RESTART, IMAGEID_NONE, MSLOT_G_RESTART);
	RegisterMenuOption(SPT_OPTIONS_RECONFIGURE, IMAGEID_NONE, MSLOT_G_RECONFIGURE);

	// Start with our sequence.
	ShowSequence();
}

static void PrepareForInput()
{
	// Reseed the generator and reset the count so we're ready for the player to repeat the sequence.
	m_bIsBuildingSequence = 0;
	m_iSequenceCount = 0;
	SeedRandomIntGenerator(m_iRandomSequenceSeed);
}

static void CheckInputSequence(unsigned char colorID)
{
	unsigned char seqColor = (unsigned char)GenerateRandomInt(0, CURRENT_BOARDSIZE*CURRENT_BOARDSIZE);

	if (colorID == seqColor)
	{
		DisplayChosenColor(colorID);
		m_iSequenceCount++;

		if (m_iSequenceCount == m_iSequenceLength)
		{
			m_iSequenceLength++;
			SetLCDScoreDisplayValue(TM_LCD_SCORE_PLAYER1, m_iSequenceLength); // Record our sequence length
			PlaySoundPreset(SOUNDID_MAJOR_VICTORY);
			m_bIsBuildingSequence = 1; // Let the game know we're about to start building our sequence and to not allow any more player input.
			StartTimer(GT_INTERMTIMER, 2000, 0); // Start our next sequence after a short intermission.
			StartTimer(GT_DISCOLORBUTTON, m_iSequenceDelay > 1000 ? 1000 : m_iSequenceDelay, 0);  // And make sure we turn off our last button pressed in the meantime.
		}
		else
		{
			// Turn our last pressed button to the default color after a short interval.
			// If the next button is pressed quick enough this will actually just restart the disable button timer.
			StartTimer(GT_DISCOLORBUTTON, m_iSequenceDelay / 2, 0); 
			PlaySoundPreset(SOUNDID_VICTORY);
		}
	}
	else
	{
		PlaySoundPreset(SOUNDID_FAILURE);
		SetLCDGameMessage(SPT_GAMEMESSAGE_LSQFAILURE);
		InitSetupPhase(0);
	}
}

static void ShowSequence()
{
	// Seed our random number generator with the same value every time we repeat the sequence.
	SeedRandomIntGenerator(m_iRandomSequenceSeed);
	m_bIsBuildingSequence = 1;
	m_iSequenceCount = 0;

	StartTimer(GT_SHOWSEQUENCE, m_iSequenceDelay / 2, 1);
}

static unsigned char XForColor(unsigned char colorID)
{
	unsigned char effectiveID = colorID;

	return colorID % CURRENT_BOARDSIZE;
}

static unsigned char YForColor(unsigned char colorID)
{
	unsigned char effectiveID = colorID;

	return colorID / CURRENT_BOARDSIZE;
}

static unsigned char ColorForXY(unsigned char xVal, unsigned char yVal)
{
	unsigned char colorVal = (xVal + yVal * CURRENT_BOARDSIZE);

	return colorVal;
}

static void DisplayChosenColor(unsigned char colorID)
{
	static unsigned char lastColor = GC_COLOR_MAX;

	// GC_COLOR_MAX is used to clear our last displayed color.
	if (colorID != GC_COLOR_MAX)
		IlluminateButton(XForColor(colorID), YForColor(colorID), colorID + COLOR_OFFSET);

	if (lastColor != GC_COLOR_MAX && lastColor != colorID)
		IlluminateButton(XForColor(lastColor), YForColor(lastColor), OFFCOLOR);

	lastColor = colorID;
}

// Standard Callbacks

GF_PREFIX int GAMEFUNC(OnGameLoaded)()
{
	// Start in the setup phase.
	return InitSetupPhase(1); // Always use a fresh configuration on initial game load.
}

GF_PREFIX void GAMEFUNC(OnButtonPressed)(int x, int y)
{
	if (m_bIsSetup || m_bIsBuildingSequence)
		PlaySoundPreset(SOUNDID_DENY);
	else
		CheckInputSequence(ColorForXY(x, y));
}

GF_PREFIX void GAMEFUNC(OnLCDButtonPressed)(int id)
{
	if (m_bAllowSequenceRepeat)
		ShowSequence();
}

GF_PREFIX void GAMEFUNC(OnExit)(int reason)
{
	// Make sure we've got a good seed going out.
	SeedRandomIntGeneratorRandomly();
}


GF_PREFIX void GAMEFUNC(OnTimerFinished)(int id)
{
	if (id == GT_SHOWSEQUENCE)
	{
		static unsigned char disableLastColor = 0;

		if (disableLastColor)
		{
			disableLastColor = 0;
			DisplayChosenColor(GC_COLOR_MAX);

			// This is our last node and we just disabled it!
			if ( m_iSequenceCount >= m_iSequenceLength )
			{
				KillTimer(GT_SHOWSEQUENCE); // No more colors to show.
				PrepareForInput();
				return;
			}
		}
		else
		{
			// Make sure to only generate one random int per timer fire, to keep the sequence consistent.
			unsigned char newColor = (unsigned char)GenerateRandomInt(0, CURRENT_BOARDSIZE*CURRENT_BOARDSIZE);

			DisplayChosenColor(newColor);
			PlaySoundPreset(SOUNDID_GAMESTART);
			m_iSequenceCount++;
			disableLastColor = 1;
		}
	}
	else if (id == GT_DISCOLORBUTTON)
	{
		DisplayChosenColor(GC_COLOR_MAX); // Just turn off whatever color was last displayed.
	}
	else if (id == GT_INTERMTIMER)
	{
		ShowSequence(); // Time to show our next sequence!
	}
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
		case MSLOT_S_DIFFICULTY:
			m_iDifficulty = (m_iDifficulty + 1) % m_iDifficultyArrayLen; // 2 is the smallest grid.

			RegisterMenuOptionWithStringParameter(SPT_OPTIONS_DIFFICULTY, IMAGEID_NONE, m_iDifficultyArray[m_iDifficulty], MSLOT_S_DIFFICULTY);
			break;
		case MSLOT_S_ALLOWREPEAT:
			m_bAllowSequenceRepeat = m_bAllowSequenceRepeat ? 0 : 1;

			RegisterMenuOptionWithStringParameter(SPT_OPTIONS_ALLOWREPEATS, IMAGEID_NONE, m_bAllowSequenceRepeat ? SPT_UTIL_ON : SPT_UTIL_OFF, MSLOT_S_ALLOWREPEAT);
			break;
		case MSLOT_S_DELAY:
			// MENU_VALUE = (MENU_VALUE - (A - 1)) % (MAX VALUE - (A - 1)) + A; where A is your minimum value for this kind of cycle.
			m_iSequenceDelay = (m_iSequenceDelay - (SEQDELAY_INTERVAL)) % (SEQDELAY_MAX - (SEQDELAY_INTERVAL)) + SEQDELAY_INTERVAL * 2; // 3 is our minimum board size.

			RegisterMenuOptionWithIntParameter(SPT_OPTIONS_WAITDURATION, IMAGEID_NONE, m_iSequenceDelay, MSLOT_S_DELAY); // Update board size display.
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

void LightSequence_RegisterGame(int id)
{
	BoardGameInfoList[id].printNamePreset = SPT_GT_LIGHTSEQUENCE;
	BoardGameInfoList[id].descriptionPreset = SPT_GD_LIGHTSEQUENCE;
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