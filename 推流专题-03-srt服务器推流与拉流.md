# srt服务器搭建与推拉视频流

## 一、srt服务器搭建

- Step1:源码安装srt

  ```shell
  sudo git clone https://github.com/Haivision/srt.git
  cd srt
  sudo ./configure
  sudo make -j8
  sudo make install
  ```

- Step2:源码安装srt-live-server

  ```shell
  sudo git clone https://github.com/Edward-Wu/srt-live-server.git
  cd srt-live-server
  sudo make -j8
  cd bin
  ./sls –h
  ```

- Step3:启动`srt`服务器

  ```shell
  book@100ask:~/share/srt_server/srt-live-server/bin$ ./sls -c ../sls.conf 
  ```

  ![](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230402233523647.png)

## Trouble Shooting

- 执行`./sls –h`的时候报错:

  ![image-20230402232721186](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230402232721186.png)

  解决办法：

  ```shell
  sudo echo "export LD_LIBRARY_PATH=/home/book/share/srt_server/srt/" >> ~/.bashrc
  source ~./bashrc
  ```

  然后重新进入`srt-live-server`目录，执行`./sls –h`

![image-20230402233231084](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230402233231084.png)



## 二、工程运行

- 板端把视频流推送给局域网内的srt服务器:

  ```shell
  ./rv1126_ffmpeg_main 1 srt://192.168.1.104:8080?streamid=uplive.sls.com/live/cz_01
  ```

- 局域网内ffplay拉流：

  ```shell
  ffplay -x 400 -y 400 srt://192.168.1.104:8080?streamid=live.sls.com/live/cz_01 -fflags nobuffer
  ```

  ![image-20230402234320988](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230402234320988.png)

