#pragma once

#include "MyTypes.h"

bool HardPatch_Init();
void HardPatch_ModulePreLoad();
void HardPatch_ModulePostLoad();
void HardPatch_AppStart();