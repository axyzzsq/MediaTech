hide := @
ECHO := echo

G++ := /opt/rv1126_gcc/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
CFLAGS := -I./include/rkmedia \
			-I./include/rkaiq/common \
			-I./include/rkaiq/xcore \
			-I./include/rkaiq/uAPI \
			-I./include/rkaiq/algos \
			-I./include/rkaiq/iq_parser \
			-I/ 

LIB_FILES := -L./rv1126_lib #-L./arm32_ffmpeg_srt/lib -L/opt/arm_libsrt/lib #-L/opt/arm32_ffmpeg/lib

LD_FLAGS := -lpthread -leasymedia -ldrm -lrockchip_mpp \
	        -lavformat -lavcodec -lswresample -lavutil \
			-lasound -lv4l2 -lv4lconvert -lrga \
			-lRKAP_ANR -lRKAP_Common -lRKAP_3A \
			-lmd_share -lrkaiq -lod_share -lrknn_api 
		        
CFLAGS += -DRKAIQ

SAMPLE_COMMON := sample_common_isp.c

all:	
	$(G++) rv1126_h264_demo.cpp $(SAMPLE_COMMON) $(CFLAGS) $(LIB_FILES) $(LD_FLAGS) -o rv1126_h264_demo
	$(hide)$(ECHO) "Build Done ..."
