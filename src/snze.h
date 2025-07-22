#pragma once

#include "sloputils.h"
#include "snooze.h"

Uint32 hashOfSelectedBox[8] = {0};

typedef void (*fptr)(void*);

struct {
    void* config[8];
    fptr func[8];
} menuStruct;

void nothing(void* config) {
    SNZ_ASSERT(config==NULL,"Config wasn't null for some reason");
}

// Div by 0 error occurs. Avoidable  but fixme anyways
void snze_boxSetSizeRespectToParent(float start_x, float start_y, float end_x, float end_y) {
    snzu_boxSetSizePctParent(end_x,SNZU_AX_X);
    snzu_boxSetSizePctParent(end_y,SNZU_AX_Y);
    snzu_boxSetSizeFromEndAx(SNZU_AX_X,(snzu_boxGetSize().X)*(end_x-start_x)/end_x);
    snzu_boxSetSizeFromEndAx(SNZU_AX_Y,(snzu_boxGetSize().Y)*(end_y-start_y)/end_y);
}

void snze_easyBox(const char* title, float start_x, float end_x, float start_y, float end_y, HMM_Vec4 color) {
    snzu_boxNew(title);
    snzu_boxAlignInParent(SNZU_AX_X,SNZU_ALIGN_LEFT);
    snzu_boxAlignInParent(SNZU_AX_Y, SNZU_ALIGN_TOP);
    // If you ever change the order of start_y and end_x, this will break. Change it for parity at some point.
    snze_boxSetSizeRespectToParent(start_x, start_y, end_x, end_y);
    snzu_boxSetColor(color);
}

void snze_easyBoxNoCol(const char* title, float start_x, float end_x, float start_y, float end_y) {
    snzu_boxNew(title);
    snzu_boxAlignInParent(SNZU_AX_X,SNZU_ALIGN_LEFT);
    snzu_boxAlignInParent(SNZU_AX_Y, SNZU_ALIGN_TOP);
    // If you ever change the order of start_y and end_x, this will break. Change it for parity at some point.
    snze_boxSetSizeRespectToParent(start_x, start_y, end_x, end_y);
}

void snze_easyBoxDisp(const char* title, float start_x, float end_x, float start_y, float end_y, HMM_Vec4 color, const char* dispStr) {
    snze_easyBox(title, start_x, end_x, start_y, end_y, color);
    snzu_boxSetDisplayStr(&ui_labelFont, ui_colblack, dispStr);
}

void snze_setBoxAsSelected(int layer) {
    hashOfSelectedBox[layer] = snzu_getSelectedBox()->pathHash;
}

bool snze_BoxIsSelected(int layer) {
    return(snzu_getSelectedBox()->pathHash==hashOfSelectedBox[layer]);
}
void snze_unsetBoxAsSelected(int layer) {
    for (int i = layer; i<8; i++) {
        hashOfSelectedBox[i] = 0;
    }
}
void snze_boxMakeIntoMenu(snzu_Interaction* inter, int layer) {
    if (inter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
        if(snze_BoxIsSelected(layer)) {
            snze_unsetBoxAsSelected(layer);
        } else {
            snze_setBoxAsSelected(layer);
        }
    }
}

// "easy button part"
bool ebp(const char* tag, HMM_Vec4 posVec, HMM_Vec4 color, const char* displayStr) {
    if(displayStr==NULL) {
        snze_easyBox(tag, posVec.X, posVec.Y, posVec.Z, posVec.W, color);
    } else {
        snze_easyBoxDisp(tag, posVec.X, posVec.Y, posVec.Z, posVec.W, color, displayStr);
    }
    snzu_Interaction* inter = SNZU_USE_MEM(snzu_Interaction, "inter");
    snzu_boxSetInteractionOutput(inter, SNZU_IF_MOUSE_BUTTONS | SNZU_IF_HOVER);
    if(inter->mouseActions[SNZU_MB_LEFT]==SNZU_ACT_DOWN) {
        return(true);
    } else {
        return(false);
    }
}


#define snze_easyButton(tag, posVec, color, displayStr) if(ebp(tag, posVec, color, displayStr))

// ABANDON ALL HOPE YE WHO ENTER THE LAND OF SEGFAULTS

// Must delete layers above the layer replaced FIXME or weird shit will happen!
// Thank goodness we don't need multiple layer functionality yet!

// When to free the memory allocated for configs? who knows!

void snze_closeMenuPast(int layer) {
    for(int i = layer; i < 8; i++) {
        menuLayerOpen[i] = 0;
        menuStruct.func[i] = nothing;
        menuStruct.config[i] = NULL;
    }
}

#define snze_openMenu(FUNCTION, CONFIG, layer) do{if(menuStruct.func[layer]==FUNCTION) {\
        snze_closeMenuPast(layer);\
    } else {\
        menuLayerOpen[layer] = 1; menuStruct.func[layer] = FUNCTION; menuStruct.config[layer] = CONFIG; snze_closeMenuPast(layer+1);}} while(0)
