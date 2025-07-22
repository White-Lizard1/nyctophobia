
#include "snooze.h"
#include "ui.h"
#include "ser.h"
#include <stdio.h>
#include "e4.h"
#include <windows.h>

snzu_Instance main_uiInstance = { 0 };
snz_Arena main_fontArena = { 0 };
snz_Arena main_lifetimeArena = { 0 };

//For opening:
// cd C:/Users/nolan/Documents/codestuff/e4

void main_init(snz_Arena* scratch, SDL_Window* window) {
    SNZ_ASSERT(window || !window, "???");

    main_uiInstance = snzu_instanceInit();
    snzu_instanceSelect(&main_uiInstance);

    main_fontArena = snz_arenaInit(10000000, "main_fontArena");
    main_lifetimeArena = snz_arenaInit(10000000, "main_lifetimeArena");

    ui_init(&main_fontArena, scratch);
    e4_init();
    
}

void main_loop(float dt, snz_Arena* frameArena, snzu_Input og_frameInputs, HMM_Vec2 og_screenSize) {
    sloppySloppySloprena = frameArena;

    // See if frame can be embedded within e4_main loop to only be called on some loops?
    // So it doesn't slow down the faster processing rate of e4_main
    snzu_frameStart(frameArena, og_screenSize, dt);
    e4_main();
    
    
    
    
    

    //Draw calls
    HMM_Mat4 uiVP = HMM_Orthographic_RH_NO(0, og_screenSize.X, og_screenSize.Y, 0, 0.0001, 100000);
    snzu_frameDrawAndGenInteractions(og_frameInputs, uiVP);
    
    //Report frame length
    
}

int main() {
    snz_main("e4", NULL, main_init, main_loop);
    return EXIT_SUCCESS;
}
