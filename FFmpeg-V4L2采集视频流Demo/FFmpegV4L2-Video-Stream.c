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
