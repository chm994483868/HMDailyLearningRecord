# BuildOwner

&emsp;BuildOwner 是 Widgets 框架的 Manager 类。

&emsp;这个类跟踪哪些 Widget 需要重建，并处理适用于整个小部件树的其他任务，比如管理树的非活动 element 列表，并在调试时需要时触发 "reassemble" 命令进行热重载。

&emsp;主要的构建拥有者通常由 WidgetsBinding 拥有，并且与操作系统一起驱动构建/布局/绘制流水线的其余部分。

&emsp;可以构建额外的构建拥有者来管理屏幕外小部件树。

&emsp;要将构建拥有者分配给树，可以在小部件树的根元素上使用 RootElementMixin.assignOwner 方法。

&emsp;这个示例展示了如何构建一个用于测量渲染树布局大小的离屏 widget 树。对于一些用例，简单的 Offstage widget 可能是一个比这种方法更好的选择。要使用此代码示例创建一个本地项目，请运行: flutter create --sample=widgets.BuildOwner.1 mysample

```dart
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

/// Flutter code sample for [BuildOwner].

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  final Size size = measureWidget(const SizedBox(width: 640, height: 480));

  // Just displays the size calculated above.
  runApp(
    WidgetsApp(
      title: 'BuildOwner Sample',
      color: const Color(0xff000000),
      builder: (BuildContext context, Widget? child) {
        return Scaffold(
          body: Center(
            child: Text(size.toString()),
          ),
        );
      },
    ),
  );
}

Size measureWidget(Widget widget) {
  final PipelineOwner pipelineOwner = PipelineOwner();
  final MeasurementView rootView = pipelineOwner.rootNode = MeasurementView();
  final BuildOwner buildOwner = BuildOwner(focusManager: FocusManager());
  final RenderObjectToWidgetElement<RenderBox> element =
      RenderObjectToWidgetAdapter<RenderBox>(
    container: rootView,
    debugShortDescription: '[root]',
    child: widget,
  ).attachToRenderTree(buildOwner);
  try {
    rootView.scheduleInitialLayout();
    pipelineOwner.flushLayout();
    return rootView.size;
  } finally {
    // Clean up.
    element.update(RenderObjectToWidgetAdapter<RenderBox>(container: rootView));
    buildOwner.finalizeTree();
  }
}

class MeasurementView extends RenderBox
    with RenderObjectWithChildMixin<RenderBox> {
  @override
  void performLayout() {
    assert(child != null);
    child!.layout(const BoxConstraints(), parentUsesSize: true);
    size = child!.size;
  }

  @override
  void debugAssertDoesMeetConstraints() => true;
}
```


## Constructors

&emsp;创建一个管理 widget 的对象。如果未指定 focusManager 参数，或者该参数为 null，这将会构建一个新的 FocusManager，并通过 FocusManager.registerGlobalHandlers 注册其全局输入处理程序，这将修改静态状态。如果调用者想要避免改变这种状态，可以在这里显式传递一个焦点管理器。

```dart
class BuildOwner {
  BuildOwner({ this.onBuildScheduled, FocusManager? focusManager }) :
      focusManager = focusManager ?? (FocusManager()..registerGlobalHandlers());
  //...
}
```

## onBuildScheduled

&emsp;在每次构建过程中，当第一个可构建元素被标记为脏时调用。

```dart
  VoidCallback? onBuildScheduled;
```

## `_inactiveElements`

&emsp;存储、管理 inactive Element 对象。

```dart
  final _InactiveElements _inactiveElements = _InactiveElements();
```

## focusManager

&emsp;负责焦点树的对象。

&emsp;很少直接使用。相反，考虑使用 FocusScope.of 来获取给定 BuildContext 的 FocusScopeNode。

&emsp;有关更多详细信息，请参见 FocusManager。

&emsp;该字段将默认为通过 FocusManager.registerGlobalHandlers 注册其全局输入处理程序的 FocusManager。希望避免注册这些处理程序（并修改关联的静态状态）的调用方可以明确地将焦点管理器传递给 BuildOwner.new 构造函数。

```dart
  FocusManager focusManager;
```

## scheduleBuildFor

&emsp;将一个元素添加到脏元素列表中，这样当 WidgetsBinding.drawFrame 调用 buildScope 时，它将被重建。

```dart
  void scheduleBuildFor(Element element) {
    assert(element.owner == this);
    assert(element._parentBuildScope != null);
    
    assert(() {
      if (debugPrintScheduleBuildForStacks) {
        debugPrintStack(label: 'scheduleBuildFor() called for $element${element.buildScope._dirtyElements.contains(element) ? " (ALREADY IN LIST)" : ""}');
      }
      if (!element.dirty) {
        throw FlutterError.fromParts(<DiagnosticsNode>[
          ErrorSummary('scheduleBuildFor() called for a widget that is not marked as dirty.'),
          element.describeElement('The method was called for the following element'),
          ErrorDescription(
            'This element is not current marked as dirty. Make sure to set the dirty flag before '
            'calling scheduleBuildFor().',
          ),
          ErrorHint(
            'If you did not attempt to call scheduleBuildFor() yourself, then this probably '
            'indicates a bug in the widgets framework. Please report it:\n'
            '  https://github.com/flutter/flutter/issues/new?template=2_bug.yml',
          ),
        ]);
      }
      return true;
    }());
    final BuildScope buildScope = element.buildScope;
    assert(() {
      if (debugPrintScheduleBuildForStacks && element._inDirtyList) {
        debugPrintStack(
          label: 'BuildOwner.scheduleBuildFor() called; '
                  '_dirtyElementsNeedsResorting was ${buildScope._dirtyElementsNeedsResorting} (now true); '
                  'The dirty list for the current build scope is: ${buildScope._dirtyElements}',
        );
      }
      // When reactivating an inactivate Element, _scheduleBuildFor should only be
      // called within _flushDirtyElements.
      if (!_debugBuilding && element._inDirtyList) {
        throw FlutterError.fromParts(<DiagnosticsNode>[
          ErrorSummary('BuildOwner.scheduleBuildFor() called inappropriately.'),
          ErrorHint(
            'The BuildOwner.scheduleBuildFor() method should only be called while the '
            'buildScope() method is actively rebuilding the widget tree.',
          ),
        ]);
      }
      return true;
    }());
    if (!_scheduledFlushDirtyElements && onBuildScheduled != null) {
      _scheduledFlushDirtyElements = true;
      onBuildScheduled!();
    }
    buildScope._scheduleBuildFor(element);
    assert(() {
      if (debugPrintScheduleBuildForStacks) {
        debugPrint("...the build scope's dirty list is now: $buildScope._dirtyElements");
      }
      return true;
    }());
  }
```

## lockState

&emsp;建立了一个范围，其中对 State.setState 的调用被禁止，并调用了给定的回调函数。这种机制用于确保，例如 State.dispose 不会调用 State.setState。

```dart
  void lockState(VoidCallback callback) {
    assert(_debugStateLockLevel >= 0);
    assert(() {
      _debugStateLockLevel += 1;
      return true;
    }());
    try {
      callback();
    } finally {
      assert(() {
        _debugStateLockLevel -= 1;
        return true;
      }());
    }
    assert(_debugStateLockLevel >= 0);
  }
```

## 







## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
