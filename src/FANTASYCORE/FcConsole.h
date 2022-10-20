#pragma once

#include "skip.h"

extern char g_szConsoleString[];
extern CSkipper g_Skip;

void ProcessConsoleString(const char *pString);
void InputKeyboard(unsigned char cInput);
void ToggleShowConsole();
