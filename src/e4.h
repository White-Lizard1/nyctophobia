#pragma once
#include "snooze.h"
#include "ui.h"
#include "e4defs.h"
#include <windows.h>
#include "sloputils.h"

Uint32 start_time;
Uint32 last_frame;

bool fileDropdownSelected = 0;

int e4_openedOutputDevices[E4_OUT_CHANNELS] = { 0 };
Uint32 e4_numOpenedOutputDevices = 0;

const char* placeholderProjectName = "placeholder";

//Right now...what to work on? Live project vs. fixed project..difference?
/*
PRIORITIES
Functions to create a new WAV file, open a file, write to that file
Functions to create a project w/ WAV file(s) associated and w/ other project necessities
i.e. links to samples, name of project, dumb stuff like that...



*/


//Make sure this whole struct thing works? e4FIXME
//Also make sure no input/output infinite loop catastrophe

// Debugging can be set true/false in sloputils

//   Should be consistent everywhere but: output loaded before input, output
// allocated 0-7 in main module, input allocated 8-15



void e4_reloadFilepathBin(const char* folderPath) {
    printf("Loading filepath bin\n");
    // e4FIXME NEED NEED NEED TO COME BACK TO FREE MEMORY
    //e4_freeBin(&filepathBin);

    //Add the filepaths

    loadFilenamesToMainBin(folderPath);
    
    //Free memory at some point
}

typedef struct {
    bool outputsTo[E4_NUM_NODES];
    bool inputsFrom[E4_NUM_NODES];
    enum e4_Nodetype nodetype;
    FILE* audioFile;
    SDL_AudioSpec desiredAudioSpec;
    SDL_AudioSpec* obtainedAudioSpec;
    int nodeID;
    const char* name;
} e4_AudioNode;

// Make sure file works here / More stuff to do here
e4_AudioNode e4_createAudioNode() {
    e4_AudioNode tempNode;
    for (int i = 0; i < E4_NUM_NODES; i++) {
        tempNode.outputsTo[i] = 0;
        tempNode.inputsFrom[i] = 0;
        tempNode.nodetype = e4UNSET;
        tempNode.audioFile = NULL;
        tempNode.name = "NA";
        tempNode.nodeID = i;
    }
    return tempNode;
}

e4_AudioNode e4_mainModule[E4_NUM_NODES];
void e4_createMainModule() {
    for (int i = 0; i < E4_NUM_NODES; i++) {
        e4_mainModule[i] = e4_createAudioNode();
    }
    e4_debug("Main module created");
}

//flags for debugging
bool PRINT_FRAMETIME = 0;
//end flags

void e4_printFrametime() {
    if (PRINT_FRAMETIME) {
        Uint32 now = SDL_GetTicks();
        printf("Frame length: %d milliseconds\n", (now - last_frame));
        printf("Elapsed time: %.3f seconds\n", (now - start_time) / 1000.0f);
        last_frame = now;
    }
}


void e4_loadTestbox() {
    snzu_boxNew("testbox");
    snzu_boxSetSizePctParent(0.5, SNZU_AX_X);
    snzu_boxSetSizePctParent(0.5, SNZU_AX_Y);
    snzu_boxSetColor(ui_colgray);
    e4_debug("Loaded testbox");
}
void e4_loadInputTitleBox() {
    snzu_boxNew("titleinput");
    snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack,"Input devices:");
    snzu_boxSetSizeFitText(0);
}
void e4_loadOutputTitleBox() {
    snzu_boxNew("titleoutput");
    snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack,"Output devices:");
    snzu_boxSetSizeFitText(0);
}
void e4_loadFilesTitleBox() {
    snzu_boxNew("titlemusicfiles");
    snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack,"Available files:");
    snzu_boxSetSizeFitText(0);
}

bool e4_fileFails(FILE* file) {
    if (file == NULL) {
        return true;
    } else {
        return false;
    }
}

//singleChannel = callback audio from only the first channel found
//Remember if you want to use multiple modules to implement that
void e4_nodeCallbacks(int nodeID, Uint8* stream, int len, bool singleChannel) {
    bool callbackSuccess = false;
    for (int i = 0; i < E4_NUM_NODES; i++) {
        if (e4_mainModule[nodeID].inputsFrom[i]) {
            if (e4_mainModule[i].outputsTo[nodeID]) {
                if (e4_mainModule[i].nodetype==e4OUTPUT) {
                    printf("e4_nodeCallbacks error: output device received callback\n");
                } else if (e4_mainModule[i].nodetype==e4INPUT){
                    printf("e4_nodeCallbacks error: input devices not implemented\n");
                } else if (e4_mainModule[i].nodetype==e4FILE){
                    //Implement this
                    if (e4_fileFails(e4_mainModule[i].audioFile)) {
                        printf("e4_nodeCallbacks: accessing audio file failed\n");
                        break;
                    }
                    size_t bytesRead = fread(stream, 1, len, e4_mainModule[i].audioFile);
                    if (bytesRead < (size_t)len) {
                        memset(stream + bytesRead, 0, len - bytesRead);
                        //Perhaps implement something for when we encounter silence?
                        printf("e4_nodeCallbacks warning: added silence to incomplete callback\n");
                    }
                    callbackSuccess = true;                   
                    if (singleChannel) {
                        break;
                    }
                } else if (e4_mainModule[i].nodetype==e4PURE) {
                    printf("e4_nodeCallbacks error: pure nodes not yet implemented\n");
                } else {
                    printf("e4_nodeCallbacks error: nodetype unset or erratic\n");
                }
            } else {
                //May desire outputting nodes 
                printf("e4_nodeCallbacks warning: one-sided link\n");
            }
        }
    }
    if (!callbackSuccess) {
        printf("e4_nodeCallbacks error: callback streamed nothing");
    }
}

//Must open the audio device with said userdata, e4FIXME
void e4_outputDeviceCallback(void* userdata, Uint8* stream, int len) {
    int* nodeIDP = (int*)userdata;
    int nodeID = *nodeIDP;
    e4_nodeCallbacks(nodeID, stream, len, 1);
}

void e4_Open_Playback_Device(const char* deviceName) {
    //Code here may be a little shoddy, make sure it fully works
    if (e4_numOpenedOutputDevices < E4_OUT_CHANNELS) {
        int i = 0;
        while (e4_openedOutputDevices[i] == 0) {
            i++;
        }
        //Add it to module
        e4_mainModule[i].nodetype = e4OUTPUT;
        e4_mainModule[i].nodeID = i;
        //Can change these later to be able to specialize more
        e4_mainModule[i].desiredAudioSpec.callback = e4_outputDeviceCallback;
        e4_mainModule[i].desiredAudioSpec.freq = 44100;
        e4_mainModule[i].desiredAudioSpec.channels = 2;
        e4_mainModule[i].desiredAudioSpec.samples = 4096;
        e4_mainModule[i].desiredAudioSpec.userdata = &e4_mainModule[i].nodeID;

        e4_openedOutputDevices[i] = SDL_OpenAudioDevice(deviceName, 0, &e4_mainModule[i].desiredAudioSpec, e4_mainModule[i].obtainedAudioSpec, 0);
        e4_numOpenedOutputDevices++;
    } else {
        //Make better error system
        printf("e4: Can't open new playback device, already max (8) open!\n");
    }
    //write stuff here
}

void e4_Check_For_Removed_Playback_Devices() {
    // Perhaps implement check that the removed device is output & not input, e4FIXME
    // Only works for output at the moment.
    if (!(e4_removedDeviceID==-1)) {
        for(int i = 0; i < E4_OUT_CHANNELS; i++) {
            if (e4_removedDeviceID==e4_openedOutputDevices[i]) {
                SDL_CloseAudioDevice(e4_removedDeviceID);
                e4_numOpenedOutputDevices--;
                e4_openedOutputDevices[i] = 0;
                e4_removedDeviceID = -1;
            }
            //Maybe could do better error msging here, e4FIXME
            if ((i == (E4_OUT_CHANNELS-1))&!(e4_removedDeviceID==-1)) {
                printf("e4: Warning/error, expected an output device to be removed but wasn't properly dealt with.\n");
            }
        }
    }
}

// Is called to reload the nodes connected to SDL output devices.
// Perhaps implement error handling here?
void e4_reloadOutputNodes() {
    int numOutputDevices = SDL_GetNumAudioDevices(0);
    if(numOutputDevices > 8) {
        numOutputDevices = 8;
    }

    for (int i = 0; i < 8; i++) {
        e4_mainModule[i] = e4_createAudioNode();
    }

    for (int i = 0; i < numOutputDevices; i++) {
        const char* deviceName = SDL_GetAudioDeviceName(i, 0);
        e4_mainModule[i].nodetype = e4OUTPUT;
        SDL_GetAudioDeviceSpec(i, 0, &e4_mainModule[i].desiredAudioSpec);
        //This node id is kinda useless... you need the ID to access this in the array anyway
        //Maybe it will have some strange edge use where we don't know the node we're using?
        e4_mainModule[i].nodeID = i;
        e4_mainModule[i].name = deviceName;
    }
}

void e4_reloadInputNodes() {
    int numInputDevices = SDL_GetNumAudioDevices(1);
    if(numInputDevices > 8) {
        numInputDevices = 8;
    }

    for (int i = 0; i < 8; i++) {
        e4_mainModule[i+8] = e4_createAudioNode();
    }

    for (int i = 0; i < numInputDevices; i++) {
        const char* deviceName = SDL_GetAudioDeviceName(i, 1);
        e4_mainModule[i+8].nodetype = e4INPUT;
        SDL_GetAudioDeviceSpec(i, 1, &e4_mainModule[i+8].desiredAudioSpec);
        //This node id is kinda useless... you need the ID to access this in the array anyway
        //Maybe it will have some strange edge use where we don't know the node we're using?
        e4_mainModule[i+8].nodeID = i+8;
        e4_mainModule[i+8].name = deviceName;
        e4_debug("Set an output name:");
        //printf(deviceName);
    }
}

void e4_loadOutputDeviceSnzBoxes() {
    e4_debug("Starting output device load");

    for(int i = 0; i < E4_OUT_CHANNELS; i++) {
        if (e4_mainModule[i].nodetype==e4OUTPUT) {
            snzu_boxNew(e4_mainModule[i].name);
            snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack, e4_mainModule[i].name);
            snzu_boxSetSizeFitText(0);

            snzu_Interaction* inter = SNZU_USE_MEM(snzu_Interaction, "inter");
            snzu_boxSetInteractionOutput(inter, SNZU_IF_MOUSE_BUTTONS | SNZU_IF_HOVER);
            bool* const selected = SNZU_USE_MEM(bool, "selected");
            
            if(inter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
                *selected = !*selected;
            }
            if(*selected) {
                snzu_boxSetBorder(3, ui_colblack);
            }
        } else if (!(e4_mainModule[i].nodetype==e4UNSET)) {
            printf("e4: Error: Node allocated for output has wrong nodetype\n");
        }
    }
    e4_debug("Output device loading ended");   
}

void e4_loadInputDeviceSnzBoxes() {
    e4_debug("Starting input device load");
    //printf(e4_mainModule[8].name);
    for(int i = 0; i < E4_IN_CHANNELS; i++) {
        if (e4_mainModule[i+8].nodetype==e4INPUT) {
            //printf("Creating node for ");
            //printf(e4_mainModule[i+8].name);
            //printf("\n");
            snzu_boxNew(e4_mainModule[i+8].name);
            snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack, e4_mainModule[i+8].name);
            snzu_boxSetSizeFitText(0);

            snzu_Interaction* inter = SNZU_USE_MEM(snzu_Interaction, "inter");
            snzu_boxSetInteractionOutput(inter, SNZU_IF_MOUSE_BUTTONS | SNZU_IF_HOVER);
            bool* const selected = SNZU_USE_MEM(bool, "selected");
            
            if(inter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
                *selected = !*selected;
            }
            if(*selected) {
                snzu_boxSetBorder(3, ui_colblack);
            }
        } else if (!(e4_mainModule[i+8].nodetype==e4UNSET)) {
            printf("e4: Error: Node allocated for input has wrong nodetype");
        }
    }
    e4_debug("Input device loading ended");    
}
// void e4_Set_File_As_Node(const char* filename, int nodeNum) {



void e4_openLiveSession() {

}

// writing this function is a big work in progress

// void e4_createNewLiveSession(const char* liveSessionName) {
//     // Check if session name is same, append if it is
//     const char* finalSessionName; //syntax, set here
//     // Create WAV file for channel 1
//     // Create settings doc
//     e4_openLiveSession(finalSessionName);
// }

// This should be obsoleted
void e4_init() {
    e4_createMainModule();
    e4_reloadFilepathBin("res\\music");
    e4_reloadOutputNodes();
    e4_reloadInputNodes();
}

void e4_main() {
    // This should be exactly one loop of the program!
    /*
    Pseudocode = in what order should things be handled?:
        snz updating, if necessary

        File managing: opening, creating, closing projects.
        // if project closed, make sure to clear its interactions to avoid weird stuff

        Lesser file managing.

        Utility stuff

        Editing of the file itself.

        

    */

    // Snz updating

    //Caption bar
    snzu_boxNew("Caption");
    snzu_boxSetSizePctParent(0.05,SNZU_AX_Y);
    snzu_boxSetSizePctParent(1,SNZU_AX_X);
    snzu_boxSetColor(ui_colgray);
    snzu_boxScope() {
        snzu_boxNew("FileDropdown");
        snzu_boxSetSizePctParent(1,SNZU_AX_Y);
        snzu_boxSetSizePctParent(0.1,SNZU_AX_X);
        snzu_boxSetColor(HMM_V4(0.75,0.75,0.75,1));
        snzu_boxSetDisplayStr(&ui_labelFont,ui_colblack, "File");
        snzu_Interaction* fileDropdownInter = SNZU_USE_MEM(snzu_Interaction, "fileDropdownInter");
        snzu_boxSetInteractionOutput(fileDropdownInter, SNZU_IF_HOVER | SNZU_IF_MOUSE_BUTTONS);
        snzu_boxScope() {
            if (fileDropdownInter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
                fileDropdownSelected = !fileDropdownSelected;
            }
            if (fileDropdownSelected) {
                snzu_boxNew("fileDropdownMenu");
                snzu_boxSetSizePctParent(5,SNZU_AX_Y);
                snzu_boxSetSizePctParent(3,SNZU_AX_X);
                snzu_boxSetSizeFromEndAx(SNZU_AX_Y,(snzu_boxGetSize().Y)*0.8);
                snzu_boxSetColor(HMM_V4(0.75,0.75,0.75,1));
                snzu_boxSetBorder(1,ui_colblack);
                snzu_boxScope() {
                    snzu_boxNew("newFile");
                    snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack,"New File");
                    snzu_boxSetSizeFitText(5);
                    // create new Windows file here


                    snzu_boxNew("exit");
                    snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack,"Exit");
                    snzu_boxSetSizeFitText(5);
                    
                }
                snzu_boxOrderChildrenInRowRecurse(5,SNZU_AX_Y,SNZU_ALIGN_LEFT);
            }
        }
    }
    // set place and size here...
    // snzu_Interaction* captionInter = SNZU_USE_MEM(snzu_Interaction, "captionInter");
    // return(captionInter);
    // if() {
    //     snzu_Interaction* fileDropdownInter = e4_loadFileDropdownBar();
    // }
    
    // File managing
    // opening a new file
    // Don't know how I will implement the menu yet
    // need help with the if statements here
    // Project mode goes here
    // if (....) {
    //     // remember this is bool & fix
    //     e4_createNewLiveSession(placeholderProjectName);

    // }

    // Utility stuff

    // Editing of the file itself



}