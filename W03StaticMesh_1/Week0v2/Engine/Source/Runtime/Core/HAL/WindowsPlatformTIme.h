#pragma once
#include "PlatformType.h"

class FWindowsPlatformTime
{
public:
    static double GSecondsPerCycle; // 0
    static bool bInitialized; // false

    static void InitTiming();

    static float GetSecondsPerCycle();

    static uint64 GetFrequency();

    static double ToMilliseconds(uint64 CycleDiff);

    static uint64 Cycles64();
};