#ifndef _RV1126_PUBLIC_H
#define _RV1126_PUBLIC_H

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "sample_common.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"
#include "rkmedia_vdec.h"

#define AAC_NB_SAMPLES 1024

#define S32CAMID 0
#define CAM_DEVICE_NAME "rkispp_scale0" 
#define AUDIO_ALSA_PATH "default:CARD=rockchiprk809co"

typedef struct
{
    unsigned int id;
    VI_CHN_ATTR_S attr;
    bool enable;
} S_VI_CONFIG;

typedef struct
{
    unsigned int id;
    AI_CHN_ATTR_S attr;
    bool enable;
} S_AI_CONFIG;

typedef struct
{
    unsigned int id;
    RGA_ATTR_S attr;
    bool enable;
} S_RGA_CONFIG;

typedef struct
{
    unsigned int id;
    VENC_CHN_ATTR_S attr;
    bool enable;
} S_VENC_CONFIG;

typedef struct
{
    unsigned int id;
    AENC_CHN_ATTR_S attr;
    bool enable;
} S_AENC_CONFIG;

#endif