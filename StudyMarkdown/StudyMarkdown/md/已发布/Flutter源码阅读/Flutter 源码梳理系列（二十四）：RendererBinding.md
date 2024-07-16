# Flutter 源码梳理系列（二十四）：RendererBinding

# runApp

&emsp;实例化（inflate）给定的 widget 并将其附加到视图中。

&emsp;runApp 方法将提供的应用程序 widget 呈现到 PlatformDispatcher.implicitView 中，将其包装在一个 View widget 中，这将为应用程序启动 render tree。想要控制要渲染到哪个 FlutterView 的应用程序可以使用 runWidget。

&emsp;在布局期间，该 widget 会被给予强制使其填充整个视图的约束。如果你希望将你的 widget 对齐到视图的一侧（例如顶部），考虑使用 Align widget。如果你希望将 wisget 居中，你也可以使用 Center widget。

&emsp;再次调用 runApp 将会将之前的根 widget 从视图中分离出来，并将给定的新 widget 放在原位。新的 widget tree 会与之前的 widget tree 进行比较，并将任何差异应用到底层 render tree 上，类似于在调用 State.setState 后重建 StatefulWidget 时发生的情况。

&emsp;如有必要，使用 WidgetsFlutterBinding 初始化绑定。

## PlatformDispatcher.implicitView

&emsp;如果平台无法创建窗口，引擎会提供一个名为 FlutterView 的视图，或者为了向后兼容性而提供。

&emsp;如果平台提供了 implicitView，则可以用它来引导 Flutter framework。这在为单视图应用程序设计的平台上很常见，比如只有一个显示屏的移动设备。

&emsp;应用程序和库不能依赖该属性已设置，因为它可能会取决于引擎的配置而为 null。相反，考虑使用 View.of 来查找当前 BuildContext 正在绘制的 FlutterView。

&emsp;虽然所引用的 FlutterView 的属性可能会改变，但引用本身保证在应用程序的整个生命周期中永远不会更改：如果该属性在启动时为 null，则在应用程序的整个生命周期中它将保持不变。如果它指向特定的 FlutterView，则它将继续指向同一个视图，直到应用程序关闭（尽管引擎可以自行替换或删除视图的底层支持表面）。

## Application shutdown

&emsp;当应用程序关闭时，这个 widget tree 不会被销毁，因为没有办法准确预测何时会发生关闭。例如，用户可能会从设备上物理移除电源，或应用程序可能会意外崩溃，或设备上的恶意软件可能会强制终止进程。

&emsp;应用程序有责任确保它们在面对快速的不可预期终止时也能表现良好。

&emsp;要监听平台（iOS/android）关闭消息（以及其他生命周期变化），可以考虑使用 AppLifecycleListener API。

&emsp;要人为地导致整个 widget tree 被销毁，可以考虑调用 runApp，并使用诸如 SizedBox.shrink 之类的小部件。

## Dismissing Flutter UI via platform native methods

&emsp;一个应用程序可能同时包含 Flutter 和非 Flutter UI。如果应用程序调用非 Flutter 方法来删除基于 Flutter 的 UI，例如使用平台原生 API 来操作平台原生导航栈，那么 Flutter framework 不知道开发者是否有意急切地释放资源。widget tree 仍然保持挂载状态，并且一旦再次显示，就可以立即渲染。

&emsp;为了更及时地释放资源，建立一个平台通道（platform channel），并使用它来调用 runApp，传入一个诸如 SizedBox.shrink 这样的 widget，当框架应该销毁活跃的 widget tree 时。（SizedBox.shrink：直接指定宽度和高度为 0）

```dart
  // 创建一个 box，它会尽可能变得和其父级容器一样小。
  const SizedBox.shrink({ super.key, super.child })
    : width = 0.0,
      height = 0.0;
```

# RendererBinding

&emsp;RendererBinding 是连接 render tree 和 Flutter engine 之间的粘合剂。

&emsp;RendererBinding 管理多个独立的 render tree（有一个 `Iterable<RenderView> get renderViews => _viewIdToRenderView.values;` Getter。）。每个 render tree 的根是一个 RenderView，必须通过 addRenderView 将其添加到 RendererBinding 中，以便在帧生成、点击测试等过程中考虑到它们。此外，render tree 必须由一个 PipelineOwner 管理，该 PipelineOwner 是 pipeline owner tree 的一部分，其根是 rootPipelineOwner。

&emsp;以上述方式向此 RendererBinding 添加 PipelineOwners 和 RenderViews 的工作是一个更高级抽象的责任。举例来说，Widgets 库引入了 View widget，它会注册其 RenderView 和 PipelineOwner 到此 RendererBinding 中。






## 参考链接
**参考链接:🔗**
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [14.2 Element、BuildContext和RenderObject](https://book.flutterchina.club/chapter14/element_buildcontext.html#_14-2-1-element)
+ [Flutter启动流程和渲染管线](https://book.flutterchina.club/chapter14/flutter_app_startup.html#_14-3-1-应用启动)
+ [FlutterView class](https://api.flutter.dev/flutter/dart-ui/FlutterView-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
