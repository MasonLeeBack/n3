#pragma once

#ifdef _BSUI_TOOL
	#define MENU_INPUT_A							VK_RETURN			
	#define MENU_INPUT_B							VK_B			
	#define MENU_INPUT_X							VK_X			
	#define MENU_INPUT_Y							VK_Y			
	#define MENU_INPUT_BLACK						VK_BLACK		
	#define MENU_INPUT_WHITE						VK_WHITE		
	#define MENU_INPUT_UP							VK_UP		
	#define MENU_INPUT_DOWN							VK_DOWN		
	#define MENU_INPUT_LEFT							VK_LEFT		
	#define MENU_INPUT_RIGHT						VK_RIGHT		
	#define MENU_INPUT_START						VK_START		
	#define MENU_INPUT_BACK							VK_BACK		
	#define MENU_INPUT_LTHUMB						VK_LTHUMB	
	#define MENU_INPUT_RTHUMB						VK_RTHUMB	
	#define MENU_INPUT_LTRIGGER						VK_LTRIGGER	
	#define MENU_INPUT_RTRIGGER						VK_RTRIGGER	
	#define MENU_INPUT_LSTICK						VK_LSTICK	
	#define MENU_INPUT_RSTICK						VK_RSTICK	
#else

#include "Input.h"

	#define MENU_INPUT_A							PAD_INPUT_A			
	#define MENU_INPUT_B							PAD_INPUT_B			
	#define MENU_INPUT_X							PAD_INPUT_X			
	#define MENU_INPUT_Y							PAD_INPUT_Y			
	#define MENU_INPUT_BLACK						PAD_INPUT_BLACK		
	#define MENU_INPUT_WHITE						PAD_INPUT_WHITE		
	#define MENU_INPUT_UP							PAD_INPUT_UP		
	#define MENU_INPUT_DOWN							PAD_INPUT_DOWN		
	#define MENU_INPUT_LEFT							PAD_INPUT_LEFT		
	#define MENU_INPUT_RIGHT						PAD_INPUT_RIGHT		
	#define MENU_INPUT_START						PAD_INPUT_START		
	#define MENU_INPUT_BACK							PAD_INPUT_BACK		
	#define MENU_INPUT_LTHUMB						PAD_INPUT_LTHUMB	
	#define MENU_INPUT_RTHUMB						PAD_INPUT_RTHUMB	
	#define MENU_INPUT_LTRIGGER						PAD_INPUT_LTRIGGER	
	#define MENU_INPUT_RTRIGGER						PAD_INPUT_RTRIGGER	
	#define MENU_INPUT_LSTICK						PAD_INPUT_LSTICK	
	#define MENU_INPUT_RSTICK						PAD_INPUT_RSTICK	
#endif