#include <stdio.h>
#include <stdlib.h>

#include "GLOBALS.h"
#include "game.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32 arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	if ( event == kEventInit )
	{
		setPDPtr(playdate);
		// NOTE: If you set an update callback in the kEventInit handler, the system assumes the game is pure C and doesn't run any Lua code in the game
		playdate->display->setRefreshRate(0);
		setupGame();
		playdate->system->setUpdateCallback(update, playdate);
	}
	
	return 0;
}