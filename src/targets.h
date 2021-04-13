#ifndef SRC_TARGETS_H
#define SRC_TARGETS_H

enum Games {
    GAME_INVALID,
    GAME_RC1,
    GAME_RC2,
    GAME_RC3,
    GAME_RC4,
    GAMES_COUNT = 4
};

#include "targets/target.h"
#include "targets/dummy.h"
#include "targets/ps2emu.h"

#endif