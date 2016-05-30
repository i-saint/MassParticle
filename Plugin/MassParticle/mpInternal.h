#pragma once
#define mpImpl
#ifdef __linux__
    #define mpWithTBB
#endif

#define mpLog(...)
#define mpTraceFunc(...)

#include "mpFoundation.h"
#include "MassParticle.h"
