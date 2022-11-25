#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "types.h"

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
		playdate->system->setUpdateCallback(update, playdate);
		setupGame();
	}
	
	return 0;
}

