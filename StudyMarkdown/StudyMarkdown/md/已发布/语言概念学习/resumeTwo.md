## 联系方式

+ 电话：18101219127
+ 邮箱：hm_dev@outlook.com
+ 博客：https://juejin.cn/user/1591748569076078
  (目前共产出 122 篇 iOS 相关的文章，累计 794 位关注者) 

## 个人简介

+ 程贺明，男，1992.09
+ 黄河科技大学，计算机科学与技术专业，统招本科：12-16，16 年毕业工作至今，有 6 年 iOS 开发经验

## 工作经历

1. 北京小呀教育科技有限公司（2021.03 - 至今）
  + 负责考呀呀 APP 的日常需求开发和维护
  + 基础架构改造搭建、崩溃统计与处理以及防劣化、组件化方案设计等等
2. 北京微赢互动科技有限公司（2019.08 - 2021.03）
  + 负责斗地主游戏中内嵌原生部分以及广告 APP 开发
3. 北京悉见科技有限公司（2018.05 - 2019.08）
  + 开发 iOS 原生 AR APP 和与 Unity 混合交互的 APP 和游戏
  + 与算法工程师对接完成 Relocation(重定位) framework 封装及优化
  + OpenCV、OpenGL、Visp、Vuforia、ARToolKit 等库在 iOS 的实践与应用的尝试 

## 部分项目经历

&emsp;独立开发加合作开发的项目大概有 10+，其中主要项目如下:

1. 考呀呀 RN + Swift + OC 混编。
+ 模块化拆分，壳工程 + 各个业务模块的 pod 引入
+ 题库、录播、直播、下载、IM 等各功能模块开发 

2. Relocation 项目语言 Swift +(OC、C++、C)混编。核心特点(难点、技术点): 
+ 多个 C、C++ 库打成 iOS 可用的 framework(OpenCV、Eigen、Boost、FLANN、FreeImage 等)
+ 多 target 输出，会议室、办公区、超市等不同的场景使用不同特征点数据输出对应的 target
+ 视频逐帧拆解处理，处理完的图片合成视频，图片视频加水印
+ ARKit 辅助定位，SceneKit 构建场景，模型加载及交互以及 Metal 的使用 
+ CoreML 使用深度学习训练的模型做视频实时人体分割、物体标注等

3. 彝人悉游 项目语言 Swift + C，架构 RxSwift + MVVM。核心特点(难点、技术点):
+ Unity + iOS 混合开发
+ 地图开发，限制区域、绘制路线、自定义锚点、覆盖手绘图、多级处理、临近景点播放音频等
+ 音频视频在线播放和本地缓存，断点续传及后台下载
+ CoreGraphics、CoreImage、CoreAnimation 绘图图片处理动画制作
+ Webview 与 native 交互，性能优化 web 缓存 
 
4. 智能家居 项目语言 OC +(部分 C)，重构时在组内推进 RAC + MVVM。核心特点(难点、技术点):
+ 移动网络、局域网内使用，多用户多手机设备，数据同步和数据备份
+ Runtime 遍历 model 属性构建数据库操作语句高度封装 FMDB
+ 多种类型、同类多个、虚拟房间分组、房间绑定、开关状态推送信息等本地保存和同步
+ 国际化，一个 workspace 多个target 功能模块类似 cocoapods 导入后的项目架构
+ 封装 iOS 10 和 iOS 10 以下的两套本地推送和远程推送。 

## 个人成长

1. code review、读书分享、技术分享，为了培养开发团队的技术氛围，部⻔不定期进行代码集体 review、读书分享和技术分享，其中由我进行的: 
+ 《代码大全》章节阅读分享
+ AR 知识点及 ARKit 核心技术点分享，给团队其他小伙伴补充 AR 知识
+ Mach-O 结构知识点分析
+ ...

2. 个人阅读书籍清单：《程序员的自我修养--链接、装载与库》、《操作系统--精髓与设计原理》、《深入解析 macOS iOS 操作系统》、《你不知道的 JavaScript 上、中、下》、《深入理解计算机系统》、《汇编语言(第3版)》、《C Primer Plus》、《C++ Primer Plus》、《Swift 异步和并发》、《SwiftUI 与 Combine 编程》、《Effective C++(52 个)》、《从 C++ 到 Objective-C》、《Objective-C 高级编程 iOS 与 OS X 多线程和内存管理》、《Effective Objective-C 2.0(52个)》、《Swift 进阶》、《函数式 Swift》、《iOS 开发进阶》、《iOS Auto Layout 开发秘籍》、《iOS UI 开发捷径》、《高性能 iOS 应用开发》、《iOS 面试之道》、《图解 HTTP》、《图解 TCP/IP》、《HTTP 权威指南》、《iOS Core Animation:Advanced Techniques》、《JavaScript 面向对象编程 指南》、《剑指 offer》...
3. 掘金个人博客，通读 Apple 的五份源码: objc4-781、libdispatch-1173.40.5、CF-1151.16、libmalloc-283.100.6、libclosure-74 并输出文章，基本对 iOS 的底层原理都有一个详细的认知。 

## 其他 

+ 为人谦虚亲和，工作态度严谨端正、一丝不苟。
+ 自驱力强、好学、渴望交流与分享。希望成为一名优秀的软件工程师。



 







