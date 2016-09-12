* [http://techblog.netflix.com/2016/06/toward-practical-perceptual-video.html](http://techblog.netflix.com/2016/06/toward-practical-perceptual-video.html)
* NetFlix 选出了34个clips，每个6秒，覆盖了各种场景的视频。然后把这34个源用h264压缩成了不同码率/分辨率的视频，一共压了300多个。
* 用这300多个压缩过的视频，找了一堆观众朋友，做了主观测试，为每个压缩过的视频打分。得到的结果作为 VMAF 的参考数据库：NFLX Video Dataset
* The set of reference videos, distorted videos and DMOS scores from observers will be referred to in this article as the NFLX Video Dataset.
* NetFlix 以主观测试的结果（DMOS）作为基准，对比了 PSNR、SSIM等客观测试的结果，得出结论：PSNR、SSIM不能准确反映人的主观感受。
* 拿PSNR、SSIM等与DMOS做对比时，引入了SRCC、PCC、RMSE这三个东西。这是三个数学量，我理解为反映两组非线性关系的数值的相似度的。
* 比如 PSNR 和 DMOS 的相似度。我有300个视频的 DMOS 值，又测出了这300个视频的 PSNR 值，然后拿这两组值计算一下 SRCC，就能一定程度上反映出 PSNR 和 DMOS 是否接近。PCC 和 RMSE 同理。
* SRCC 和 PCC 是越接近1越好（越大越好），RMSE 是越接近0越好（越小越好）。

		SRCC and PCC values closer to 1.0 and RMSE values closer to zero are desirable

* NetFlix 引入了自己的客观视频质量算法：VMAF。其目的是尽可能的反映出人的主观感受，即尽可能逼近 DMOS。
* VMAF的想法跟VQM是一致的。
* NetFlix 开源了测试 VMAF的工具：[https://github.com/Netflix/vmaf](https://github.com/Netflix/vmaf)