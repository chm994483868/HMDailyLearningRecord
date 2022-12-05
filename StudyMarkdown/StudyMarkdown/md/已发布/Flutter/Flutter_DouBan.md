# Flutter 不容错过的项目实践

&emsp;首先感谢原作者的无私奉献：[kaina404/FlutterDouBan](https://github.com/kaina404/FlutterDouBan)，为我学习 Flutter 开发技术的路上亮起了明灯！

&emsp;[FlutterPractise](https://github.com/chm994483868/FlutterPractise) 这里是一个根据作者原始项目整理的一个 Flutter 项目小实践，方便 Flutter 初学者增强自己的 Flutter 开发技术熟练度，加深对 Flutter 中各种 Widget 的了解和使用。

&emsp;由于豆瓣的限制，项目中的所有接口已经无法正常请求数据，即使原始项目中做了一个开关读取本地 JSON 数据，但是并不彻底，所以导致很多作者精心编写的 UI 页面无法展示出来，所以我跟着作者的源代码一步一步梳理了原始项目中的每一个页面，使每个页面都读取到本地的 JSON 数据，然后解析数据使页面正常展示出来，这样方便大家观看页面效果进行学习。

&emsp;再次感谢原作者的无私奉献，下面我会以一个 Flutter 初学者的视角顺着 App 的各个页面梳理整个项目，并且对相关功能和页面牵涉到的 Flutter 开发基础点进行展开学习。

## 一、如何在 Flutter 中控制 Widget 的显示与隐藏：Offstage 的使用

<figure class="half">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/98bceac0382f47e9a88b9af5111d7ecf~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/d780ed0172f1484d845022f9da474a78~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;App 启动首先映入我们眼帘的是 5 秒倒计时的启动屏，倒计时结束后呈现出 App 的首页。原始项目中作者用了两个 Offstage 分别显示启动屏页面和 App 内容页面，这里其实可以牵涉出一个 App 开发中比较常见的场景，那就是我们控制显示和隐藏一个页面的方式，在 iOS 中我们比较常用的是 `hidden` 或 `alpha` 属性来控制 `UIView` 的显示与隐藏，或者调整视图的约束或 frame，让它离开当前位置或 size 为 0。不过此时即使是隐藏状态的 UIView 也并没有从其父视图中移除，因为下一秒我们可能就需要把它显示出来了。那这里和启动屏的场景并不匹配，我们都知道启动屏在 App 的本次生命周期中仅显示一次，启动过后我们就可以把它销毁了。所以这里我对启动屏和 App 内容页面进行了改造，封装启动屏在倒计时结束后回调显示 App 内容页面。这里我添加了两个文件：root_widget.dart 和 splash_new_widget.dart，其中 RootWidget 根据一个 `_showAD` 标识判断显示 `SplashNewWidget`，当 5 秒倒计时结束回调中，在 setState 中修改 `_showAD` 的值，把 `ContainerPage` 显示出来。

```dart
import 'package:flutter/material.dart';
import 'package:flutter_official_project/pages/container_page.dart';
import 'package:flutter_official_project/pages/splash/splash_new_widget.dart';

class RootWidget extends StatefulWidget {
  const RootWidget({super.key});

  @override
  State<RootWidget> createState() => _RootWidgetState();
}

class _RootWidgetState extends State<RootWidget> {
  bool _showAD = true;
  var container = const ContainerPage();

  @override
  Widget build(BuildContext context) {
    return _showAD ? SplashNewWidget(onCountDownFinishCallBack: (bool value) {
    if (value) {
      setState(() {
        _showAD = false;
      });
    }
  }) : container;
  }
}
```

&emsp;下面我们拓展一下 Offstage 和 Visibility 相关的知识点。

&emsp;我们可以先看一下这篇文章来了解: [flutter学习之widget的显示和隐藏](https://blog.csdn.net/yong_19930826/article/details/126018613)。

&emsp;这里是 Offstage 的官方文档：[Offstage class](https://api.flutter.dev/flutter/widgets/Offstage-class.html#widgets.Offstage.1)，对比 iOS 中 UIView 的 hidden 属性，Offstage 的最大区别在于当 Offstage 的 offstage 属性值为 true 时，它的 child 是不显示的，且它不再占用 parent Widget 的任何空间，就像它完全不存在一样，但是 child 依然是保持 active 的依然维持着 child 的 state 的（依然占用着内存和资源），它能 receive focus 或者键盘输入。虽然它在屏幕上消失了我们不能点击到它，但是例如它包含一个输入框并且在消失前已经获得焦点，那么我们依然可以点击键盘向其中输入内容。

&emsp;学习 Offstage 时我们其实可以对比 [Visibility](https://api.flutter.dev/flutter/widgets/Visibility-class.html) 来学习，`Visibility` 能对隐藏态的 `child` 做更细致的控制，它有如下属性，我们简单过一下：

+ `final Widget child;`: 要显示或隐藏的 widget，由 `visible` 控制。
+ `final bool visible;`: 在显示 `child` 或隐藏它之间切换。无论 `visible` 属性的状态如何，`maintain` flags 都应设置为相同的值，否则它们将无法正常运行（具体而言，每当任何 `maintain` flags 发生更改时，无论 `maintainState` 的状态如何，`state` 都将丢失，因为这样做将导致 subtree shape 更改）。除非设置了 `maintainState`，否则 `child` 将在隐藏时被释放（从树中删除）。这一段的信息有点多，我们需要理解一下，首先 Visibility 有一组 bool 类型的 maintain flags 的属性：`maintainState`（维持状态）、`maintainAnimation`（维持动画）、`maintainSize`（维持大小）、`maintainSemantics`（维持语义）、`maintainInteractivity`（维持交互），它们的值应该统一。另外，如果 `maintainState` 的值为 `false`，那么当 `visible` 变化为 `true` 时，`child` 会隐藏，此时不单单是隐藏，`child` 会被释放，即它的 dispose 函数会被调用，child 的所有状态也都会丢失。
+ `final bool maintainState;`: 是否在 `child` 子树不可见时维持其 `State` 对象。保持 child 的 state 可能代价高昂（因为这意味着所有对象仍在内存中，它们的资源不会释放）。仅当无法按需重新创建它时，才应维护它。何时维护 state 的一个示例是 child 是否包含 Navigator，因为该 widget 维护无法动态重新创建的复杂 state。如果此属性为 false，则 maintainAnimation 也必须为 false。动态更改此值可能会导致 child 的当前状态丢失（如果 visible 为 true，则会立即创建具有新 State 对象的子树的新实例）。如果此属性为 true，则使用 Offstage widget 隐藏 child，而不是将其替换为 replacement。
+ `final Widget replacement;`: 当 `child` 不可见时使用的 widget，假设未设置任何 maintain flags（特别是 maintainState，即它们的值都为 false）。此属性的默认值行为是将 widget 替换为 zero box（`this.replacement = const SizedBox.shrink()`）。
+ `final bool maintainAnimation;`: 是否在 child 不可见时维持动画。要设置此项，还必须设置 maintainState。当 child 不可见时保持动画处于活动状态比仅维护状态更昂贵。这可能有用的一个示例是，如果子树使用 AnimationController 及时对其布局进行动画处理，并且该布局的结果用于影响其他一些逻辑。如果此标志为 false，则当可见标志为 false 时，child 中托管的任何 AnimationController 都将 muted。如果此属性为 true，则不使用 TickerMode widget。如果此属性为 false，则 maintainSize 也必须为 false。动态更改此值可能会导致 child 的当前状态丢失（如果 visible 为 true，则会立即创建具有新 State 对象的 child 的新实例）。
+ `final bool maintainSize;`: 当 child 不可见时是否为 child 本来存在的位置保留空间。要设置此项，还必须设置 maintainAnimation 和 maintainState。在 child 不可见时保持大小并不比仅保持动画运行而不保持大小昂贵得多，并且在某些情况下，如果子树很简单并且经常切换可见属性，则在某些情况下可能会稍微便宜一些，因为它避免在切换可见属性时触发布局更改。如果子树不是微不足道的，那么甚至不保留状态会便宜得多。如果此属性为 true，则可使用 Opacity widget 代替而不是 Offstage widget。（Opacity 的 child 隐藏时仍然保留 child 所占据的布局空间）如果此属性为 false，则 maintainSemantics 和 maintainInteractivity 也必须为 false。动态更改此值可能会导致 child 的当前状态丢失（如果可见为 true，则会立即创建具有新 State 对象的 child 的新实例）。
+ `final bool maintainSemantics;`: 是否在隐藏 child 时维护其语义（例如，为了 accessibility）。要设置此值，还必须设置 maintainSize。默认情况下，将 maintainSemantics 设置为 false 时，当 child 对用户隐藏时，accessibility tools 不可见。如果此标志设置为 true，则 accessibility tools 将报告 widget，就好像它存在一样。动态更改此值可能会导致 child 的当前状态丢失（如果可见为 true，则会立即创建具有新 State 对象的 child 的新实例）。
+ `final bool maintainInteractivity;`: 是否允许 child 在隐藏时具有交互性。要设置此设置，还必须设置 maintainSize。默认情况下，将 maintainInteractivity 设置为 false 时，触摸事件在对用户隐藏时无法到达子项。如果此标志设置为 true，则仍会传递触摸事件。动态更改此值可能会导致 child 的当前状态丢失（如果可见为 true，则会立即创建具有新 State 对象的 child 的新实例）。

&emsp;至此 Visibility 的属性就看完了，因为它牵涉的内容比较多关于 Widget 的隐藏与显示时的布局空间占据、交互的响应、动画的维持、State 的维持、热重载时 child 的重建等等，所以我们花的时间有点多。 

&emsp;我们简单过一下 ContainerPage 中的内容，我们可以把它对比为 iOS 中的我们自己封装的 TabBarController，在里面组织屏幕底部的一排 TabBarItem 以及选中其中某个时显示它们对应的页面，这里使用了 5 个 Offstage，一个 TabBarItem 对应一个 Offstage，当选中某个 TabBarItem 时就使指定的 Offstage 显示它的 child。使用 Offstage 保证当 child 隐藏时 state 还能维持，widget 也不会被销毁。

&emsp;下面我们进入 HomePage 的学习，进入之前呢我们要补充一下 Flutter 中滚动视图相关的知识点，它太重要了！

## 二、如何在 Flutter 中构建滚动视图：NestedScrollView 的使用

&emsp;HomePage 是一个非常经典的 App 页面，顶部是一个搜索框，下面是一个滚动列表，包括两种样式的 cell，一种是并排三张图片，一种是一个视频播放器。关于这个滑动列表我们用到了一个比较复杂的 Widget: NestedScrollView。

&emsp;[NestedScrollView class](https://api.flutter.dev/flutter/widgets/NestedScrollView-class.html#widgets.NestedScrollView.1) 官方文档。

&emsp;NestedScrollView 是一个滚动视图，其中可以嵌套其他滚动视图，其滚动位置本质上是具有 intrinsically linked（内在联系）。

&emsp;此 widget 最常见的用例是具有灵活的 SliverAppBar 的可滚动视图，该视图在 header 中包含 TabBar（由 headerSliverBuilder 构建，并在 body 中具有 TabBarView，因此可滚动视图的内容会根据可见的 Tab 而有所不同。（即下面第一个示例中的滚动页面）

&emsp;在通常的 ScrollView 中，有一组 slivers（滚动视图的 components）。如果其中一个 sliver 托管了一个以相反方向滚动的 TabBarView（例如，允许用户在 tabs 表示的页面之间水平滑动切换，而列表垂直滑动），则该 TabBarView 内的任何列表都不会与外部 ScrollView 交互。例如，滑动内部列表滚动到顶部后并不会导致外部 ScrollView 中折叠的 SliverAppBar 展开。（这里的意思即比如我们在 iOS 中屏幕上下半部分各放一个可以上下滑动的 UIScrollView，当下面的 UIScrollView 滑动到顶部时它本身会有一个果冻效果，而不是促使上半部分的 UIScrollView 跟着一起向下滑动，而 Flutter 可以通过一些方法，让两个滑动视图进行联动）

&emsp;NestedScrollView 通过为外部 ScrollView 和内部 ScrollViews（TabBarView 内部的那些，将它们 hooking 在一起，以便它们向用户显示为一个连贯的滚动视图）提供自定义 ScrollController 来解决此问题。

&emsp;下面我们看一个实例：此示例显示一个 NestedScrollView，其 header 是 SliverAppBar 中的 TabBar 的组合，其 body 是 TabBarView。它使用 SliverOverlapAbsorber/SliverOverlapInjector 对，使内部列表正确对齐，并使用 SafeArea 来避免任何水平干扰（例如，iOS 顶部的刘海底部的下巴安全区。我们使用 SafeArea widget 并给它的 bottom 属性置为 true，那么内部的滚动列表就会把屏幕底部的安全区留白，无法延伸到屏幕底部，置为 false 便可以延伸到屏幕最底部，这个大概就是 iOS 开发中的滚动列表适配手机下巴，想必 iOS 开发者是比较熟悉的。）。此外，PageStorageKeys 用于记住每个 tab's list 的滚动位置。

&emsp;这里是运行截图，这个大概在 android 开发或者很多 iOS App 都特别常见到的页面布局。在 Flutter 大概是这样的：首先顶部的 TabBar 包含一组水平排布的 Tab 标签，它们可以横向左右滑动，然后是每个 Tab 标签和下面的 TabBarView 的一个 child 对应绑定的，TabBar 中 Tab 的个数与 TabBarView 的 children 的个数完全一致且一一对应，我们点击哪个 Tab 标签就左右切换到 TabBarView 的哪个 child，然后我们也可以在屏幕的下半部分左右滑动切换 TabBarView 的 child，然后顶部的 Tab 标签也会跟着一起左右滑动切换。（这一套布局在 iOS 下的话要开发者完全自己手动开发，上下搭配自己做滑动回调处理，iOS 大概只提供了转头，需要开发者自己手动砌墙...）

<figure class="half">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/13646f9c2209491395dedc2db96e504c~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/54c892aac4be41ac8faa5ae6227ae5a0~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;下面是代码实现，初看我们可能有点懵，但是就这 60 行代码便实现了一个在 iOS 原生开发中比较 “复杂” 的滚动视图，开发效率还是比较高的。这段代码我们大概需要 “全文背诵”，这个是 Flutter 中最基本的滚动列表实现。

```dart
class MyStatelessWidget extends StatelessWidget {
  const MyStatelessWidget({super.key});

  @override
  Widget build(BuildContext context) {
  
    // 顶部 Tab 的标题字符串
    final List<String> tabs = <String>[
      'Tab 1',
      'Tab 2',
      'Tab 3',
      'Tab 4',
      'Tab 5',
      'Tab 6',
      'Tab 7',
      'Tab 8',
      'Tab 9'
    ];
    
    // DefaultTabController 是一个 inherited widget，用于与 TabBar 或 TabBarView 共享 TabController。
    // 当共享显式创建的 TabController 不方便时使用它，因为 tab bar widgets 是由无状态 parent widgth 或不同的 parent widget 创建的。
    return DefaultTabController(
      // 通常大于 1，且必须与 [TabBar.tabs] 和 [TabBarView.children] 的长度相等。
      length: tabs.length,
      child: Scaffold(
        // Scaffold body，这里就是一个完整的 Neste dScrollView 嵌套滚动视图 
        body: NestedScrollView(
          // 用于构建 NestedScrollView 的 header 的 builder。通常，这用于创建带有 [TabBar] 的 [SliverAppBar]。
          headerSliverBuilder: (BuildContext context, bool innerBoxIsScrolled) {
            debugPrint("🐶🐶🐶 DefaultTabController NestedScrollView 中的：$context");

            return <Widget>[
              // SliverOverlapAbsorber 与 body 中的 SliverOverlapInjector 对应
              SliverOverlapAbsorber(
                handle: NestedScrollView.sliverOverlapAbsorberHandleFor(context), // 记录 absorbed overlap（吸收重叠）的对象
                // 在 [CustomScrollView] 中使用的 app bar
                sliver: SliverAppBar(
                  title: const Text('Books'),
                  // 是否把这个 app bar 固定在 scrollview 的顶部，否则 scrollview 向上滑动时这个 app bar 跟着向上滑动
                  pinned: true,
                  expandedHeight: 150.0,
                  forceElevated: innerBoxIsScrolled,
                  // 底部便是一个 TabBar 内部是一组 Tab
                  bottom: TabBar(
                    isScrollable: true,
                    tabs: tabs.map((String name) => Tab(text: name)).toList(),
                  ),
                ),
              ),
            ];
          },
          // NestedScrollView 的 body：这里是一个 TabBarView，然后它的每一个 child 是一个上下滚动的列表
          body: TabBarView(
            // TabBarView 的 children 便是一组与上面 Tab 一一对应的一组 Widget
            children: tabs.map((String name) {
              // SafeArea 安全最大化的使用当前一些异形屏的手机屏幕空间
              return SafeArea(
                top: false,
                bottom: false,
                // 为了使用 context，在整个 NestedScrollView 中需要上下滚动视图联动，所以使用 Builder 来构建 Widget，
                // 这个有点类似 SwiftUI 中的 ViewBuilder，所以我们可以对比学习
                child: Builder(
                  builder: (BuildContext context) {
                    debugPrint("🐶🐶🐶 TabBarView SafeArea 中的：$context");

                    // 即 TabBarView 的每个 child 是一个 CustomScrollView
                    return CustomScrollView(
                      key: PageStorageKey<String>(name),
                      slivers: <Widget>[
                        SliverOverlapInjector(
                          handle: NestedScrollView.sliverOverlapAbsorberHandleFor(context),
                        ),
                        // SliverPadding 仅仅是为了添加 padding，当不需要添加 padding 时也可直接使用 SliverFixedExtentList  
                        SliverPadding(
                          padding: const EdgeInsets.all(0.0),
                          sliver: SliverFixedExtentList(
                            itemExtent: 48.0,
                            delegate: SliverChildBuilderDelegate(
                              (BuildContext context, int index) {
                                return Container(
                                  color: Color.fromARGB(255, 174, 168, 174),
                                  child: ListTile(
                                    title: Text('Item $index'),
                                  ),
                                );
                              },
                              childCount: 30,
                            ),
                          ),
                        ),
                      ],
                    );
                  },
                ),
              );
            }).toList(),
          ),
        ),
      ),
    );
  }
}
```

&emsp;NestedScrollView 就暂时看到这里，由于不是本文中心，就不过多关注了，当然它真的贼重要，里面的细节每一个 Flutter 开发者都应该彻底掌握。

&emsp;看完 NestedScrollView 的相关内容，那么我们看 HomePage 中的布局就极其简单了，整体框架和上面的示例代码完全相同，不同之处仅是滚动列表我们使用了自定义的列表项，顶部用了自己封装的 SearchTextFieldWidget widget 搜索框。然后我们下面把目光聚集在 HomePage 列表中的视频播放上和搜索页面的跳转上。

## 三、如何在 Flutter 中播放视频：video_player 的使用

&emsp;在 HomePage 中我们看到有一个视频播放的 cell，这里使用了 [video_player](https://pub.dev/packages/video_player) 进行视频播放。

<figure class="half">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7f2555e7fd454e32b768256590e1dfbb~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/49ff222b559840c2ad16a35b360bce03~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;video_player 使用起来也超简单：

```dart
import 'package:flutter/material.dart';
import 'package:video_player/video_player.dart';

void main() => runApp(const VideoApp());

/// Stateful widget to fetch and then display video content.
class VideoApp extends StatefulWidget {
  const VideoApp({Key? key}) : super(key: key);

  @override
  _VideoAppState createState() => _VideoAppState();
}

class _VideoAppState extends State<VideoApp> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network('https://flutter.github.io/assets-for-api-docs/assets/videos/bee.mp4')
      ..initialize().then((_) {
        // Ensure the first frame is shown after the video is initialized, even before the play button has been pressed.
        setState(() {});
      });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Video Demo',
      home: Scaffold(
        body: Center(
          child: _controller.value.isInitialized
              ? AspectRatio(
                  aspectRatio: _controller.value.aspectRatio,
                  child: VideoPlayer(_controller),
                )
              : Container(),
        ),
        floatingActionButton: FloatingActionButton(
          onPressed: () {
            setState(() {
              _controller.value.isPlaying
                  ? _controller.pause()
                  : _controller.play();
            });
          },
          child: Icon(
            _controller.value.isPlaying ? Icons.pause : Icons.play_arrow,
          ),
        ),
      ),
    );
  }

  @override
  void dispose() {
    super.dispose();
    _controller.dispose();
  }
}
```

&emsp;在原始项目中这里有一个问题，就是当视频播放 widget 滑出屏幕或者页面发生跳转，比如点击顶部搜索框跳转到搜索页面后，我们需要暂停视频的播放，这里我们使用了另外一个 package：[visibility_detector](https://pub.flutter-io.cn/packages/visibility_detector) 当视频播放的 widget 不可见时我们执行视频的暂停播放。 visibility_detector 是一个特别有用的 package，例如我们也能检测滑动视图中的 widget 滑出屏幕的时机。

```dart
...
    final List<Widget> children = <Widget>[
      GestureDetector(
        // 这里我们使用 VisibilityDetector widget，把原始的 VideoPlayer(_controller) 作为其 child 使用
        // child: VideoPlayer(_controller),
        child: VisibilityDetector(
            key: Key(widget.url),
            child: VideoPlayer(_controller),
            onVisibilityChanged: (visibilityInfo) {
              var visiblePercentage = visibilityInfo.visibleFraction * 100;
              debugPrint('🥥🥥🥥 VisibilityDetector 打印: Widget ${visibilityInfo.key} is $visiblePercentage% visible');

              if (visiblePercentage < 100 && _controller.value.isPlaying) {
                _controller.pause();
              }
            }),
        onTap: () {
          // 点击隐藏或者显示视频暂停播放按钮以及视频进度条
          setState(() {
            _showSeekBar = !_showSeekBar;
          });
        },
      ),
      // 视频播放控制按钮等 Widget
      getPlayController(),
    ];
...
```

&emsp;我们看一下 visibility_detector 的简介：

&emsp;VisibilityDetector widget 包装现有的 Flutter widget，并在 widget 的可见性改变时触发回调。（它实际上报告 VisibilityDetector 本身的可见性何时更改，并且其可见性应与其 child 的可见性相同。

&emsp;回调不会在可见性更改时立即触发。相反，回调被延迟和合并，以便每个 VisibilityDetector 的回调将在每个 VisibilityDetectorController.updateInterval 调用时调用一次（除非通过 VisibilityDetectorController.notifyNow() 强制调用）。所有 VisibilityDetector widget 的回调在帧之间同步触发。VisibilityDetectorController.notifyNow() 可用于强制触发挂起的可见性回调，例如切换视图或退出应用程序时，这可能是可取的。

&emsp;visibility_detector 的一些限制:

+ VisibilityDetector 仅考虑 widget 的边界框。它不考虑 widget 的 opacity。
+ 可见性回调中的 visibleFraction（可见分数）可能无法解释重叠的 widget，这些 widget 会掩盖 VisbilityDetector。

## 四、如何在 Flutter 中控制页面跳转：Navigator 的使用

&emsp;在 HomePage 中点击顶部的搜索框，页面发生了跳转，在点击事件中执行了：

```dart
MyRouter.push(context, MyRouter.searchPage, '我是传递到搜索页面中的参数');
```

```dart
class MyRouter {
...
  MyRouter.push(BuildContext context, String url, dynamic params) {
    Navigator.push(context, MaterialPageRoute(builder: (context) {
      // 这里根据 params 构建一个指定的页面 widget
      return _getPage(url, params);
    }));
  }
}
```

&emsp;MyRouter 是定义的一个便于做页面跳转的 class，它的内部很简单，主要根据传入的跳转 url 然后跳转到指定的页面。其中最重要的 push 函数，用到了 Navigator，它是 Flutter 中做页面跳转的核心 widget。Navigator 使用 stack 规则管理一组 child widget 的 widget。许多应用程序在其 widget 层次结构的顶部附近都有一个 navigator，以便使用 Overlay 显示其逻辑历史记录，其中最近访问的页面直观地位于旧页面的顶部。使用此模式，navigator 可以通过在 overlay 中移动 widget 来直观地从一个页面过渡到另一个页面。同样，navigator 可用于通过将 dialog widget 定位在当前页面上方来显示 dialog。

&emsp;然后是 Navigatior 的 push 函数，将给定 route（路由）push 到最紧密地包围给定 context 的 navigator 上（Navigator.of(context)）。新 route 和以前的 route（如果有）会收到通知（Route.didPush 和 Route.didChangeNext）。如果 Navigator 有任何 Navigator.observers，他们也会收到通知（NavigatorObserver.didPush）。push 新 route 时，当前 route 中的 ongoing gestures（持续手势）将被取消。T 类型参数是 route 返回值的类型。返回一个 Future，该 Future 完成到从 navigator 弹出 push 的 route 时传递给 pop 的结果值。

```dart
...
  @optionalTypeArgs
  static Future<T?> push<T extends Object?>(BuildContext context, Route<T> route) {
    return Navigator.of(context).push(route);
  }
...
```

&emsp;push 的典型用法：

```dart
void _openMyPage() {
  Navigator.push<void>(
    context,
    MaterialPageRoute<void>(
      builder: (BuildContext context) => const MyPage(),
    ),
  );
}
```

&emsp;然后下面是一系列的：书影音、小组中的嵌套滚动视图布局，和首页滚动视图的使用类似，这里便不再深入。

<figure class="half">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/fd11f162b3564e8eaf8587137eed503d~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/a574a92ca33645f599db2538b133c415~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b7313235be234306b9e9c0d8f712d997~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/94fdc70da2a64844aa68ea5e5900bc2f~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/9769fc67f93d4d03b2b9400afd85ab98~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/fb17ca78eab444f5bb20eea95a9c6597~tplv-k3u1fbpfcp-watermark.image?">
</figure>

## 五、如何在 Flutter 中加载网页：flutter_webview_plugin 的使用

&emsp;在 ShopPageWidget 中展示的是网页的加载，这里使用一个 package：[flutter_webview_plugin](https://pub.dev/packages/flutter_webview_plugin)。

<figure class="half">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3c408a32679040b48de53668eefbefa0~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3bbb5a4057c340cb8ec33d84def66061~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;对应的网页加载代码：

```dart
class WebViewPage extends StatelessWidget {
  final String url;
  final dynamic params;
  // ignore: constant_identifier_names
  static const String TITLE = 'title';

  const WebViewPage(this.url, this.params, {super.key});

  @override
  Widget build(BuildContext context) {
    return WebviewScaffold(
      url: url,
      appBar: AppBar(
        title: Text(params[TITLE]),
        backgroundColor: Colors.green,
      ),
    );
  }
}
```

&emsp;或者使用 FlutterWebviewPlugin 用于变换加载不同网页的 URL。

```dart
// 提供链接到一个唯一 WebView 的单例实例，以便你可以从应用程序的任何位置控制 webview
final _webviewReference = FlutterWebviewPlugin();

...
  @override
  Widget build(BuildContext context) {
    debugPrint('build widget.url=${widget.url}');

    return _WebviewPlaceholder(
      onRectChanged: (value) {
        if (_rect == null || _closed) {
          if (_rect != value) {
            _rect = value;
          }

          RenderBox? renderBox = context.findRenderObject() as RenderBox;
          double left = 0;
          double top = renderBox.localToGlobal(Offset.zero).dy;
          double width = renderBox.size.width;

          // 这里 34 是针对苹果刘海屏系列，底部安全区高度是 34
          double height = ScreenUtils.screenH(context) - top - kBottomNavigationBarHeight - 34;

          MediaQueryData mq = MediaQuery.of(context);
          double safeBottom = mq.padding.bottom;

          debugPrint('🌍🌍🌍 _webviewReference.launch ${renderBox.size} ${renderBox.localToGlobal(Offset.zero)} ${ScreenUtils.screenH(context)} $kBottomNavigationBarHeight $height $safeBottom');

          Rect rect = Rect.fromLTWH(left, top, width, height);
          _webviewReference.launch(widget.url, withJavascript: true, withLocalStorage: true, scrollBar: true, rect: rect);
        } else {
          if (_rect != value) {
            _rect = value;
          }
          _webviewReference.reloadUrl(widget.url);
        }
      },
      child: const Center(
        // 环形菊花加载器
        child: CircularProgressIndicator(),
      ),
    );
  }
...
```

## 六、如何在 Flutter 中不同的页面切换状态栏主题：SystemUiOverlayStyle 的使用

&emsp;在书影音、小组的列表中点击任何一个电影都会跳转到一个电影详情页面 DetailPage，这里的电影详情数据都使用了本地的 json 数据。

&emsp;这里我们注意到在前一个页面是黑色的状态栏，然后在 DetailPage 页面使用了白色的状态栏，这里过程也比较简单，首先我们用一个 `_lastStyle` 变量在 `initState` 中记录上一个页面的状态栏颜色，然后我们便可以随意调用 `SystemChrome.setSystemUIOverlayStyle(SystemUiOverlayStyle.light);` 设置当前页面的状态栏颜色。然后我们在 `dispose` 中调用：`SystemChrome.setSystemUIOverlayStyle(_lastStyle!);` 恢复前一个页面的状态栏颜色。

```dart
class _DetailPageState extends State<DetailPage> {
...
  // 记录上一个页面的状态栏颜色
  SystemUiOverlayStyle? _lastStyle;

  @override
  void initState() {
    // 获取上一个页面的状态栏颜色
    _lastStyle = SystemChrome.latestStyle;
    // 设置当前页面状态栏颜色为白色
    SystemChrome.setSystemUIOverlayStyle(SystemUiOverlayStyle.light);

    super.initState();
  }

  @override
  void dispose() {
    // 将状态栏设置为之前的颜色
    if (_lastStyle != null) {
      SystemChrome.setSystemUIOverlayStyle(_lastStyle!);
    }

    super.dispose();
  }
```

&emsp;然后是 DetailPage 中的其他布局，在仓库代码中都有详细注释，这里就不在展开了。

<figure class="half">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/bf4374767dbf489bb23675a0175d9999~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4cbd6be8b1b24ac08198039b275657be~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/a1c7fb698d694d77906c8a413441bd19~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/8c709d44099946c8a37245b34c0a3172~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f08c595757924c1e84643143bf5d248c~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/a1142297ccec4053bb8190218741a710~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/2c7f051a50da4acfb8ce72d8d8cb1b17~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/79359adf4b4f47419622947c1eba1e37~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;作为 Flutter 学习的一个阶段性学习成果就先到这里了，仓库有完整可直接运行的项目供大家参考。

## 参考链接
**参考链接:🔗**
+ [kaina404/FlutterDouBan](https://github.com/kaina404/FlutterDouBan)
+ [chm994483868/FlutterPractise](https://github.com/chm994483868/FlutterPractise)
