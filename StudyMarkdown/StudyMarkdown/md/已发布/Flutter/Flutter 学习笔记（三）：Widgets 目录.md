#  Flutter 学习笔记（三）：Widgets 目录

&emsp;使用 Flutter 的一套的视觉、结构、平台、和交互式的 widgets，快速创建漂亮的 APP。

## 基础组件

&emsp;在构建 Flutter 应用程序之前，绝对要了解的 widgets。下面大概是一大组类似 iOS 中官方提供的 UI 控件，下面一起看一下吧。  

### Container

&emsp;将 common 绘制、定位和大小 widget 相结合的便利 widget。

&emsp;Container 首先用 padding（`final EdgeInsetsGeometry? padding;`）包围 child（由 decoration（`final Decoration? decoration;`）中的任何边框 inflated），然后对填充度施加额外的 constraints（如果两者都不是空的，则将宽度和高度合并为约束）。然后，container 被 margin（`final EdgeInsetsGeometry? margin;`）描述的其他空位包围。

## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
