#include <stdio.h>
#include "rv1126_public.h"

void * collect_venc_thread(void * args)
{
   pthread_detach(pthread_self());
   MEDIA_BUFFER mb;
   FILE * h264_file = fopen("./test_output.h264", "w+");

   while (1)
   {
       mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, 0 , -1);
       if(!mb)
       {
           printf("Get Venc Buffer Break....\n");
           break;
       }

       printf("mmmmmm\n");
       fwrite(RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb), 1, h264_file);
   }
   return NULL;
}


int main()
{
    RK_U32 u32Width = 1920;
    RK_U32 u32Height = 1080;
    RK_CHAR *pDeviceName = "rkispp_scale0";
    RK_CHAR *pOutPath = NULL;
    RK_CHAR *pIqfilesPath = NULL;
    CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;
    RK_CHAR *pCodecName = "H264";
    RK_S32 s32CamId = 0;
    RK_U32 u32BufCnt = 3;

    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    SAMPLE_COMM_ISP_Init(hdr_mode, RK_FALSE);
    SAMPLE_COMM_ISP_Run();
    SAMPLE_COMM_ISP_SetFrameRate(30);

    int ret;
    RK_MPI_SYS_Init();
    VI_CHN_ATTR_S vi_chn_attr;
    vi_chn_attr.pcVideoNode = pDeviceName;
    vi_chn_attr.u32BufCnt = u32BufCnt;
    vi_chn_attr.u32Width = u32Width;
    vi_chn_attr.u32Height = u32Height;
    vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
    vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;  //VI通道的缓冲区类型为内存映射方式.在内存映射方式下，VI通道的缓冲区将被映射到用户空间的一块内存中，用户可以直接访问该内存，以读取或写入VI通道的数据。
    vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;  
    ret = RK_MPI_VI_SetChnAttr(s32CamId, 1, &vi_chn_attr); 
    ret |= RK_MPI_VI_EnableChn(s32CamId, 1);
    if (ret)
    {
        printf("ERROR: create VI[0] error! ret=%d\n", ret);
        return 0;
    }

    VENC_CHN_ATTR_S venc_chn_attr;
    memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
    venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR; //H.264编码器的码率控制模式为恒定码率(CBR)模式。在恒定码率模式下，编码器将尝试以恒定的比特率对视频帧进行编码，以便在网络传输和存储方面具有一致的性能。
    venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30; 
    venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = u32Width * u32Height;
    // frame rate: in 30/1, out 30/1.
    //源帧率
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30; //分子为30，分母为1，表示输出帧率为30帧/秒。
    //目标帧率
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;

    venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12;
    venc_chn_attr.stVencAttr.u32PicWidth = u32Width;
    venc_chn_attr.stVencAttr.u32PicHeight = u32Height;
    venc_chn_attr.stVencAttr.u32VirWidth = u32Width;
    venc_chn_attr.stVencAttr.u32VirHeight = u32Height;
    venc_chn_attr.stVencAttr.u32Profile = 77; //H.264编码器的编码配置参数为77，表示Main Profile。双向预测和高级压缩编码
    ret = RK_MPI_VENC_CreateChn(0, &venc_chn_attr);
    if (ret)
    {
        printf("ERROR: create VENC[0] error! ret=%d\n", ret);
        return 0;
    }

    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = RK_ID_VI;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 1;
    
    MPP_CHN_S stDestChn;
    stDestChn.enModId = RK_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = 0;
    ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (ret)
    {
        printf("ERROR: Bind VI[0] and VENC[0] error! ret=%d\n", ret);
        return 0;
    }

    pthread_t pid;
    ret = pthread_create(&pid, NULL, collect_venc_thread, NULL);
    if(ret != 0)
    {
        printf("Create Venc Thread Failed....\n");
        return;
    }

    while (1)
    {
       sleep(20);
    }
    

    return 0;
}