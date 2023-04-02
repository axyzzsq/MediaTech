# Nginx流媒体服务器搭建与工程运行

## 1、Nginx搭建

- Step1：下载依赖库

  ```shell
  sudo apt-get install build-essential libpcre3 libpcre3-dev libssl-dev
  ```

- Step2：下载`nginx-1.21.6.tar.gz`

  ```shell
  wget http://nginx.org/download/nginx-1.21.6.tar.gz
  ```

- Step3：下载`nginx-rtmp-module`

  ```shell
  wget https://github.com/arut/nginx-rtmp-module/archive/master.zip
  ```

- Step4：解压

  ```shell
  # 解压nginx文件
  tar -zxvf nginx-1.21.6.tar.gz
  # 解压rtmp模块
  unzip master.zip
  ```

- Step5：编译

  ```shell
  # 进入解压后的nginx路径
  cd nginx-1.21.6
  # 编译配置:
  ./configure --with-http_ssl_module --add-module=../nginx-rtmp-module-master
  # 编译
  make
  # 安装
  sudo make install
  ```

- Step6：启动`nginx`，检测`nginx`是否能成功运行(没有报错就是运行成功了)

  ```shell
  sudo /usr/local/nginx/sbin/nginx
  ```

- Step7：配置`nginx`使用`RTMP`

  `/usr/local/nginx/conf/nginx.conf`

  ```json
  rtmp {
  	server {
  		listen 1935;
  		chunk_size 4096;
  		application live {
              live on;
              record off;
          }
  	}
  }
  ```

- Step8：重启Nginx服务器

  ```shell
  sudo /usr/local/nginx/sbin/nginx -s stop
  sudo /usr/local/nginx/sbin/nginx
  ```

## 2、工程运行

- Step1：Ubuntu编译

  ```shell
  book@100ask:~/share/rv1126_rv1109_linux_sdk_v1.8.0_20210224/mydemo/rv1126_ffmpeg_project$ make
  ```

- Step2：板端执行推流到Nginx服务器：

  ```shell
  [root@RV1126_RV1109:/mnt/nfs]# ./rv1126_ffmpeg_main 0 rtmp://192.168.31.177:1935
  ```

- Step3：局域网内ffplay从Nginx拉流

  ```shell
  E:\MyEVB-RV1126\doc\ffmpeg-20200504-5767a2e-win64-static\bin> ffplay -x 800 -y 800 rtmp://192.168.31.177:1935/live/01 -fflags nobuffer
  ```

![image-20230402194603526](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230402194603526.png)