// 初始化输出文件上下文
AVFormatContext *out_fmt_ctx = NULL;
avformat_alloc_output_context2(&out_fmt_ctx, NULL, NULL, "output.aac");
if (!out_fmt_ctx) {
    printf("Could not create output context\n");
    return -1;
}

// 初始化编码器上下文
AVCodecContext *codec_ctx = NULL;
AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
if (!codec) {
    printf("Could not find AAC encoder\n");
    return -1;
}
codec_ctx = avcodec_alloc_context3(codec);
if (!codec_ctx) {
    printf("Could not allocate AAC codec context\n");
    return -1;
}

// 设置编码器参数
codec_ctx->sample_rate = 44100; // 采样率
codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO; // 声道布局
codec_ctx->channels = av_get_channel_layout_nb_channels(codec_ctx->channel_layout); // 声道数
codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP; // 采样格式，AAC只支持FLTP格式的PCM数据
codec_ctx->bit_rate = 64000; // 比特率

// 打开编码器
int ret = avcodec_open2(codec_ctx, codec, NULL);
if (ret < 0) {
    printf("Could not open AAC codec\n");
    return -1;
}

// 创建输出流并添加到输出文件上下文中
AVStream *out_stream = avformat_new_stream(out_fmt_ctx, codec);
if (!out_stream) {
    printf("Failed allocating output stream\n");
    return -1;
}
out_stream->time_base.num = 1;
out_stream->time_base.den = codec_ctx->sample_rate;

// 复制编码器参数到输出流中
ret = avcodec_parameters_from_context(out_stream->codecpar, codec_ctx);
if (ret < 0) {
    printf("Failed to copy encoder parameters to output stream\n");
    return -1;
}

// 打开输出文件并写入头信息
ret = avio_open(&out_fmt_ctx->pb, "output.aac", AVIO_FLAG_WRITE);
if (ret < 0) {
    printf("Could not open output file\n");
    return -1;
}
ret = avformat_write_header(out_fmt_ctx, NULL);
if (ret < 0) {
    printf("Error occurred when opening output file\n");
    return -1;
}

// 初始化输入文件上下文和输入流索引
AVFormatContext *in_fmt_ctx = NULL;
int audio_index;

// 打开输入文件并读取头信息
ret = avformat_open_input(&in_fmt_ctx, "input.pcm", NULL, NULL);
if (ret < 0) {
    printf("Could not open input file\n");
    return -1;
}
ret = avformat_find_stream_info(in_fmt_ctx,NULL);
if (ret < 0) {
   printf("Failed to retrieve input stream information\n");
   return -1;
}

// 查找音频流索引并设置解码器参数为s16le格式的PCM数据 
for(int i=0;i<in_fmt_ctx->nb_streams;i++){
   if(in_fmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){
       audio_index=i;

       AVCodecParameters *parameters=in_fmt_ct
	   // 设置解码器参数为s16le格式的PCM数据 
       parameters->codec_id=AV_CODEC_ID_PCM_S16LE;
       parameters->format=AV_SAMPLE_FMT_S16;
       parameters->sample_rate=44100;
       parameters->channels=2;
       parameters->channel_layout=av_get_default_channel_layout(parameters->channels);
       parameters->bit_rate=av_samples_get_buffer_size(NULL,parameters->channels,parameters->sample_rate,(enum AVSampleFormat)parameters->format,1);
   }
}

// 初始化解码器上下文和解码器
AVCodecContext *decode_ctx = NULL;
AVCodec *decode = avcodec_find_decoder(in_fmt_ctx->streams[audio_index]->codecpar->codec_id);
if (!decode) {
    printf("Could not find PCM decoder\n");
    return -1;
}
decode_ctx = avcodec_alloc_context3(decode);
if (!decode_ctx) {
    printf("Could not allocate PCM decoder context\n");
    return -1;
}

// 复制解码器参数到解码器上下文中
ret = avcodec_parameters_to_context(decode_ctx,in_fmt_ctx->streams[audio_index]->codecpar);
if (ret < 0) {
    printf("Failed to copy decoder parameters to input stream\n");
    return -1;
}

// 打开解码器
ret = avcodec_open2(decode_ctx, decode, NULL);
if (ret < 0) {
    printf("Could not open PCM decoder\n");
    return -1;
}

// 初始化音频重采样上下文和参数
SwrContext *swr_ctx = NULL;

int64_t in_ch_layout = decode_ctx->channel_layout; // 输入声道布局
int64_t out_ch_layout = codec_ctx->channel_layout; // 输出声道布局

int in_sample_rate = decode_ctx->sample_rate; // 输入采样率
int out_sample_rate = codec_ctx->sample_rate; // 输出采样率

enum AVSampleFormat in_sample_fmt = decode_ctx->sample_fmt; // 输入采样格式
enum AVSampleFormat out_sample_fmt = codec_ctx->sample_fmt; // 输出采样格式

// 创建音频重采样上下文并设置参数
swr_ctx = swr_alloc();
swr_alloc_set_opts(swr_ctx,
    out_ch_layout,
    out_sample_fmt,
    out_sample_rate,
    in_ch_layout,
    in_sample_fmt,
    in_sample_rate,
    0,NULL);
swr_init(swr_ctx);

// 初始化输入和输出的音频帧
AVFrame *in_frame = av_frame_alloc();
AVFrame *out_frame = av_frame_alloc();

// 设置输出帧的参数
out_frame->nb_samples = codec_ctx->frame_size; // 输出帧的采样数
out_frame->format = codec_ctx->sample_fmt; // 输出帧的采样格式
out_frame->channel_layout = codec_ctx->channel_layout; // 输出帧的声道布局

// 分配输出帧的缓冲区
int buffer_size = av_samples_get_buffer_size(NULL,codec_ctx->channels,codec_ctx->frame_size,codec_ctx->sample_fmt,1);
uint8_t *out_buffer = (uint8_t *)av_malloc(buffer_size);
avcodec_fill_audio_frame(out_frame,codec_ctx->channels,codec_ctx->sample_fmt,(const uint8_t *)out_buffer,buffer_size,1);

// 初始化输入和输出的数据包
AVPacket *in_packet = av_packet_alloc();
AVPacket *out_packet = av_packet_alloc();

// 读取输入文件中的数据包并解码成音频帧，然后重采样成FLTP格式并编码成AAC数据包，最后写入到输出文件中
while (av_read_frame(in_fmt_ctx,in_packet) >= 0) {
    if (in_packet->stream_index == audio_index) {
        ret = avcodec_send_packet(decode_ctx,in_packet);
        if (ret < 0) {
            printf("Error sending a packet for decoding\n");
            return -1;
        }
        while (ret >= 0) {
            ret = avcodec_receive_frame(decode_ctx,in_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                printf("Error during decoding\n");
                return -1;
            }
			// 重采样输入帧到输出帧
            swr_convert(swr_ctx,
                out_frame->data,
                out_frame->nb_samples,
                (const uint8_t **)in_frame->data,
                in_frame->nb_samples);

            // 编码输出帧到输出数据包
            ret = avcodec_send_frame(codec_ctx,out_frame);
            if (ret < 0) {
                printf("Error sending a frame for encoding\n");
                return -1;
            }
            while (ret >= 0) {
                ret = avcodec_receive_packet(codec_ctx,out_packet);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    printf("Error during encoding\n");
                    return -1;
                }
				// 写入输出数据包到输出文件
                out_packet->stream_index = out_stream->index;
                av_packet_rescale_ts(out_packet,out_stream->codec->time_base,out_stream->time_base);
                ret = av_interleaved_write_frame(out_fmt_ctx,out_packet);
                if (ret < 0) {
                    printf("Error muxing packet\n");
                    return -1;
                }
            }
        }
    }
    av_packet_unref(in_packet);
}

// 写入输出文件的尾部信息
av_write_trailer(out_fmt_ctx);

// 释放资源
av_frame_free(&in_frame);
av_frame_free(&out_frame);
av_packet_free(&in_packet);
av_packet_free(&out_packet);
swr_free(&swr_ctx);
avcodec_close(decode_ctx);
avcodec_close(codec_ctx);
avformat_close_input(&in_fmt_ctx);
avio_close(out_fmt_ctx->pb);
avformat_free_context(out_fmt_ctx);

return 0;