# FFmpeg V4L2级联

## 需求

从 V4L2 设备捕获视频帧，使用 FFmpeg 将帧从 YUYV422 格式转换为 YUV420P 格式，并将转换后的帧写入 MP4 视频文件。

## 思路

1. 打开一个 V4L2 视频捕获设备并配置它以捕获 YUYV422 格式的帧。
2. 为 V4L2 缓冲区分配内存并设置它们以进行视频捕获。
3. 为视频编码初始化 FFmpeg，并创建具有所需视频格式、宽度、高度和帧速率的输出上下文和编解码器上下文。
4. 输入循环以从 V4L2 设备捕获视频帧。
5. 在循环的每次迭代中，排队一个V4L2缓冲区用于视频捕获，等待缓冲区可用，使用FFmpeg的函数将捕获的视频帧从YUYV转换为YUV420P，并使用FFmpeg的编码函数将转换后的帧写入输出视频`sws_scale()`文件.
6. 在捕获所有需要的视频帧后清理资源，包括将预告片写入输出文件、关闭编解码器上下文、释放上下文和帧以及关闭 V4L2 设备。

## 实现

[源码文件]

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#define DEVICE_PATH "/dev/video0"  // path to the V4L2 device
#define WIDTH 640                  // width of the captured video frame
#define HEIGHT 480                 // height of the captured video frame
#define FPS 30                     // frames per second of the captured video

int main(int argc, char *argv[]) {
    int fd;                         // file descriptor for the V4L2 device
    struct v4l2_capability cap;     // V4L2 capability structure
    struct v4l2_format fmt;         // V4L2 format structure
    struct v4l2_requestbuffers req; // V4L2 request buffers structure
    struct v4l2_buffer buf;         // V4L2 buffer structure
    void *buffer;                   // buffer to hold captured video frames
    AVFormatContext *format_ctx;    // FFmpeg format context
    AVCodecContext *codec_ctx;      // FFmpeg codec context
    AVStream *stream;               // FFmpeg stream
    AVFrame *frame;                 // FFmpeg video frame
    AVPacket packet;                // FFmpeg packet
    struct SwsContext *sws_ctx;     // FFmpeg scaling context
    int ret, i, j;

    // Open the V4L2 device for reading and writing
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // Get the V4L2 device capability
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        perror("ioctl(VIDIOC_QUERYCAP)");
        exit(1);
    }

    // Check if the device supports video capture
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "Device does not support video capture\n");
        exit(1);
    }

    // Set the V4L2 capture format
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // pixel format of the captured video
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        perror("ioctl(VIDIOC_S_FMT)");
        exit(1);
    }

    // Request V4L2 buffers
    memset(&req, 0, sizeof(req));
    req.count = 4; // number of buffers to request
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(fd, VIDIOC_REQBUFS, &req);
    if (ret < 0) {
        perror("ioctl(VIDIOC_REQBUFS)");
        exit(1);
    }

   // Map V4L2 buffers to user space
    buffer = calloc(req.count, fmt.fmt.pix.sizeimage);
    for (i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
        if (ret < 0) {
            perror("ioctl(VIDIOC_QUERYBUF)");
            exit(1);
        }
        mmap(buffer + buf.m.offset, buf.length, PROT_READ | PROT_WRITE,
             MAP_SHARED, fd, buf.m.offset);
    }

    // Initialize FFmpeg
    av_register_all();
    avformat_alloc_output_context2(&format_ctx, NULL, NULL, "out.mp4");
    if (!format_ctx) {
        fprintf(stderr, "Could not create output context\n");
        exit(1);
    }
    codec_ctx = avcodec_alloc_context3(NULL);
    codec_ctx->codec_id = AV_CODEC_ID_H264;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->width = WIDTH;
    codec_ctx->height = HEIGHT;
    codec_ctx->time_base.num = 1;
    codec_ctx->time_base.den = FPS;
    avcodec_open2(codec_ctx, avcodec_find_encoder(codec_ctx->codec_id), NULL);
    stream = avformat_new_stream(format_ctx, NULL);
    stream->codecpar->codec_id = codec_ctx->codec_id;
    stream->codecpar->codec_type = codec_ctx->codec_type;
    stream->codecpar->width = codec_ctx->width;
    stream->codecpar->height = codec_ctx->height;
    stream->codecpar->format = codec_ctx->pix_fmt;
    stream->codecpar->codec_tag = 0;
    avformat_write_header(format_ctx, NULL);
    frame = av_frame_alloc();
    frame->width = WIDTH;
    frame->height = HEIGHT;
    frame->format = codec_ctx->pix_fmt;
    av_image_alloc(frame->data, frame->linesize, codec_ctx->width,
                   codec_ctx->height, codec_ctx->pix_fmt, 32);

    // Start capturing video frames
    for (i = 0; i < 100; i++) { // capture 100 frames
        // Queue a V4L2 buffer for video capture
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl(fd, VIDIOC_QBUF, &buf);
        if (ret < 0) {
            perror("ioctl(VIDIOC_QBUF)");
            exit(1);
        }

        // Wait for a V4L2 buffer to become available
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl(fd, VIDIOC_DQBUF, &buf);
        if (ret < 0) {
            perror("ioctl(VIDIOC_DQBUF)");
            exit(1);
        }

        // Convert the captured video frame from YUYV to YUV420P using FFmpeg
        sws_ctx = sws_getContext(WIDTH, HEIGHT, AV_PIX_FMT_YUYV422, WIDTH, HEIGHT,
                                 AV_PIX_FMT_YUV420P, 0, NULL, NULL, NULL);
        if (!sws_ctx) {
            fprintf(stderr, "Could not initialize conversion context\n");
            exit(1);
        }
        sws_scale(sws_ctx, (const uint8_t *const *) &buffer + buf.m.offset,
                  &buf.bytesused, 0, HEIGHT, frame->data, frame->linesize);
        sws_freeContext(sws_ctx);

        // Write the converted frame to the output video file
        frame->pts = i;
        avcodec_send_frame(codec_ctx, frame);
        while (avcodec_receive_packet(codec_ctx, &pkt) == 0) {
            av_interleaved_write_frame(format_ctx, &pkt);
            av_packet_unref(&pkt);
        }
    }

    // Clean up resources
    av_write_trailer(format_ctx);
    avcodec_close(codec_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_free_context(format_ctx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    munmap(buffer, req.count * fmt.fmt.pix.sizeimage);
    close(fd);

    return 0;
}

```

## 解析

### 1、结构体解析

#### 结构体 AVFormatContext

`AVFormatContext` 是 FFmpeg 库中的一个结构体，表示输入或输出格式上下文。 它包含有关媒体容器格式的信息以及访问或写入多媒体流所需的参数。

- 对于输入格式，`AVFormatContext` 结构体包含有关输入流格式的信息，例如流的数量、类型、编解码器、比特率、采样率等。 它还包含有关输入文件的信息，例如文件名、文件格式和元数据。

- 对于输出格式，`AVFormatContext` 结构体包含有关输出容器格式的信息，例如容器格式名称、编解码器、比特率等。 它还包含有关输出文件的信息，例如文件名和元数据。

- `AVFormatContext` 结构体在 FFmpeg 函数中用于读取或写入多媒体文件。 例如，要从文件中读取，您可以使用指向 `AVFormatContext` 结构体的指针调用 `avformat_open_input()`，该结构体将填充有关输入文件的信息。 要写入文件，您可以使用指向 `AVFormatContext` 结构体的指针调用 `avformat_alloc_output_context2()`，该结构体将填充有关输出文件的信息。

`AVFormatContext` 是 FFmpeg 中的一个中央数据结构体，库中的许多其他结构体和函数使用它来表示和操作多媒体数据。 它是任何基于 FFmpeg 的媒体处理管道的重要组成部分。

#### 结构体 AVCodecContext

`AVCodecContext`是 FFmpeg 库中的一个结构体，表示编解码器的上下文，它负责对多媒体流进行编码或解码。它包含有关编解码器参数和配置的信息，以及编码或解码过程的当前状态。

`AVCodecContext`用于编码和解码，结构体包含特定于每个操作的字段。例如，在编码时，`AVCodecContext`包含输出格式、码率、质量等字段，而在解码时，包含输入格式、码流过滤器、视频帧大小等字段。

`AVCodecContext`是 FFmpeg 库的重要组成部分，因为它用于初始化编解码器和配置编码或解码多媒体流所需的参数。它还用于维护编码或解码过程的当前状态，例如处理的帧数、比特率和输出流的质量。

除了基本的编解码器参数外，`AVCodecContext`还提供了用于微调编码或解码过程的选项，例如设置各种标志、启用或禁用特定功能以及设置自定义编码或解码功能的能力。

总的来说，`AVCodecContext`是在 FFmpeg 库中使用编解码器的基本结构体，它为处理多媒体数据提供了一个灵活而强大的框架。

#### 结构体 AVStream

`AVStream`是 FFmpeg 库中的一个结构体，表示多媒体流，可以是音频、视频或字幕。它包含有关流属性的信息，例如使用的编解码器、比特率、持续时间和帧数。

每个结构体都与一个表示多媒体容器格式的结构体`AVStream`相关联。一个`AVFormatContext`结构体可以包含一个或多个`AVStream`结构体，每个结构体代表容器内的一个单独的多媒体流。

`AVStream`提供了多个字段来描述多媒体流的属性，包括流索引、编解码器参数、持续时间、开始时间、时基、帧率、比特率等。它还包含指向`AVCodecContext`表示流使用的编解码器的结构体的指针。

`AVStream`是 FFmpeg 库中的基本结构体，因为它用于从多媒体文件中提取或写入多媒体流。它为处理多媒体数据提供了一个灵活而强大的框架，并在整个 FFmpeg 库中广泛使用。

总的来说，`AVStream`是 FFmpeg 库中多媒体处理的关键结构体，它为在多媒体容器中处理多媒体流提供了方便而强大的抽象。

#### 结构体 AVFrame

`AVFrame`是 FFmpeg 库中的一个结构体，表示单个视频或音频帧。它包含多媒体流的单个帧的解码数据，例如视频帧或音频样本。

`AVFrame`提供了许多字段来描述帧的属性，包括数据的大小和格式、表示时间戳 (PTS)、时基以及特定于帧类型的各种其他属性。

`AVFrame`在整个 FFmpeg 库中广泛用于解码和编码多媒体流。解码视频或音频流时，解码的帧存储在`AVFrame`结构体中，然后可以根据需要对其进行处理、过滤或显示。在对视频或音频流进行编码时，`AVFrame`结构体用于存储要编码的数据，然后传递给编码器进行处理。

除了存储解码后的数据外，`AVFrame`还可以用来存储元数据和有关帧的其他信息。例如，一个`AVFrame`结构体可以存储有关视频帧的颜色空间、纵横比和其他属性的信息，或者有关音频样本的采样率、通道布局和其他属性的信息。

总的来说，`AVFrame`它是 FFmpeg 库中用于处理视频和音频帧的基本结构体，它为处理多媒体数据提供了一个灵活而强大的框架。

### 结构体 AVPacket

`AVPacket`是 FFmpeg 库中的一个结构体，表示多媒体流的压缩数据包，例如视频帧或音频样本。它用于编码和解码多媒体流。

`AVPacket`提供了许多字段来描述数据包的属性，包括数据的大小和格式、表示时间戳 (PTS)、解码时间戳 (DTS) 以及特定于数据包类型的各种其他属性。

解码多媒体流时，`AVPacket`用于存储从输入流中读取的压缩数据包。然后将数据包传递给适当的解码器进行解码，并将生成的`AVFrame`结构体存储在内存中以供进一步处理或显示。

编码多媒体流时，`AVPacket`用于存储编码器生成的压缩数据包。然后，数据包在生成时连同任何关联的元数据或其他信息一起写入输出流。

`AVPacket`是 FFmpeg 库中的基本结构体，因为它提供了一个灵活而强大的框架来处理压缩的多媒体数据。它在整个 FFmpeg 库中广泛用于解码和编码多媒体流，并为处理压缩数据包提供了方便的抽象。

总的来说，`AVPacket`是 FFmpeg 库中多媒体处理的重要结构体，它为处理多媒体流中的压缩数据包提供了方便而强大的抽象。

### 2、API解析