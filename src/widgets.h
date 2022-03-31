#ifndef SRC_WIDGETS_H
#define SRC_WIDGETS_H

#include "moby.h"

enum GameIDs {
    GAME_INVALID,
    GAME_RC1,
    GAME_RC2,
    GAME_RC3,
    GAME_RC4,
    GAMES_COUNT = 4
};
typedef unsigned int GameID;

//Load Moby oClass->Name mapping strings from file
void loadStrings(void);

//Unload Moby oClass->Name mapping strings from memory
void unloadStrings(void);

//Get a string containing the name of a game based on ID
char const* GetGameNameFromID(GameID id);

//Get a string containing the name of an oClass based on ID
char const* GetOClassStringForID(uint16_t oClass, uint32_t game);

//Calls all routines to dump a moby's state as ImGui objects
void AddOG3MobyWidgets(Moby* m, GameID game);
void AddRC4MobyWidgets(RC4Moby* m);

#endif