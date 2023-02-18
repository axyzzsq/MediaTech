# FFmpeg V4L2 Video Stream

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

[github源码地址](https://github1s.com/axyzzsq/Audio-Video-Tech/blob/main/FFmpeg-V4L2%E9%87%87%E9%9B%86%E8%A7%86%E9%A2%91%E6%B5%81Demo/FFmpegV4L2-Video-Stream.c)

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

### 1、V4L2结构体解析

#### 结构体 v4l2_capability

`v4l2_capability`是 Video4Linux (V4L2) API 中的一个结构，它提供有关视频捕获设备功能的信息。它用于查询 V4L2 设备的功能并确定支持哪些功能和操作。

该`v4l2_capability`结构包含许多提供设备信息的字段，包括驱动程序名称、卡名称和设备支持的 V4L2 API 版本。它还包含描述设备功能的字段，例如支持的输入和输出格式、最大分辨率和帧速率以及支持的任何其他功能。

通过查询`v4l2_capability`结构，应用程序可以确定 V4L2 设备支持哪些功能和操作，并相应地调整其行为。例如，应用程序可能会检查支持的视频格式并选择适当的格式来从设备捕获或流式传输视频。或者它可能会检查最大帧速率并调整捕获速率以避免丢帧。

总的来说，该`v4l2_capability`结构是 V4L2 API 的重要组成部分，因为它提供了一种查询和确定视频捕获设备功能的方法，并相应地调整应用程序的行为。

#### 结构体 v4l2_format

`v4l2_format`是 Video4Linux (V4L2) API 中的一个结构体，用于指定视频捕获或输出流的格式。用于设置或查询V4L2设备采集或输出的视频数据格式。

该`v4l2_format`结构体包含许多描述视频数据格式的字段，包括图像宽度和高度、像素格式以及图像缓冲区的大小。它还包含描述与视频数据关联的任何元数据或其他信息的字段。

当用于设置视频捕获或输出流的格式时，该`v4l2_format`结构体将传递给 V4L2 API 函数`VIDIOC_S_FMT`，该函数设置设备捕获或输出的视频数据的格式。当用于查询视频捕获或输出流的格式时，该`v4l2_format`结构体将传递给 V4L2 API 函数`VIDIOC_G_FMT`，该函数检索设备捕获或输出的视频数据的当前格式。

通过使用`v4l2_format`结构体来设置或查询视频捕获或输出流的格式，应用程序可以确保以与系统其他部分（如显示设备、视频编解码器）兼容的格式捕获或输出视频数据，或其他软件组件。

总体而言，该`v4l2_format`结构体是 V4L2 API 的重要组成部分，因为它提供了一种灵活而强大的方式来设置或查询视频捕获或输出流的格式，并确保以兼容的格式捕获或输出视频数据。

#### 结构体 v4l2_requestbuffers

`v4l2_requestbuffers`是 Video4Linux (V4L2) API 中的一个结构体，用于请求用于存储视频数据的缓冲区。它用于分配将保存捕获或输出视频数据的内存缓冲区，并为捕获或输出视频数据准备设备。

`v4l2_requestbuffers`结构体包含许多描述正在请求的缓冲区的字段，包括缓冲区的数量、缓冲区的类型（例如内存映射或用户空间）以及缓冲区的大小。它还包含指定分配缓冲区所需的任何其他参数的字段，例如对齐、偏移和其他特定于设备的参数。

当用于请求用于存储视频数据的缓冲区时，该`v4l2_requestbuffers`结构体将传递给 V4L2 API 函数`VIDIOC_REQBUFS`，该函数分配所需的内存缓冲区并为捕获或输出视频数据准备设备。设备驱动程序可以根据需要分配额外的内存或执行其他操作以支持请求的缓冲区配置。

通过使用`v4l2_requestbuffers`结构请求用于存储视频数据的缓冲区，应用程序可以确保设备已准备好捕获或输出视频数据，并且所需的内存缓冲区可供使用。

总体而言，该`v4l2_requestbuffers`结构体是 V4L2 API 的重要组成部分，因为它提供了一种分配和准备用于存储视频数据的内存缓冲区的方法，并确保设备在需要时准备好捕获或输出视频数据。

#### 结构体 v4l2_buffer

`v4l2_buffer`是 Video4Linux (V4L2) API 中的一个结构体，用于在设备和应用程序之间传输视频数据。它用于表示单个视频数据缓冲区，并包含有关缓冲区状态、数据和元数据的信息。

该`v4l2_buffer`结构体包含一些描述缓冲区的字段，包括缓冲区索引、缓冲区的内存位置、缓冲区的大小和缓冲区的状态（例如是否已填充视频数据或是否可用）. 它还包含提供有关缓冲区的附加元数据的字段，例如视频数据的时间戳，或与视频数据关联的任何标志或元数据。

当用于在设备和应用程序之间传输视频数据时，该`v4l2_buffer`结构体被传递给 V4L2 API 函数`VIDIOC_QBUF`，该函数将缓冲区排队等待设备驱动程序处理。设备驱动程序可以在捕获过程中用视频数据填充缓冲区，或者在输出过程中使用缓冲区输出视频数据。一旦设备驱动程序处理了缓冲区，应用程序就可以通过调用函数从缓冲区中检索视频数据`VIDIOC_DQBUF`。

通过使用该`v4l2_buffer`结构体在设备和应用程序之间传输视频数据，应用程序可以确保设备驱动程序正确缓冲和处理视频数据，并且任何关联的元数据或信息都与视频数据一起正确传输。

总体而言，该`v4l2_buffer`结构体是 V4L2 API 的重要组成部分，因为它提供了一种灵活高效的方式来在设备和应用程序之间传输视频数据，并确保视频数据在捕获或输出过程中得到正确的缓冲和处理。



### 2、FFmpeg结构体解析

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

#### 结构体 AVPacket

`AVPacket`是 FFmpeg 库中的一个结构体，表示多媒体流的压缩数据包，例如视频帧或音频样本。它用于编码和解码多媒体流。

`AVPacket`提供了许多字段来描述数据包的属性，包括数据的大小和格式、表示时间戳 (PTS)、解码时间戳 (DTS) 以及特定于数据包类型的各种其他属性。

解码多媒体流时，`AVPacket`用于存储从输入流中读取的压缩数据包。然后将数据包传递给适当的解码器进行解码，并将生成的`AVFrame`结构体存储在内存中以供进一步处理或显示。

编码多媒体流时，`AVPacket`用于存储编码器生成的压缩数据包。然后，数据包在生成时连同任何关联的元数据或其他信息一起写入输出流。

`AVPacket`是 FFmpeg 库中的基本结构体，因为它提供了一个灵活而强大的框架来处理压缩的多媒体数据。它在整个 FFmpeg 库中广泛用于解码和编码多媒体流，并为处理压缩数据包提供了方便的抽象。

总的来说，`AVPacket`是 FFmpeg 库中多媒体处理的重要结构体，它为处理多媒体流中的压缩数据包提供了方便而强大的抽象。

### 3、API解析