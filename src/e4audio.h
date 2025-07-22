#pragma once

#include "e4structs.h"

enum e4_Mode e4_mode = e4_DEFAULT_MODE;

/*
    Code for audio devices, input and output

    In progress.
*/
typedef struct {
    // SCUFFED AHH HELL FIXME the name stuff 
    SDL_AudioDeviceID ID;
    char* name;
    int loadOrder;
    SDL_AudioSpec obtainedSpecs;
    bool isChannelConnected[Total_Channels];
} e4_ActiveOutputDevice;

typedef struct {
    SDL_AudioDeviceID ID;
    char* name;
    int number;
    SDL_AudioSpec obtainedSpecs;
    bool isChannelConnected[Total_Channels];
} e4_ActiveInputDevice;

e4_ActiveOutputDevice* activeOutputDevices[Total_Output_Devices] = { NULL };
e4_ActiveInputDevice* activeInputDevices[Total_Input_Devices] = { NULL };
int e4_getNumActiveOutputDevices() {
    int ctr = 0;
    for (int i = 0; i < Total_Output_Devices; i++) {
        if(activeOutputDevices[i]!=NULL) {
            ctr++;
        } else {
            break;
        }
    }
    return(ctr);
}
int e4_getNumActiveInputDevices() {
    int ctr = 0;
    for (int i = 0; i < Total_Input_Devices; i++) {
        if(activeInputDevices[i]!=NULL) {
            ctr++;
        } else {
            break;
        }
    }
    return(ctr);
}
// Fixme : deactivation functions for in and out devices.

/*
    Code for samples.

    In progress.
*/
typedef struct {
    SDL_AudioSpec audioSpec;
    Uint8* audioBuffer;
    Uint32 audioLength;
    char name[64];
} e4_Sample;

e4_Sample* sampleList[Total_Samples] = { NULL };

int e4_loadSample(nfdchar_t* path) {
    SNZ_ASSERT(path!=NULL, "Attempted to load sample from nonexistent path.");
    // Loading the sample
    e4_Sample sample = { 0 };
    SNZ_ASSERT(SDL_LoadWAV((const char*)path, &sample.audioSpec, &sample.audioBuffer, &sample.audioLength)!=NULL, "Sample loading failed.");
    if (sample.audioSpec.channels!=2) {
        printf("Channels not 2.\n");
    }
    if (sample.audioSpec.freq!=44100) {
        printf("Frequency not 44100, it is %d.\n",sample.audioSpec.freq);
    }
    if (SDL_AUDIO_BITSIZE(sample.audioSpec.format)!=16) {
        printf("Bitsize not 16\n");
    }
    printf("Length is: %u bits\n",sample.audioLength);
    if ((const char*)path!=NULL) {
        // Fixme: code is chopped & slopped.
        copy_filename_after_backslash(sample.name, (const char*)path);
    } else {
        printf("error: path null\n");
    }
    
    e4_Sample* sampleptr = malloc(sizeof(e4_Sample));
    *sampleptr = sample;
    printf(sampleptr->name);
    
    // Sending it to the list
    for(int i = 0; i < Total_Samples; i++) {
        if(sampleList[i]==NULL) {
            sampleList[i] = sampleptr;
            return(i);
        }
    }

    // If no space
    printf("No space for this sample!\n");
    free(sampleptr);
    return(-1);
}

void e4_deleteSample(int ID) {
    SNZ_ASSERT((ID>=0)&(ID<Total_Channels), "ID is not an expected value.");
    if (sampleList[ID]==NULL) {
        printf("Warning: tried to delete nonexistent sample.\n");
    } else {
        free(sampleList[ID]);
        sampleList[ID] = NULL;
    }
}

/*
    Code for channels.

    In progress.
*/
typedef struct {
    Uint64 counterToQueueAt;
    Uint8* samplePtr;
    Uint32 bytesToQueue;
    Uint32 totalBytes;
} e4_QueueFlag;

typedef struct {
    int ID;
    e4_QueueFlag** queueFlags;
    int numQueueFlags;
} e4_Channel;

e4_Channel* channelList[Total_Channels] = { NULL };

void e4_addChannel() {
    int newID = 0;
    while(goofyChannelIdArray[newID]) {
        newID++;
        SNZ_ASSERT(newID<64, "id greater than 64 somehow");
    }


    for (int i = 0; i < 64; i++) {
        if(channelList[i]==NULL) {
            // Fix this
            e4_Channel* channelptr = malloc(sizeof(e4_Channel));
            channelptr->numQueueFlags=0;
            channelptr->queueFlags=NULL;
            channelptr->ID=newID;
            channelList[i] = channelptr;
            printf("New channel added: ID %d, loadOrder %d.\n",newID,i);
    
            break;
        }
    }

    goofyChannelIdArray[newID] = 1;
}

// Code can be tightened a little.
void e4_removeChannel(int loadOrder) {
    printf("Channel removed: ID %d, loadOrder %d.\n",channelList[loadOrder]->ID, loadOrder);
    // Fixme: make sure everything associated with the channel is freed and deleted.
    if(channelList[loadOrder]==NULL) {
        printf("Warning: tried to remove inactive channel!\n");
    } else {
        goofyChannelIdArray[channelList[loadOrder]->ID] = 0;
        // FIXME: Lock the audio here
        free(channelList[loadOrder]->queueFlags);
        free(channelList[loadOrder]);

        channelList[loadOrder] = NULL;
        for(int i = 0; i < Total_Channels; i++) {
            if(channelList[i]!=NULL) {
                printf("Before %d is a channel", i);
            }
        }
        
        if(loadOrder != Total_Channels-1) {
            for(int i = loadOrder; i < Total_Channels - 1; i++) {
                channelList[i] = channelList[i+1];
            }
        }
    }


}

void e4_queueSampleToChannel(int channelID, int sampleID, Uint64 counter, Uint32 bytes) {
    // Check if sample or channel is null.
    SNZ_ASSERT(sampleList[sampleID]!=NULL, "Sample id is null");
    SNZ_ASSERT(channelList[channelID]!=NULL, "Channel id is null");
    SNZ_ASSERT(counter > counter_unpaused, "Attempted to queue sample in the past");
    SNZ_ASSERT(sampleList[sampleID]->audioLength>=bytes, "Wanted too many bytes from sample");

    e4_QueueFlag* newQueueFlag = malloc(sizeof(e4_QueueFlag));
    newQueueFlag->counterToQueueAt = counter;
    newQueueFlag->bytesToQueue = bytes;
    newQueueFlag->totalBytes = bytes;
    newQueueFlag->samplePtr = sampleList[sampleID]->audioBuffer;

    int nqf = channelList[channelID]->numQueueFlags;
    if(nqf==0) {
        // Fixme: dealloc memory. Actually go through check all mallocs and make sure.
        channelList[channelID]->queueFlags = malloc(sizeof(e4_QueueFlag*));
        //Fixme: data start in bytes customizable to nonzero vals.
        channelList[channelID]->queueFlags[0] = newQueueFlag;
        channelList[channelID]->numQueueFlags++;
    } else {
        channelList[channelID]->queueFlags = realloc(channelList[channelID]->queueFlags,sizeof(e4_QueueFlag*)*(1+nqf));
        channelList[channelID]->queueFlags[nqf] = newQueueFlag;
        channelList[channelID]->numQueueFlags++;
    }
    // This all SHOULD be working....Potentially is not.
}

/*
    Audio callback.

    In progress.
*/
void e4_audioCallback(void* userdata, Uint8* stream, int len) {
    // printf("Starting callback.\n");
    e4_ActiveOutputDevice* outDevPtr = (e4_ActiveOutputDevice*)userdata;
    SDL_memset(stream, 0, len);
    
    // Atm, just overwrites stream - no mixing yet!
    for(int actChan = 0; actChan < Total_Channels; actChan++) {
        if (outDevPtr->isChannelConnected[actChan]) {

            // printf("Callbacking channel %d.\n", actChan);
            e4_Channel* channelPtr = channelList[actChan];

            // Resolving audio queue flags for a channel
            for (int actQF = 0; actQF < channelPtr->numQueueFlags; actQF++) {
                e4_QueueFlag* actQFptr = channelPtr->queueFlags[actQF];
                //printf("Doing active queue flag %d", actQF);
                if(actQFptr->counterToQueueAt < counter_unpaused) {
                    if(actQFptr->bytesToQueue > (Uint32)len) {
                        SDL_memcpy(stream, actQFptr->samplePtr, len);
                        actQFptr->bytesToQueue = actQFptr->bytesToQueue - len;
                        actQFptr->samplePtr = actQFptr->samplePtr + len;
                    } else if (actQFptr->bytesToQueue == (Uint32)len) {
                        SDL_memcpy(stream, actQFptr->samplePtr, len);
                        actQFptr->bytesToQueue = 0;
                    } else if (actQFptr->bytesToQueue < (Uint32)len) {
                        SDL_memcpy(stream, actQFptr->samplePtr, actQFptr->bytesToQueue);
                        SDL_memset(stream + actQFptr->bytesToQueue, 0, len - actQFptr->bytesToQueue);
                        actQFptr->bytesToQueue = 0;
                    }
                }
            }
        }
    }
}
// Fixme MUST deal w/ queue flags w/ 0 bytes left to queue OUTSIDE of callback!

/*
    Device activations.

    In progress.
*/

void e4_activateOutputDevice(const char* devName) {

    // Check the inputs are valid
    SNZ_ASSERT(devName!=NULL, "No device name put in.");

    // List this as an open output device.
    // Fixme: make this more robust i.e. make parts of it happen later.
    e4_ActiveOutputDevice* outDevPtr = calloc(1, sizeof(e4_ActiveOutputDevice));
    
    // Default specs. Can be changed.
    SDL_AudioSpec desiredSpec = {
        .callback = e4_audioCallback,
        .channels = 2,
        .format = AUDIO_S16SYS,
        .freq = 44100,
        .samples = 4096,
        .silence = 0,
        .userdata = outDevPtr
    };
    
    outDevPtr->ID = SDL_OpenAudioDevice(devName, 0, &desiredSpec, &outDevPtr->obtainedSpecs, 0);
    SNZ_ASSERT(outDevPtr->ID!=0, "Opening the device failed, somehow.");
    
    activeOutputDevices[e4_getNumActiveOutputDevices()] = outDevPtr;
    SDL_PauseAudioDevice(outDevPtr->ID, 0);
    // Fixme later: write a check here that obtained is desired.
}

void e4_deactivateOutputDevice(SDL_AudioDeviceID devID) {
    for(int i = 0; i < Total_Output_Devices; i++) {
        SNZ_ASSERT(activeOutputDevices[i]!=NULL, "Tried to deactivate already deactivated device.");
        if(activeOutputDevices[i]->ID==devID) {
            SDL_PauseAudioDevice(devID, 1);
            SDL_CloseAudioDevice(devID);
            // Fix this :(
            //free(activeOutputDevices[i]->name);
            free(activeOutputDevices[i]);
            for (int j = i+1; j < Total_Output_Devices; j++) {
                activeOutputDevices[j-1] = activeOutputDevices[j];
            }
            activeOutputDevices[Total_Output_Devices] = NULL;
            break;
        }
    }
}

