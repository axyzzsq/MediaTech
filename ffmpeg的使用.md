# FFmpeg

## 1、FFmpeg源码安装

- 源码备份:

  ```shell
  链接：https://pan.baidu.com/s/1sJwIFvISXeQYdLwL5nUU1A 
  提取码：ma38 
  ```

- Step1:创建目录

  ```shell
  cd /usr/
  mkdir ffmpeg4.1
  ```

- Step2: 安装yasm

  ```shell
  wget http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
  tar xvzf yasm-1.3.0.tar.gz
  cd yasm-1.3.0
  ./configure
  sudo make
  make install
  ```

  ![image-20221219165823521](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20221219165823521.png)

- Step2: 安装nasm(2.13以上版本)

  ```shell
  wget https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.bz2
  tar xvf nasm-2.14.02.tar.bz2
  cd nasm-2.14.02
  ./configure
  sudo make
  make install
  ```

  ![image-20221219165924848](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20221219165924848.png)

- Step3: 安装Cmake

  ```shell
  apt install cmake -y
  ```

- Step4:安装其他依赖

  x264下载地址

  ```shell
  http://ftp.videolan.org/pub/videolan/x264/snapshots/
  ```

  ```shell
  tar xvf x264-snapshot-20191024-2245-stable.tar.bz2
  cd x264-snapshot-20191024-2245-stable
  ./configure --enable-static --prefix=../x264 --enable-pic 
  sudo make -j16
  make install
  ```

  ![image-20221219193046141](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20221219193046141.png)

- Step6: 编译ffmpeg

  源码下载地址：

  ```shell
  https://ffmpeg.org/releases/?C=M;O=D
  ```

  ```shell
  cd ffmpeg-4.1.3
  export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:../x264/lib/pkgconfig
  ./configure --enable-shared --enable-nonfree --enable-gpl --enable-pthreads --enable-libx264 --prefix=../ffmpeg
  make -j16
  make install
  ```

  ![image-20221219193147119](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20221219193147119.png)

  ![image-20221219202709631](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20221219202709631.png)

  **注：如果报错找不到x264或x265，ubunut下多半是因为没有安装pkg-config。安装命令为：apt install pkg-config**

  