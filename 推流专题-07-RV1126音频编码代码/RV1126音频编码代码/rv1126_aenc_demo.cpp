#include <stdio.h>
#include "rv1126_public.h"

void *get_audio_aenc_thread(void *args)
{
    MEDIA_BUFFER mb = NULL;
    FILE * audio_file = fopen("test_out.aac", "w+");

    while (1)
    {
        mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_AENC, 0, -1);
        if (!mb)
        {
            printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
            break;
        }

        printf("#Get Frame:ptr:%p, size:%zu, mode:%d, channel:%d, "
               "timestamp:%lld\n",
               RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb),
               RK_MPI_MB_GetModeID(mb), RK_MPI_MB_GetChannelID(mb),
               RK_MPI_MB_GetTimestamp(mb));

        fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), audio_file);
    }

    return NULL;
}

int main()
{
    int ret;
    MPP_CHN_S mpp_chn_ai, mpp_chn_aenc;
    RK_U32 u32SampleRate = 16000;
    RK_U32 u32BitRate = 64000; // 64kbps
    RK_U32 u32ChnCnt = 2;
    RK_U32 u32FrameCnt = 1024; // always 1024 for mp3 aac
    SAMPLE_FORMAT_E enSampleFmt = RK_SAMPLE_FMT_FLTP;
    // default:CARD=rockchiprk809co
    RK_CHAR *pDeviceName = "default";
    RK_CHAR *pOutPath = "/tmp/aenc.mp3";

    AI_CHN_ATTR_S ai_attr;
    ai_attr.pcAudioNode = pDeviceName;
    ai_attr.enSampleFormat = enSampleFmt;
    ai_attr.u32NbSamples = u32FrameCnt;
    ai_attr.u32SampleRate = u32SampleRate;
    ai_attr.u32Channels = u32ChnCnt;
    ai_attr.enAiLayout = AI_LAYOUT_NORMAL;
    ret = RK_MPI_AI_SetChnAttr(0, &ai_attr);
    ret |= RK_MPI_AI_EnableChn(0);
    if (ret)
    {
        printf("Create AI[0] failed! ret=%d\n", ret);
        return -1;
    }

    // 2. create AENC
    AENC_CHN_ATTR_S aenc_attr;
    aenc_attr.enCodecType = RK_CODEC_TYPE_AAC;
    aenc_attr.u32Bitrate = u32BitRate;
    aenc_attr.u32Quality = 1;
    aenc_attr.stAencAAC.u32Channels = u32ChnCnt;
    aenc_attr.stAencAAC.u32SampleRate = u32SampleRate;
    ret = RK_MPI_AENC_CreateChn(0, &aenc_attr);
    if (ret)
    {
        printf("Create AENC[0] failed! ret=%d\n", ret);
        return -1;
    }

    mpp_chn_ai.enModId = RK_ID_AI;
    mpp_chn_ai.s32ChnId = 0;

    mpp_chn_aenc.enModId = RK_ID_AENC;
    mpp_chn_aenc.s32ChnId = 0;
    ret = RK_MPI_SYS_Bind(&mpp_chn_ai, &mpp_chn_aenc);
    if (ret)
    {
        printf("Bind AI[0] to AENC[0] failed! ret=%d\n", ret);
        return -1;
    }

    pthread_t aenc_thread;
    ret = pthread_create(&aenc_thread, NULL, get_audio_aenc_thread, NULL);
    if(ret != 0)
    {
        printf("create aenc thread failed....\n");
    }

    while (1);

    RK_MPI_SYS_UnBind(&mpp_chn_ai, &mpp_chn_aenc);
    RK_MPI_AI_DisableChn(mpp_chn_ai.s32ChnId);
    RK_MPI_AENC_DestroyChn(mpp_chn_aenc.s32ChnId);

    return 0;
}