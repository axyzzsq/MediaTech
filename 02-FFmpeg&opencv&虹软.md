# FFmpeg+opencv+虹软人脸检测框架

项目核心是在Linux平台上利用摄像头采集人脸，并进行人脸识别。这个项目使用的是FFMPEG+OPENCV+虹软框架完成。

- FFMPEG的主要工作是负责采集摄像头的数据并把摄像头数据发送给opencv。

- Opencv的主要工作则是把摄像头数据转换成矩阵数据。

- 虹软的主要功能则是利用Opencv的数据进行数据检测和识别并且和人脸数据库进行比较，如果识别成功则显示这个人的姓名，并把数据显示出来。

  目前整体项目只是简单的一个demo，后期会根据需求来进行开发扩展的，比如添加ui界面来查看！

![image-20230103150535504](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230103150535504.png)

- **视频采集线程(read_camera_thread）**：利用FFMPEG去进行采集摄像头的数据，并把摄像头的YUV存放到缓存队列里面，这里的缓存队列使用的是`AVFifoBuffer`进行存储(操作的API是`av_fifo_generic_write`写入YUV数据);
- **虹软识别线程(process_asfort_recognize_thread)**：从`AVFifoBuffer`队列里面取出YUV数据(操作的API是`av_fifo_generic_read`读取YUV数据)并利用OPENCV框架进行转换，转换完成之后，利用虹软人脸识别API进行识别操作。识别完成之后，则把识别结果存放到显示队列`putMatQueue`里面;
- **视频显示线程(show_opencv_thread)**：从`opencv_queue`队列取出每一帧数据(`getMatQueue`操作)处理过后的数据，显示到播放器里面。

**重点：这个项目重点在于多线程和队列的操作，一般一个大型的项目都需要用到多线程的操作。因为使用多线程能够充分利用CPU资源，并快速响应。这个项目使用了三个线程去处理，分别是视频采集线程、 虹软识别线程、 视频显示线程。而线程之间的通信则利用队列进行通信。**



## 第1节 asfort_face_insert

![image-20230103155303423](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230103155303423.png)

[参考文献]

- [strftime()函数的用法](https://blog.csdn.net/lwmjm/article/details/8156648)
- [sqlite3数据库基本操作](https://blog.csdn.net/gangtienaner/article/details/119575172?spm=1001.2101.3001.6650.4&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-4-119575172-blog-100712844.pc_relevant_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-4-119575172-blog-100712844.pc_relevant_default&utm_relevant_index=9)
- [sqlite3接口说明](https://xiaoyege.blog.csdn.net/article/details/86484547?spm=1001.2101.3001.6650.12&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-12-86484547-blog-6530529.pc_relevant_multi_platform_whitelistv2_ad_hc&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-12-86484547-blog-6530529.pc_relevant_multi_platform_whitelistv2_ad_hc&utm_relevant_index=20)
- [cv::mat的使用](https://blog.csdn.net/czsnooker/article/details/118345494)
- [imread函数的使用](https://blog.csdn.net/aijie099/article/details/104392143)
- [resize的使用](https://yangyongli.blog.csdn.net/article/details/121449412?spm=1001.2101.3001.6650.4&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-4-121449412-blog-114271178.pc_relevant_multi_platform_whitelistv1_exp2&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-4-121449412-blog-114271178.pc_relevant_multi_platform_whitelistv1_exp2&utm_relevant_index=6)



## 第2节 read_camera_thread

![image-20230104111703349](https://pic-1304959529.cos.ap-guangzhou.myqcloud.com/DB/image-20230104111703349.png)

[参考文献]

- [FFmpeg通过摄像头实现对视频流进行解码并显示测试代码(新接口)](https://blog.csdn.net/fengbingchun/article/details/93975844)
- [雷霄骅-最简单的基于FFmpeg的AVDevice例子（读取摄像头)](https://blog.csdn.net/leixiaohua1020/article/details/39702113?spm=1001.2101.3001.6650.4&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-4-39702113-blog-89822927.pc_relevant_multi_platform_whitelistv1_exp2&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-4-39702113-blog-89822927.pc_relevant_multi_platform_whitelistv1_exp2&utm_relevant_index=9)
- [雷霄骅-最简单的基于FFmpeg的libswscale的示例（YUV转RGB）](https://blog.csdn.net/leixiaohua1020/article/details/42134965?spm=1001.2101.3001.6650.11&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-11-42134965-blog-89289979.pc_relevant_multi_platform_featuressortv2dupreplace&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-11-42134965-blog-89289979.pc_relevant_multi_platform_featuressortv2dupreplace&utm_relevant_index=17)
- [FFmpeg之sws_getContext处理图像函数分析](https://blog.csdn.net/e891377/article/details/126724504?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EAD_ESQUERY%7Eyljh-1-126724504-blog-121206818.pc_relevant_3mothn_strategy_and_data_recovery&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EAD_ESQUERY%7Eyljh-1-126724504-blog-121206818.pc_relevant_3mothn_strategy_and_data_recovery&utm_relevant_index=2)
- [详解YUV数据格式](https://blog.csdn.net/lucky_greenegg/article/details/9942619?spm=1001.2101.3001.6650.3&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-3-9942619-blog-118542594.pc_relevant_sortByStrongTime&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-3-9942619-blog-118542594.pc_relevant_sortByStrongTime&utm_relevant_index=6)
- [FFMpeg 实现视频编码、解码、封装、解封装、转码、缩放](https://lazybing.github.io/blog/2017/01/01/ffmpeg-sdk-learning/)
- [存储格式：packed和planar](https://www.cnblogs.com/faithlocus/p/15659383.html)