#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

bool debugging = false;
typedef struct {
    wchar_t deviceNames[8*MAX_PATH];
    int nameLens[8];
} e4_FilepathBin;

e4_FilepathBin e4_mainFilepathBin;

void loadFilenamesToMainBin(const char *folderPath) {
    
    printf("Starting to load filenames...\n");
    WIN32_FIND_DATA findData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // Build the search path with wildcard (*.*)
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*.*", folderPath);

    hFind = FindFirstFile(searchPath, &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FindFirstFile failed (%lu)\n", GetLastError());
        return;
    }
    printf("Got to this point...\n");
    int filenum = 0;
    do {
        // Skip "." and ".." entries
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;

        // Check if it's a regular file
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (filenum < 8) {
                for (size_t i = 0; i < strlen(findData.cFileName); i++) {
                    e4_mainFilepathBin.deviceNames[filenum*MAX_PATH+i] = findData.cFileName[i];
                }
                e4_mainFilepathBin.nameLens[filenum] = strlen(findData.cFileName);
                filenum++;
            }
            printf("File: %s\\%s\n", folderPath, findData.cFileName);
            
        }
    } while (FindNextFile(hFind, &findData) != 0);

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        fprintf(stderr, "FindNextFile error (%lu)\n", GetLastError());
    }

    FindClose(hFind);
}

void e4_debug(const char* message) {
    if (debugging) {
        printf("e4 debug: ");
        printf(message);
        printf("\n");
    }
}

void wideprint(wchar_t* name, int length, int place) {
    for(int i = 0; i < length; i++) {
        wprintf(L"%lc",name[place*MAX_PATH + i]);
    }
}