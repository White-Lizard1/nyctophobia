#pragma once

#define Total_Channels 64
#define Total_Samples 64
#define Total_Input_Devices 16
#define Total_Output_Devices 16

bool PRINT_FRAMETIME = 0;

bool goofyChannelIdArray[Total_Channels] = { 0 };

bool menuLayerOpen[8] = { 0 };

enum e4_Mode {
    e4_DEFAULT_MODE = 0,
    e4_SESSION_MODE = 1,
};

const char* e4_modeStr(enum e4_Mode mode) {
    if (mode==e4_DEFAULT_MODE) {
        return("Default");
    } else if (mode==e4_SESSION_MODE) {
        return("Session");
    }
    else {
        return("Erratic");
    }
}

float e4_clamp(float var, float st, float en) {
    if((var >= st) & (var<= en)) {
        return(var);
    } else if (var < st) {
        return(st);
    } else {
        return(en);
    }
}
