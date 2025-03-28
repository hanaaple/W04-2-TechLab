#pragma once
#include "HAL/WindowsPlatformTIme.h"

struct TStatId
{
};

typedef FWindowsPlatformTime FPlatformTime;

class FScopeCycleCounter
{
public:
    FScopeCycleCounter(TStatId StatId)
        : StartCycles(FPlatformTime::Cycles64())
        , UsedStatId(StatId)
    {
    }

    ~FScopeCycleCounter()
    {
        Finish();
    }

    uint64 Finish()
    {
        const uint64 EndCycles = FPlatformTime::Cycles64();
        const uint64 CycleDiff = EndCycles - StartCycles;

        return CycleDiff;
    }

private:
    uint64 StartCycles;
    TStatId UsedStatId;
};

