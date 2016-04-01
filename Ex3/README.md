# 模式识别HW3

标签: 作业

---
作业需要完成的功能在`src\Ex3.cpp`中实现。

测试环境：Windows 10 64bit
编译工具：MinGW gcc version 4.8.1
目录结构：
```
_
 |-in1                  // 测试用输入命令，每个文件对应不同的数据集
 |-in2
 |-in3
 |-README.md            // 本文件
 |-Dataset              // 数据集1
   |-1.jpg              // n个测试样例文件
   |-……
   |-6.jpg
   |-n.jpg_vote.bmp     // 每个测试样本对应的中间输出，包括投票结果，识别出的边缘
   |-n_draw.jpg
   |-n_draw.jpg_a4.jpg  // 每个测试文件对应的最终输出
 |-Dataset2             // 数据集2
   |……
 |-Dataset3             // 数据集3
   |……
 |-bin
   |-Ex3.exe            // 编译好的测试文件
 |-src                  // 源文件
   |-Ex3.cpp
   |-myImg.h
   |-CImg.h
```

`src\Ex2.cpp`已实现测试功能，可接受多组测试，要求的输入格式如下：
```
测试个数n
第1组测试文件名
……
第n组测试文件名
```
默认输出到源文件目录下。
根文件夹下的in文件中有示例输入。
