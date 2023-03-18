# FFmpeg PCM转AAC

## 流程



![image-20230318212610214](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230318212610214.png)

- 首先，初始化输入和输出的格式上下文，这是用于存储输入和输出文件的格式信息的结构体。

- 然后，打开输入文件并读取流信息，这是用于获取输入文件中包含的音频、视频等流的信息。

- 接着，找到音频流的索引，这是用于确定输入文件中哪个流是我们要处理的音频流。

- 再接着，设置输出文件的格式和编码器参数，这是用于指定输出文件的容器格式（如MP4）和音频编码器（如AAC）以及相关参数（如比特率、采样率等）。

- 然后，初始化解码器上下文和解码器，这是用于存储解码器相关信息和创建解码器实例的步骤。

- 接着，复制解码器参数到解码器上下文中，这是用于将输入文件中读取到的解码器参数（如编码格式、声道数等）赋值给解码器上下文结构体。

- 再接着，打开解码器，这是用于准备好解码器开始工作的步骤。

- 然后，初始化音频重采样上下文和参数，这是用于存储重采样相关信息和创建重采样实例的步骤。重采样是指将一种音频格式转换为另一种音频格式（如改变采样率、声道数等）。

- 接着，初始化输入和输出的音频帧，这是用于存储音频数据和相关信息的结构体。一个帧表示一段时间内连续采样得到的音频数据。

- 再接着，分配输出帧的缓冲区，这是用于为输出帧分配足够大小的内存空间来存储转换后的音频数据。

- 然后，在一个循环中不断读取输入文件中的数据包。一个数据包表示一段压缩编码后得到的二进制数据。如果读取成功，则进行以下步骤：

  - 解码输入数据包到输入帧。这是将压缩编码后得到二进制数据还原成原始PCM数据。

  - 重采样输入帧到输出帧。这是将原始PCM数据转换成目标AAC需要使用或支持使用或更适合使用或更优化使用或更高效使用或更节省空间使用或更兼容使用或更标准化使用或更通用使用或更易扩展使用或更易维护使用或其他原因需要转换成另一种PCM格式。

  - 编码输出帧到输出数据包。这是将转换后得到PCM数据压缩编码成二进制数据。

  - 写入输出数据包到输出文件。这是将压缩编码后得到二进制数据写入目标AAC文件。

  - 释放资源。这是释放已经不再需要使用或占据内存空间而影响性能与效率与稳定性与安全性与可靠性与其他方面而需要释放掉已经分配给相应变量与结构体与对象与实例与指针等资源以便回收利用而避免造成内存泄漏等问题而进行释放操作。

- 如果读取失败，则结束循环，并关闭所有打开过并且没有关闭过并且还需要关闭以避免造成错误与异常与问题而进行关闭操作，并释放所有分配过并且没有释放过并且还需要释放以避免造成内存泄漏等问题而进行释放操作，并退出程序。

## 技术点

这个过程中涉及到了一些FFmpeg使用的关键技术和关键知识点，如：

- 格式上下文（Format Context）：用于存储输入或输出文件的格式信息，如容器格式、流信息等。
- 数据包（Packet）：用于存储压缩编码后得到的二进制数据，以及一些元数据，如时间戳、持续时间等。
- 帧（Frame）：用于存储原始或转换后得到的音频或视频数据，以及一些元数据，如采样率、声道数等。
- 解码器上下文（Codec Context）：用于存储解码器相关信息，如编码格式、比特率等。
- 解码器（Codec）：用于将压缩编码后得到二进制数据还原成原始PCM数据或将原始PCM数据压缩编码成二进制数据的实例。
- 重采样上下文（Resample Context）：用于存储重采样相关信息，如输入和输出的采样率、声道数等。
- 重采样（Resample）：用于将一种音频格式转换为另一种音频格式的实例。

新手要掌握这个流程，可以参考网上搜索得到的一些教程²³⁴⁵。这些教程会介绍FFmpeg的基本概念和常用命令，并给出一些示例代码和解释。

这个流程的核心是**解码**和**编码**。解码是指将压缩编码后得到二进制数据还原成原始PCM数据。编码是指将原始PCM数据压缩编码成二进制数据。这两个步骤是实现音频格式转换的关键。

FFmpeg在这个流程中的使用要领是：

- 使用正确的参数来打开输入和输出文件，并读取或设置相应的格式信息。
- 使用正确的参数来初始化和打开解码器和编码器，并复制或设置相应的解码器参数。
- 使用正确的参数来初始化和分配输入和输出帧，并设置或获取相应的帧信息。
- 使用正确的函数来读取、解码、重采样、编码、写入并释放相应的数据包和帧，并检查返回值是否正常。
- 使用正确的函数来关闭并释放相应的文件、上下文、解码器、编码器等资源，并避免内存泄漏等问题。

参考文献：

[(1) Tutorial de ffmpeg - Línea de comandos en español.]( https://terminaldelinux.com/terminal/multimedia/ffmpeg/)

[(2) ffmpeg Tutorial => Getting started with ffmpeg.]( https://riptutorial.com/ffmpeg)

[(3) StreamingGuide – FFmpeg.](https://trac.ffmpeg.org/wiki/StreamingGuide ) 

[(4) GitHub - mpenkov/ffmpeg-tutorial: A set of tutorials that demonstrates ](https://github.com/mpenkov/ffmpeg-tutorial/)

[(5) 3 Ways to Convert Media with FFmpeg - wikiHow.](https://www.wikihow.com/Convert-Media-with-FFmpeg)

### FFmpeg 上下文

#### 定义

在FFmpeg中，上下文（Context）是一种用于存储和传递相关信息的数据结构。它的本质是一个结构体（Struct），包含了一些成员变量和指针，用于描述某个对象或过程的属性和状态。上下文可以通过相应的函数来初始化、分配、设置、获取、释放等操作。

代码中涉及到的不同的上下文有不同的作用，例如：

- 格式上下文（Format Context）：用于存储输入或输出文件的格式信息，如容器格式、流信息等。可以通过avformat_alloc_context()函数来分配一个新的格式上下文，并通过avformat_free_context()函数来释放它。
- 解码器上下文（Codec Context）：用于存储解码器相关信息，如编码格式、比特率等。可以通过avcodec_alloc_context3()函数来分配一个新的解码器上下文，并通过avcodec_free_context()函数来释放它。
- 重采样上下文（Resample Context）：用于存储重采样相关信息，如输入和输出的采样率、声道数等。可以通过swr_alloc()函数来分配一个新的重采样上下文，并通过swr_free()函数来释放它。

参考文献：

​	[(1) Creating multiple outputs – FFmpeg.](https://trac.ffmpeg.org/wiki/Creating%20multiple%20outputs)

​	[(2) FFmpeg: AVFormatContext Struct Reference.]( https://www.ffmpeg.org/doxygen/trunk/structAVFormatContext.html )

[	(3) ffmpeg Tutorial => Open a codec context.]( https://riptutorial.com/ffmpeg/example/30961/open-a-codec-context )

#### 编程范式

**FFmpeg使用上下文的编程范式可以概括为以下几个步骤：**

- 分配一个新的上下文，并设置或复制相应的参数。
- 打开或初始化相应的对象或过程，并将上下文与之关联。
- 使用相应的函数来操作或访问上下文中的数据或状态。
- 关闭或释放相应的对象或过程，并断开与上下文的关联。
- 释放上下文并清理内存。

**例如，如果要使用FFmpeg进行音频格式转换，可以按照以下范式编写代码：**

- 分配一个新的格式上下文，并使用avformat_open_input()函数打开输入文件。
- 分配一个新的解码器上下文，并使用avcodec_parameters_to_context()函数复制输入文件中音频流的解码器参数。
- 使用avcodec_find_decoder()函数找到合适的解码器，并使用avcodec_open2()函数打开解码器并将解码器上下文与之关联。
- 分配一个新的重采样上下文，并使用swr_alloc_set_opts()函数设置输入和输出音频格式的参数。
- 使用swr_init()函数初始化重采样过程并将重采样上下文与之关联。
- 分配一个新的帧，并使用av_frame_get_buffer()函数分配内存空间用于存储输出音频数据。
- 使用av_read_frame()函数循环读取输入文件中音频流的数据包，并使用avcodec_send_packet()和avcodec_receive_frame()函数将数据包解码成原始帧。
- 使用swr_convert_frame()函数将原始帧重采样成目标帧，并根据需要处理输出音频数据（如写入文件、播放等）。
- 使用av_packet_unref()和av_frame_unref()函数释放数据包和帧占用的内存空间。
- 使用swr_close()和swr_free()函数关闭并释放重采样过程和重采样上下文。
- 使用avcodec_close()和avcodec_free_context()函数关闭并释放解码器和解码器上下文。
- 使用avformat_close_input()和avformat_free_context()函数关闭并释放输入文件和格式上下文。

#### Example

- 创建一个自定义的IO上下文（IO Context），用于从内存中读取或写入数据：

```c
// Allocate memory for IO buffer
unsigned char* io_buffer = (unsigned char*)av_malloc(IO_BUFFER_SIZE);

// Allocate memory for IO context
AVIOContext* io_context = avio_alloc_context(
    io_buffer,
    IO_BUFFER_SIZE,
    0,
    NULL,
    &read_packet,
    &write_packet,
    &seek);

// Allocate memory for format context
AVFormatContext* format_context = avformat_alloc_context();

// Set IO context to format context
format_context->pb = io_context;
```

- 使用预设文件（Preset File）来设置解码器上下文（Codec Context）的参数：

```c
// Find a video encoder by name
AVCodec *codec = avcodec_find_encoder_by_name("libx264");

// Allocate memory for codec context
AVCodecContext *codec_context = avcodec_alloc_context3(codec);

// Open a preset file by name and codec
FILE *f = get_preset_file("medium", "libx264", 0);

// Parse the preset file and set options to codec context
parse_preset_file(codec_context, f);
```

- 添加元数据信息（Metadata）到格式上下文（Format Context）中：

```c
// Create an output format context by filename
avformat_alloc_output_context2(&format_context, NULL, NULL, filename);

// Add a video stream to the output format context
video_stream = avformat_new_stream(format_context, NULL);

// Add some custom metadata to the output format context
av_dict_set(&format_context->metadata, "title", "My Video", 0);
av_dict_set(&format_context->metadata, "author", "Me", 0);
av_dict_set(&format_context->metadata, "comment", "This is a test video", 0);

// Write header to the output file
avformat_write_header(format_context, NULL);
```

参考文献：

[(1) Creating Custom FFmpeg IO-Context - CodeProject.](https://www.codeproject.com/tips/489450/creating-custom-ffmpeg-io-context)

[(2) ffmpeg Documentation. ](https://ffmpeg.org/ffmpeg.html )

[(3) c++ - Adding metadata informations with ffmpeg - Stack Overflow.]( https://stackoverflow.com/questions/45050177/adding-metadata-informations-with-ffmpeg )