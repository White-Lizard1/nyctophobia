#pragma once

#include "snooze.h"
#include "ui.h"
#include "e4defs.h"
#include <windows.h>
#include "sloputils.h"
#include "nfd/include/nfd.h"
#include "snze.h"

bool sessionPaused = 0; // Should be switched to 1 later
Uint64 session_start_time = 0;
Uint64 last_frame = 0;

Uint64 counter_paused = 0;
Uint64 counter_unpaused = 0;

Uint64 staticPerfFreq = 1;

void e4_updateFrametime() {
    Uint64 now = SDL_GetPerformanceCounter();
    if(sessionPaused) {
        counter_paused = counter_paused + now - last_frame;
    } else {
        counter_unpaused = counter_unpaused + now - last_frame;
    }
    SNZ_ASSERT(session_start_time + counter_paused + counter_unpaused==now, "Timing failed somehow.");
    last_frame = now;
}
