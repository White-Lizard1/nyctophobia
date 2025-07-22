#pragma once

#define E4_IN_CHANNELS 8
#define E4_OUT_CHANNELS 8
#define E4_NUM_NODES 64
#define E4_FPBIN_CAPACITY 8

enum e4_Nodetype {
    e4OUTPUT = 0,
    e4INPUT = 1,
    e4FILE = 2,
    e4PURE = 3,
    e4UNSET = -1,
};

int channel0 = 0;