#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "snooze.h"


// !!!SLOP SLOP SLOP!!! SLOP ALERT!!! SLOP SLOP SLOP SLOP SLOP

bool debugging = false;

snz_Arena* sloppySloppySloprena = NULL;
const char* int_to_string(int value) {
    return snz_arenaFormatStr(sloppySloppySloprena, "%d", value);
}


void copy_filename_after_backslash(char buffer[64], const char* filepath) {
    // Find the last backslash in the filepath
    const char* last_backslash = strrchr(filepath, '\\');

    // Pointer to the part after the last backslash, or start of string if none found
    const char* filename = last_backslash ? last_backslash + 1 : filepath;

    // Copy to buffer safely, leaving room for null terminator
    strncpy(buffer, filename, 63);
    buffer[63] = '\0';  // Ensure null termination
}

void e4_debug(const char* message) {
    if (debugging) {
        printf("e4 debug: ");
        printf(message);
        printf("\n");
    }
}

