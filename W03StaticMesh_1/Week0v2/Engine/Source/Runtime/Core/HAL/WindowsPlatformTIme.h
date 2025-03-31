#pragma once
#include "PlatformType.h"
#include "Core/Container/String.h"
#include "Core/Container/Map.h"

class FWindowsPlatformTime
{
public:
    static double GSecondsPerCycle; // 0
    static bool bInitialized; // false
    static TMap<FString, double> GElapsedMap;

    static void InitTiming();

    static float GetSecondsPerCycle();

    static uint64 GetFrequency();

    static double ToMilliseconds(uint64 CycleDiff);

    static uint64 Cycles64();
};