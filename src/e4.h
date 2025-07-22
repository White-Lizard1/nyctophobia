#pragma once
#include "e4audio.h"

// Random opp bugs which I dont want to deal with yet
// NFD opening crashes if file not selected
// int_to_string

nfdchar_t* tempSamplePath = NULL;
nfdchar_t* workingSamplePath = NULL;

float channelBoxScroll = 0;
float channelsVisible = 5;
float pastSecondsVisible = 1;
float futureSecondsVisible = 10;

HMM_Vec4 pause_color = {0};


// Glitching at last second - low priority fix.
float e4_audBlockPctStart(e4_QueueFlag* queueFlag) {
    return(e4_clamp((pastSecondsVisible+((float)queueFlag->counterToQueueAt - (float)counter_unpaused)/(float)staticPerfFreq)/(pastSecondsVisible+futureSecondsVisible), 0, 1));
}

float e4_audBlockPctEnd(e4_QueueFlag* queueFlag) {
    return(e4_clamp((pastSecondsVisible+((float)queueFlag->totalBytes)/(44100*2*2)+((float)queueFlag->counterToQueueAt-(float)counter_unpaused)/((float)staticPerfFreq))/(pastSecondsVisible+futureSecondsVisible),0,1));
}

void e4_initSessionMode() {
    e4_mode = e4_SESSION_MODE;
}

// this shit hella scuffed..
void e4_loadBox_NewSession_File_Caption() {
    snzu_boxNew("newSession");
    snzu_boxSetDisplayStr(&ui_labelFont, ui_colred,"New Session");
    snzu_boxSetSizeFitText(5);
    // create new Windows file here
    snzu_Interaction* newSessionInter = SNZU_USE_MEM(snzu_Interaction, "newSessionInter");
    snzu_boxSetInteractionOutput(newSessionInter, SNZU_IF_HOVER | SNZU_IF_MOUSE_BUTTONS);
}
void e4_loadBox_OpenSession_File_Caption() {
    snzu_boxNew("openSession");
    snzu_boxSetDisplayStr(&ui_labelFont, ui_colred,"Open Session");
    snzu_boxSetSizeFitText(5);
    snzu_Interaction* openSessionInter = SNZU_USE_MEM(snzu_Interaction, "openSessionInter");
    snzu_boxSetInteractionOutput(openSessionInter, SNZU_IF_HOVER | SNZU_IF_MOUSE_BUTTONS);
    if (openSessionInter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
        nfdchar_t* path = NULL;
        nfdresult_t result = NFD_OpenDialog(&path, NULL, 0, NULL);
        if (result != NFD_OKAY) {
            printf("NFD stuff is not okay :(\n");
        }
        printf(path);
        free(path);
        // fixme this gets changed with my improvements in menus
        e4_initSessionMode();
        
    }
}
void e4_loadBox_Exit_File_Caption() {
    snzu_boxNew("exit");
    snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack,"Exit");
    snzu_boxSetSizeFitText(5);
    snzu_Interaction* exitInter = SNZU_USE_MEM(snzu_Interaction, "exitInter");
    snzu_boxSetInteractionOutput(exitInter, SNZU_IF_HOVER | SNZU_IF_MOUSE_BUTTONS);
    if (exitInter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
        for (int i = 0; i < 64; i++) {
            // If you see this it means you have to make e4_quit() a thing.
            free(sampleList[i]);
        }
        snz_quit();
    }
}
void e4_loadBox_FileDropdown_Caption(void* config) {
    SNZ_ASSERT(config==NULL,"Config wasn't null for some reason");

    snze_easyBox("fileDropdownMenu",0,0.12,0.05,0.2,ui_collightgray);
    snzu_boxSetBorder(1,ui_colblack);
    snzu_boxScope() {
        e4_loadBox_NewSession_File_Caption();
        e4_loadBox_OpenSession_File_Caption();
        e4_loadBox_Exit_File_Caption();
    }
    snzu_boxOrderChildrenInRowRecurse(5,SNZU_AX_Y,SNZU_ALIGN_LEFT);
}


void e4_channelAudInpConnectBox(void* config) {

    HMM_Vec4 posVec = ((struct {HMM_Vec4 posVec; int loadOrder;}*)config)->posVec;
    int loadOrder = ((struct {HMM_Vec4 posVec; int loadOrder;}*)config)->loadOrder;


    // Next step: make sure this is loading in the right place. Rn its not.
    // Ask rob about absolute start/end values, snzu_boxNewF, & other issues.
    snze_easyBox("indrop",0.9,1,posVec.W/(_snzu_instance->treeParent.end.Y),posVec.W/(_snzu_instance->treeParent.end.Y)+0.1,ui_collightgray);
    snzu_boxClipChildren(true);
    snzu_boxScope() {
        for(int i = 0; i < SDL_GetNumAudioDevices(0); i++) {
            snze_easyBoxDisp(int_to_string(i), 0, 1, 0, 0.2, ui_colgray, SDL_GetAudioDeviceName(i, 0));
            snzu_boxSetSizeFitText(0.2);
            snzu_Interaction* inter = SNZU_USE_MEM(snzu_Interaction, "inter");
            snzu_boxSetInteractionOutput(inter, SNZU_IF_MOUSE_BUTTONS | SNZU_IF_HOVER);
            if (inter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
                SNZ_ASSERT(activeOutputDevices[i]!=NULL, "Connected channel to inactive device.");
                activeOutputDevices[i]->isChannelConnected[loadOrder] = 1;
                snzu_boxSetColor(ui_colgreen);
            }
        }
        snzu_boxOrderChildrenInRowRecurse(0.02,SNZU_AX_Y,SNZU_ALIGN_LEFT);
    }
}

void e4_menu_deviceBox(void* config) {
    SNZ_ASSERT(config==NULL,"Config wasn't null for some reason");
    snze_easyBox("devicesbox", 0.25, 0.5, 0.05, 0.2, ui_collightgray);
    snzu_boxClipChildren(true);
    
    snzu_boxScope() {
        for(int i = 0; i < SDL_GetNumAudioDevices(0); i++) {
            HMM_Vec4 dispcol = HMM_V4(1,0.75,0.75,1);
            if (activeOutputDevices[i]!=NULL) {
                dispcol = HMM_V4(0.75,1,0.75,1);
            }
            snze_easyButton(SDL_GetAudioDeviceName(i, 0),HMM_V4(0,1,0,1),dispcol,SDL_GetAudioDeviceName(i, 0)) {
                printf("Attempting to activate device ");
                printf(SDL_GetAudioDeviceName(i, 0));
                printf(" with loadorder %d.\n", i);
                e4_activateOutputDevice(SDL_GetAudioDeviceName(i, 0));
            }
            snzu_boxSetSizeFitText(5);
        }
    }
    snzu_boxOrderChildrenInRowRecurse(5, SNZU_AX_Y, SNZU_ALIGN_LEFT);
    snzu_boxSetBorder(1,ui_colblack);
}

void e4_MENU_sessionDrop(void* config) {
    SNZ_ASSERT(config==NULL,"Config wasn't null for some reason");
    snze_easyBox("sessionDropM", 0.1, 0.25, 0.05, 0.2, HMM_V4(0.75,0.75,0.75,1));
    snzu_boxSetBorder(1,ui_colblack);
    snzu_boxScope() {
        snze_easyButton("addOutDevice", HMM_V4(0, 1, 0, 1), HMM_V4(0, 0, 0, 0), "Add Output Device") {
            snze_openMenu(e4_menu_deviceBox, NULL, 1);
        }
        snzu_boxSetSizeFitText(5);

        snze_easyButton("addInDevice", HMM_V4(0, 1, 0, 1), HMM_V4(0, 0, 0, 0), "Add Input Device") {
            snze_closeMenuPast(0);
        }
        snzu_boxSetSizeFitText(5);

        snze_easyButton("addChannel", HMM_V4(0, 1, 0, 1), HMM_V4(0, 0, 0, 0), "Add Channel") {
            e4_addChannel();
            snze_closeMenuPast(0);
        };
        snzu_boxSetSizeFitText(5);
    }
    snzu_boxOrderChildrenInRowRecurse(5,SNZU_AX_Y,SNZU_ALIGN_LEFT);
}

void e4_MENU_sampleDrop(void* config) {
    SNZ_ASSERT(config==NULL,"Config wasn't null for some reason");
    snze_easyBox("sampleDD", 0.2, 0.35, 0.05, 0.2, ui_collightgray);
    snzu_boxSetBorder(1, ui_colblack);
    snzu_boxScope() {
        snze_easyButton("sampleAdd", HMM_V4(0, 1, 0, 1), ui_coltransparent, "Add Sample") {
            nfdresult_t result = NFD_OpenDialogU8(&tempSamplePath, NULL, 0, NULL);
            if (result != NFD_OKAY) {printf("NFD stuff is not okay :(\n");}
            e4_loadSample(tempSamplePath);
        }
        snzu_boxSetSizeFitText(5);

        snze_easyButton("sampleFolderAdd", HMM_V4(0, 1, 0, 1), ui_coltransparent, "Select Working Folder") {
            nfdresult_t result = NFD_PickFolderU8(&workingSamplePath, NULL);
            if (result != NFD_OKAY) {printf("NFD stuff is not okay :(\n");}
        }
        snzu_boxSetSizeFitText(5);
    }
    snzu_boxOrderChildrenInRowRecurse(5, SNZU_AX_Y, SNZU_ALIGN_LEFT);
}

void e4_loadSampleWorkbench() {
    snze_easyBox("SampleWorkbench",0.1,0.9,0.1, 0.3, ui_collightblue);
    snzu_boxScope() {
        

        for(int i = 0; i < Total_Samples; i++) {
            if(sampleList[i]!=NULL) {
                snze_easyBoxDisp(int_to_string(i), i/16.0f, (i+1)/16.0f, 0.5, 1, HMM_V4(0,0,1,1),int_to_string(i));
            } else {
                break;
            }
        }
    }
}


void e4_init() {
    staticPerfFreq = SDL_GetPerformanceFrequency();
    session_start_time = SDL_GetPerformanceCounter();
    last_frame = session_start_time;
    // Visual bug when initialized, change this
    pause_color = ui_colgreen;
    for(int i = 0; i < 8; i++) {
        menuStruct.func[i] = nothing;
        menuStruct.config[i] = NULL;
    }
    // only here for debugging
    e4_activateOutputDevice(SDL_GetAudioDeviceName(0,0));
    e4_addChannel();


    e4_initSessionMode();

}

void e4_quit() {
    // Freeing stuff that needs to be freed should happen here.
    snz_quit();
}

void e4_main() {

    // Caption bar
    snze_easyBox("Caption",0,1,0,0.05,ui_colgray);
    snzu_boxScope() {
        // File dropdown bar
        snze_easyButton("File_Dropdown", HMM_V4(0, 0.1, 0, 1), ui_collightgray, "File") {
            snze_openMenu(e4_loadBox_FileDropdown_Caption, NULL, 0);
        }
        snzu_boxSetBorder(1, ui_colblack);

        if (e4_mode==e4_SESSION_MODE) {
            snze_easyButton("SessionDropdown",HMM_V4(0.1,0.2,0,1),ui_collightgray, "Session") {
                snze_openMenu(e4_MENU_sessionDrop, NULL, 0);
            }
            snzu_boxSetBorder(1,ui_colblack);

            snze_easyButton("SamplesDD", HMM_V4(0.2,0.3,0,1),ui_collightgray,"Samples") {
                snze_openMenu(e4_MENU_sampleDrop, NULL, 0);
            }
            snzu_boxSetBorder(1,ui_colblack);
        }
        // Some goofy stuff with playback.
        snze_easyButton("ribs", HMM_V4(0.3,0.4,0,1),ui_collightgray,"Ribs!") {
            e4_queueSampleToChannel(0, 0, counter_unpaused + staticPerfFreq, 500000);
        }
    }

    e4_loadSampleWorkbench();

    snze_easyBoxDisp("Mode_Indicator", 0.9, 1, 0, 0.05, ui_colgray, e4_modeStr(e4_mode));
    
    if (e4_mode == e4_SESSION_MODE) {
        snze_easyButton("Pause_Button",HMM_V4(0.9,1, 0.05,0.1), pause_color, NULL) {
            sessionPaused = !sessionPaused;
            if(sessionPaused) {pause_color = ui_colred;} else {pause_color = ui_colgreen;}
        }

        // In addition, have other timers.
        static char nowBuf[12];  // Enough for max 32-bit uint + null terminator
        snprintf(nowBuf, sizeof(nowBuf), "%f", (double)counter_unpaused/(double)staticPerfFreq);
        const char* nowStr= nowBuf;
        snze_easyBoxDisp("timebox", 0.8, 0.9, 0.05, 0.1, ui_collightgray, nowStr);
        snzu_boxSetBorder(3,ui_colblack);
        
        snze_easyBoxNoCol("Channel_Parent", 0, 1, 0.3, 1);
        snzu_boxClipChildren(true);
        snzu_boxScope() {
            for(int actChan = 0; actChan < Total_Channels; actChan++) {

                e4_Channel* activeChnlPtr = channelList[actChan];

                if (activeChnlPtr!=NULL) {
                    // please fix the slopcode
                    // Introduce scroll
                    // Channel box
                    snze_easyBox(int_to_string(actChan),0,1,actChan/channelsVisible,(actChan+1)/channelsVisible,ui_coldarkgray);
                    snzu_boxClipChildren(true);
                    snzu_boxScope() {
                        snze_easyBox("Channel", 0.025, 0.975, 0.005, 0.995, ui_coldarkred);
                        snzu_boxClipChildren(true);
                        snzu_boxScope() {
                            // Herein the sample stuff
                            // Delete queue flag after!!
                            for(int qfNum = 0; qfNum < activeChnlPtr->numQueueFlags; qfNum++) {
                                e4_QueueFlag* queueFlag = activeChnlPtr->queueFlags[qfNum];
                                if((counter_unpaused + futureSecondsVisible*staticPerfFreq > queueFlag->counterToQueueAt)) {
                                    snze_easyBox("ribs", e4_audBlockPctStart(queueFlag), e4_audBlockPctEnd(queueFlag), 0.05,0.8,ui_collightgray);
                                }
                            }

                            snze_easyBox("bottombar",0,1,0.95,1,ui_colblack);
                            for(int j = 0; j < (int)(pastSecondsVisible+futureSecondsVisible); j++) {
                                snze_easyBox(int_to_string(j),(j+1-((double)(counter_unpaused % staticPerfFreq))/(double)staticPerfFreq)/(pastSecondsVisible+futureSecondsVisible)-0.005,(j+1-((double)(counter_unpaused % staticPerfFreq))/(double)staticPerfFreq)/(pastSecondsVisible+futureSecondsVisible)+0.005, 0.9, 0.95, ui_colblack);
                            }
                            snze_easyButton("Exit_Button", HMM_V4(0.95, 1, 0, 0.2), ui_colblack, NULL) {
                                e4_removeChannel(actChan);
                            }
                            
                            snze_easyBoxDisp("id", 0.95, 1, 0.2, 0.4,ui_collightgray, int_to_string(activeChnlPtr->ID));
                        }
                        snze_easyButton("devConnect", HMM_V4(0.975,1,actChan*0.2,actChan*0.2+0.2), ui_colgray, "In") {
                            // When to free?
                            // Scary struct pointer stuff!
                            struct {HMM_Vec4 posVec; int loadOrder;}* configPtr = malloc(sizeof(struct {HMM_Vec4 posVec; int loadOrder;}));
                            
                            configPtr->posVec.X = _snzu_instance->selectedBox->start.X;
                            configPtr->posVec.Y = _snzu_instance->selectedBox->end.X;
                            configPtr->posVec.Z = _snzu_instance->selectedBox->start.Y;
                            configPtr->posVec.W = _snzu_instance->selectedBox->end.Y;
                            configPtr->loadOrder = actChan;
                            
                            
                            snze_openMenu(e4_channelAudInpConnectBox, configPtr, 0);
                        }
                    }
                } else {
                    break;
                }
            }
            snze_easyBox("timebar",0.025+(0.975-0.025)*0.1-0.001,0.025+(0.975-0.025)*0.1+0.001,0,1,HMM_V4(0.5,0.5,0.5,0.5));
            
        }
    }
    // Handle menu stuff down here.

    for(int i = 0; i < 8; i++) {
        menuStruct.func[i](menuStruct.config[i]);
    }

    // to make this consistent, perhaps handle pausing / unpausing at the beginning?
    // Surely the latency cannot matter so much?
    e4_updateFrametime();
}