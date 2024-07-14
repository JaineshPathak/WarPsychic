#pragma once

#include "CoreMinimal.h"
#include <Engine/Engine.h>

#define DebugLog(k, screenTime, textColor, debugText) if(GEngine) { GEngine->AddOnScreenDebugMessage(k, screenTime, textColor, TEXT(debugText)); }
#define DebugLogVal(k, screenTime, textColor, debugText, debugVal) if(GEngine) { GEngine->AddOnScreenDebugMessage(k, screenTime, textColor, FString::Printf(TEXT(debugText @ "%i"), debugVal)); }