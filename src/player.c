#include "pd_api.h"

#include "GLOBALS.h"
#include "entity.h"
#include "player.h"

game_sprite Player;
LCDBitmap* playerBitmap;

void createPlayer()
{
    pd->system->logToConsole("Creating Player sprite");
    playerBitmap = loadImageAtPath("images/playerSprite64x64.png");

    Player.sprite = sprite->newSprite();
    sprite->setImage(Player.sprite, playerBitmap, kBitmapUnflipped);
    sprite->addSprite(Player.sprite);
};