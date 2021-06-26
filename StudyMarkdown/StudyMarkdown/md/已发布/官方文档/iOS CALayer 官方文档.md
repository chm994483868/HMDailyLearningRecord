# iOS CALayer 官方文档

## CAMediaTiming
&emsp;CAMediaTiming 协议由图层（CALayer）和动画（CAAnimation）实现，它为分层计时系统（hierarchical timing system）建模，每个对象描述了从其父对象的时间值到本地时间的映射。

&emsp;绝对时间被定义为 mach time 转换成秒。为了方便查询当前绝对时间，提供了 CACurrentMediaTime 函数。

&emsp;从父时间到本地时间的转换分为两个阶段：
1. 转换为 “活动的本地时间”（active local time）。这包括对象在父时间轴中出现的时间点，以及它相对于父级播放的速度。
2. 从活动时间转换为 “基本本地时间”（basic local time）。时序模型允许对象重复其基本持续时间多次，并可以选择在重复播放之前进行播放。

```c++
@protocol CAMediaTiming
...
@end
```
### CACurrentMediaTime
&emsp;返回当前的绝对时间，以秒为单位。
```c++
CFTimeInterval CACurrentMediaTime(void);
```
&emsp;Return Value: 通过调用 mach_absolute_time() 并将结果转换为秒而得出的 CFTimeInterval。

### Animation Start Time（动画开始时间）
#### beginTime
&emsp;指定 receiver 相对于其父对象的开始时间（如果适用），预设为 0。
```c++
/* The begin time of the object, in relation to its parent object, if applicable. Defaults to 0. */
@property CFTimeInterval beginTime;
```
&emsp;对象的开始时间（相对于其父对象）（如果适用）。预设为 0。
#### timeOffset
&emsp;指定活动的本地时间中的附加时间偏移，预设为 0。
```c++
/* Additional offset in active local time. 
 * i.e. to convert from parent time tp to active local time t: t = (tp - begin) * speed + offset.
 * One use of this is to "pause" a layer by setting 'speed' to zero and 'offset' to a suitable value. 
 * Defaults to 0. 
 */
@property CFTimeInterval timeOffset;
```
&emsp;活动的本地时间增加的偏移量。例如；从父时间 tp 转换为活动的本地时间 t：t = (tp - begin) * speed + offset。一种用法是通过将 `speed` 设置为零并将 offset 设置为合适的值来暂停（"pause"）layer。预设为 0。

### Repeating Animations（重复动画）
#### repeatCount
&emsp;确定动画将重复的次数。
```c++
/* The repeat count of the object. May be fractional. Defaults to 0. */
@property float repeatCount;
```
&emsp;可能是分数（类型是 float）。如果 repeatCount 为 0，则将其忽略。预设值为 0。如果同时指定了 repeatDuration 和 repeatCount，则行为未定义。

#### repeatDuration
&emsp;确定动画将重复多少秒。（对象的重复持续时间。预设为 0。）
```c++
/* The repeat duration of the object. Defaults to 0. */
@property CFTimeInterval repeatDuration;
```
&emsp;预设值为 0。如果 repeatDuration 为 0，则将其忽略。如果同时指定了 repeatDuration 和 repeatCount，则行为是不确定的。

### Duration and Speed（持续时间和速度）
#### duration
&emsp;指定动画的基本持续时间（以秒为单位），默认为 0。
```c++
/* The basic duration of the object. Defaults to 0. */
@property CFTimeInterval duration;
```
&emsp;对象的基本持续时间。预设为 0。

#### speed
&emsp;指定时间如何从父时间空间映射到 receiver 的时间空间。
```c++
/* The rate of the layer. Used to scale parent time to local time, 
 * e.g. if rate is 2, local time progresses twice as fast as parent time.
 * Defaults to 1. 
 */
@property float speed;
```
&emsp;例如，如果 speed 为 2.0，则本地时间的进度是父时间的两倍。默认为 1.0。

&emsp;layer 的速率。用于将父时间缩放为本地时间，例如如果比率为 2，则本地时间的进度是父时间的两倍。
### Playback Modes（播放模式）
#### autoreverses
&emsp;确定 receiver 在完成时是否反向播放。
```c++
/* When true, the object plays backwards after playing forwards. Defaults to NO. */
@property BOOL autoreverses;
```
&emsp;如果为 true，则对象在向前播放后向后播放。默认为 NO。
#### fillMode
&emsp;确定 receiver 的 presentation 在其有效期限完成后是否被冻结或删除。可能的值在 Fill Modes 中说明。默认值为 kCAFillModeRemoved。
```c++
/* Defines how the timed object behaves outside its active duration.
 * Local time may be clamped to either end of the active duration, 
 * or the element may be removed from the presentation. 
 * The legal values are 'backwards', 'forwards', 'both' and 'removed'. 
 * Defaults to 'removed'. 
 */
@property(copy) CAMediaTimingFillMode fillMode;
```
&emsp;定义 timed object 在其活动持续时间之外的行为。本地时间可以固定在活动持续时间的任一端，或者可以从 presentation 中删除该元素。合法值是 backwards、forwards、both 和 removed。默认为 removed。
#### Fill Modes
&emsp;这些常数确定了 timed object 的活动持续时间完成后的行为。它们与 fillMode 属性一起使用。
```c++
typedef NSString * CAMediaTimingFillMode NS_TYPED_ENUM;
/* `fillMode' options. */

CA_EXTERN CAMediaTimingFillMode const kCAFillModeForwards   API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CAMediaTimingFillMode const kCAFillModeBackwards  API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CAMediaTimingFillMode const kCAFillModeBoth       API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CAMediaTimingFillMode const kCAFillModeRemoved    API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```

+ kCAFillModeForwards: 动画完成后，receiver 在其最终状态下仍然可见。
+ kCAFillModeBackwards: The receiver clamps values before zero to zero when the animation is completed.
+ kCAFillModeBoth: receiver 将值固定在对象时间空间的两端。
+ kCAFillModeRemoved: 动画完成后，receiver 将从 presentation 中删除。

## CAAction
&emsp;允许对象响应由 CALayer 更改触发的 actions 的接口。(CAAction 是一个仅有一个代理方法的 protocol。CAAnimation 类遵循 CAAction 协议。)

&emsp;当使用 action 标识符（key path、外部 action 名称或预定义 action 标识符）查询时，CALayer 返回相应的 action 对象（必须实现 CAAction 协议），并向其发送 `runActionForKey:object:arguments:` 消息。

### Responding to an action（响应操作）
#### - runActionForKey:object:arguments:
&emsp;调用以触发标识符指定的 action。

&emsp;`key`: action 的标识符。标识符可以是相对于对象的键或键路径、任意外部 action 或 CALayer 中定义的 action 标识符之一。`anObject`: 发生 action 的 CALayer。`dict`: 包含与此 `event` 关联的参数的字典。可能是 nil。
```c++
/** Action (event handler) protocol. **/

@protocol CAAction

- (void)runActionForKey:(NSString *)event object:(id)anObject arguments:(nullable NSDictionary *)dict;

@end

/** NSNull protocol conformance. **/

@interface NSNull (CAActionAdditions) <CAAction>

@end
```
> &emsp;/* Called to trigger the event named 'path' on the receiver. 
  *调用以触发 receiver 上名为 'event' 的事件。
  * The object (e.g. the layer) on which the event happened is 'anObject'. 
  * 发生事件的对象（例如 CALayer）为 'anObject'。
  * The arguments dictionary may be nil, if non-nil it carries parameters associated with the event. 
  * 参数字典可能是 nil，如果非 nil，它携带与事件关联的参数。
  */
  
## CALayerDelegate
&emsp;CALayer 的 delegate 对象需要遵循此协议，以响应与 CALayer 相关的事件。
```c++
@protocol CALayerDelegate <NSObject>
@optional // CALayerDelegate 的协议方法都是可选的
...
@end
```
&emsp;你可以实现此协议的方法来提供 CALayer 的内容、处理 sublayers 的布局以及提供要执行的自定义动画动作（custom animation actions）。必须将实现此协议的对象分配给 CALyer 对象的 delegate 属性。

&emsp;在 iOS 中 View 的 layer 的 delegate 默认是 View 本身。如下示例代码打印：
```c++
NSLog(@"😻😻 view 本身: %@", self.view);
NSLog(@"😻😻 view 的 layer 的 delegate: %@", self.view.layer.delegate);
// 控制台打印：
😻😻 view 本身: <UIView: 0x7fcdf090b170; frame = (0 0; 390 844); autoresize = W+H; layer = <CALayer: 0x6000038df680>>
😻😻 view 的 layer 的 delegate: <UIView: 0x7fcdf090b170; frame = (0 0; 390 844); autoresize = W+H; layer = <CALayer: 0x6000038df680>>
```
### Providing the Layer's Content（提供 CALayer 的内容）
#### - displayLayer:
&emsp;告诉 delegate 执行显示过程。
```c++
- (void)displayLayer:(CALayer *)layer;
```
&emsp;`layer`: 其内容需要更新的 CALayer。

&emsp;`- displayLayer:` 委托方法在 CALayer 被标记为要重新加载其内容时被调用，通常由 `setNeedsDisplay` 方法启动（标记）。典型的更新技术是设置 CALayer 的 contents 属性。

> &emsp;如果已实现此委托方法，则由 `- display` 方法的默认实现调用，在这种情况下，它应该实现整个显示过程（通常通过设置 contents 属性）。
#### - drawLayer:inContext:
&emsp;告诉 delegate 使用 CALayer 的 CGContextRef 实现显示过程。
```c++
- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)ctx;
```
&emsp;`layer`: 需要绘制其内容的 CALayer。`ctx`: 用于绘图的图形上下文。图形上下文包含用于绘制到目标屏幕的适当比例因子。

&emsp;`drawLayer:inContext:` 委托方法在 CALayer 被标记为要重新加载其内容时调用，通常使用 `setNeedsDisplay` 方法标记。如果 delegate 实现了 `displayLayer:` 方法，则不调用它。可以使用上下文来绘制向量，例如曲线和直线，或者使用 `draw(_:in:byTiling:)` 方法绘制图像。

> &emsp;Important:如果 delegate 实现了 `displayLayer:` 方法，则不会调用此方法。

> &emsp;如果已实现此委托方法，则由 `- drawInContext:` 方法的默认实现调用。
#### - layerWillDraw:
&emsp;通知 delegate 即将 draw。
```c++
- (void)layerWillDraw:(CALayer *)layer API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
```
&emsp;`layer`: 将绘制其内容的 CALayer。

&emsp;在 `drawLayer:inContext:` 之前调用 `layerWillDraw:` 方法。你可以使用此方法在 `drawLayer:inContext:` 之前配置影响 contents 的任何 CALayer 状态，例如 contentsFormat 和 opaque。

> &emsp;Important:如果 delegate 实现了 `displayLayer:` 方法，则不会调用此方法。

> &emsp;如果已实现此委托方法，则由 `- display` 方法的默认实现调用。允许委托在 `- drawLayer:InContext:` 之前配置影响 contents 的任何 CALayer 状态，例如 contentsFormat 和 opaque。如果委托实现 `- displayLayer`，则不会调用该方法。
### Laying Out Sublayers（布局子图层）
#### - layoutSublayersOfLayer:
&emsp;告诉 delegate CALayer 的 bounds 已更改。
```c++
- (void)layoutSublayersOfLayer:(CALayer *)layer;
```
&emsp;`layer`: 需要布局其 sublayers 的 CALayer。

&emsp;`layoutSublayersOfLayer:` 方法在 CALayer 的 bounds 发生更改时调用，例如通过更改其 frame 的大小。如果需要精确控制 CALayer 的 sublayers 的布局，可以实现此方法。

> &emsp;在检查 layout manager 之前，由默认的 `- layoutSublayers` 实现调用。请注意，如果调用了委托方法（`- layoutSublayersOfLayer:`），则 layout manager 将被忽略。
### Providing a Layer's Actions（提供图层的操作）
#### - actionForLayer:forKey:
&emsp;返回 `actionForKey:` 方法的默认 action。
```c++
- (nullable id<CAAction>)actionForLayer:(CALayer *)layer forKey:(NSString *)event;
```
&emsp;`layer`: 作为 action target 的 CALayer。`event`: action 的标识符。

&emsp;Return Value: 实现 CAAction 协议的对象，如果 delegate 没有为指定的 event 指定行为，则为 nil。

&emsp;实现此方法的 CALayer 的 delegate 返回指定键的 action 并停止任何进一步的搜索（即，不返回 CALayer 的 actions 字典中相同键的 action 或 `+ defaultActionForKey:` 指定的 action）。

> &emsp;如果已实现此委托方法，则由 `- actionForKey:` 方法的默认实现调用。应返回实现 CAAction 协议的对象。如果 delegate 未指定当前 `event` 的行为，则可能返回 nil。返回空对象（即 [NSNull null]）会明确强制不再进行进一步的搜索。（即，`+ defaultActionForKey:` 方法将不会被调用。）

## CAMediaTiming、CAAction、CALayerDelegate 总结 
&emsp;到这里看完了 CAMediaTiming、CAAction、CALayerDelegate 三个 protocol。其中 CAAnimation 遵循 CAMediaTiming 和 CAAction 协议，CALayer 遵循 CAMediaTiming 协议，而 CALayerDelegate 则是 CALayer 的 delegate 遵循的协议。

+ CAMediaTiming 协议中控制动画开始时间（beginTime、timeOffset 属性）、设置重复动画的次数或者重复时间（repeatCount、repeatDuration 属性）、设置动画的持续时间和速度（duration、speed 属性）、动画播放模式（autoreverses 是否结束后反向播放、fillMode 动画结束后的 向前、向后、两端、移除）。
+ CAAction 协议则是仅有 CAAnimation 类遵循此协议，仅有一个委托方法 `- runActionForKey:object:arguments:` 用于响应当 CALayer 更改时触发 Action。（执行添加到 CALayer 上的某个 CAAnimation 动画对象）
+ CALayerDelegate 协议则是提供给 CALayer 的 delegate 必须遵守的协议，实现三个作用：提供 CALayer 的内容、布局 CALayer 子图层（`- layoutSublayersOfLayer:`）、提供图层的操作（`- actionForLayer:forKey:`）。
  但是它的所有协议方法默认都是可选的（@optional）。其中 `- displayLayer:` 或 `- drawLayer:inContext:` 以两种不同的方式为 CALayer 提供内容，不过 `- displayLayer:` 执行级别高于 `- drawLayer:inContext:`，当 CALayer 的 delegate 实现了 `- displayLayer:` 方法后则不再调用 `- drawLayer:inContext:` 方法。
  `- displayLayer:` 委托方法通常在 CALayer 调用其 `setNeedsDisplay` 方法标记 CALayer 需要重新加载其内容时被调用，且 CALayer 的 `- display` 方法的默认实现会调用 `- displayLayer:` 委托方法。
  同样，当 `- displayLayer:` 委托方法未实现时，`- drawLayer:inContext:`  委托方法通常在 CALayer 调用其 `setNeedsDisplay` 方法标记 CALayer 需要重新加载其内容时被调用，不同的是 CALayer 的 `- drawInContext:` 方法的默认实现会调用 `- drawLayer:inContext:`  委托方法。
  而 `- layerWillDraw:` 委托方法则是在 `- drawLayer:inContext:` 之前调用。你可以使用此方法在 `- drawLayer:inContext:` 之前配置影响 contents 的任何 CALayer 状态，例如 contentsFormat 和 opaque。

&emsp;那么下面我们继续详细分析 CALayer 的文档。

## CALayer
&emsp;管理基于图像的内容并允许你对该内容执行动画的对象。（继承自 NSObject 并遵循 CAMediaTiming 协议）
```c++
API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0))
@interface CALayer : NSObject <NSSecureCoding, CAMediaTiming>
{
@private
  struct _CALayerIvars {
    int32_t refcount;
    uint32_t magic;
    void *layer;
#if TARGET_OS_MAC && !TARGET_RT_64_BIT
    void * _Nonnull unused1[8];
#endif
  } _attr;
}
```
### Overview（概述）
&emsp;Layers 通常用于为 view 提供 backing store，但也可以在没有 view 的情况下使用以显示内容。layer 的主要工作是管理你提供的视觉内容（visual content），但 layer 本身也具有可以设置的视觉属性（visual attributes），例如背景色（background color）、边框（border）和阴影（shadow）。除了管理视觉内容外，layer 还维护有关其内容的几何（geometry）（例如其位置（position）、大小（size）和变换（transform））的信息，这些信息用于在屏幕上显示该内容。修改 layer 的属性是在 layer 的内容或几何（geometry）上启动动画的方式。layer 对象通过 CAMediaTiming 协议封装 layer 及其动画的持续时间（duration）和步调（pacing），该协议定义了 layer 的时间信息（timing information）。

&emsp;如果 layer 对象是由 view 创建的，则 view 通常会自动将自身指定为 layer 的 delegate，并且不应更改该关系。对于你自己创建的 layers，可以为其指定一个 delegate 对象，并使用该对象动态提供 layer 的内容并执行其他任务。layer 可能还具有布局管理器（layout manager）对象（指定给 layoutManager 属性），以分别管理子图层（sublayers）的布局。
### Creating a Layer（创建 layer）
#### + layer
&emsp;创建并返回 layer 对象的实例。
```c++
+ (instancetype)layer;
```
&emsp;Return Value: 初始化的 layer 对象；如果初始化失败，则返回 nil。

&emsp;如果你是 CALayer 的子类，则可以重写此方法，并使用该函数提供特定子类的实例。
#### - init
&emsp;返回一个初始化的 CALayer 对象。
```c++
- (instancetype)init;
```
&emsp;这是不在 presentation layer（表示层）中的 layer 对象的指定初始化程序。
#### - initWithLayer:
&emsp;重写以复制或初始化指定 layer 的自定义字段。

&emsp;Core Animation 使用此初始值设定项来创建 layers 的 shadow 副本，例如用作 presentation layers。子类可以重写此方法，以将其实例变量复制到 presentation layer 中（子类随后应调用超类）。在任何其他情况下调用此方法都将导致未定义的行为。
```c++
- (instancetype)initWithLayer:(id)layer;
```
&emsp;`layer`: 应从其复制自定义字段的 layer。

&emsp;Return Value:从 `layer` 复制的任何自定义实例变量的 layer 实例。

&emsp;此初始化程序用于创建 layer 的 shadow 副本，例如，用于 `presentationLayer` 方法。在任何其他情况下使用此方法都会产生不确定的行为。例如，请勿使用此方法用现有 layer 的内容初始化新 layer。

&emsp;如果要实现自定义 layer 子类，则可以重写此方法并将其用于将实例变量的值复制到新对象中。子类应始终调用超类实现。

&emsp;此方法是 presentation layer（表示层）中各 layer 对象的指定初始化器。
### Accessing Related Layer Objects（访问相关 layer 对象）
#### - presentationLayer
&emsp;返回 presentation layer 对象的副本，该对象表示当前在屏幕上显示的 layer 的状态。
```c++
- (nullable instancetype)presentationLayer;
```
&emsp;Return Value: 当前 presentation layer 对象的副本。

&emsp;通过此方法返回的 layer 对象提供了当前在屏幕上显示的 layer 的近似值。在动画制作过程中，你可以检索该对象并使用它来获取那些动画的当前值。

&emsp;返回 layer 的 sublayers、mask 和 superlayer 属性从表示树（presentation tree）（而不是模型树）返回相应的对象。此模式也适用于任何只读 layer 方法。例如，返回对象的 hitTest: 方法查询 presentation tree 中的 layer 对象。

> &emsp;返回包含所有属性的 CALayer 的副本，这些属性与当前 transaction 开始时的属性相同，并应用了所有活动动画。这非常接近当前显示的 CALayer 版本。如果尚未 committed 该 CALayer，则返回 nil。
> &emsp;尝试以任何方式修改返回的 CALayer 对象，其产生的效果都是不确定的。（返回的 CALayer 对象仅应当用于读取其当前的各种属性）
> &emsp;返回的 CALayer 对象的 sublayers、mask 和 superlayer 属性返回的是这些属性的 presentation versions 。这将执行只读 CALayer 方法。例如，在 `- presentationLayer` 返回的 CALayer 对象上调用 `- hitTest:` 将查询 layer tree 的 presentation values。

#### - modelLayer
&emsp;返回与 receiver 关联的 model layer 对象（如果有）。
```c++
- (instancetype)modelLayer;
```
&emsp;Return Value: 表示基础模型层（underlying model layer）的 CALayer 实例。

&emsp;在 presentation tree 中的 CALayer 上调用此方法将返回 model tree 中的相应 CALayer 对象。仅当涉及表示层更改的事务正在进行时，此方法才返回值。如果没有正在进行的事务，则调用此方法的结果是不确定的。

> &emsp;在 `- presentationLayer` 方法的结果上调用时，返回具有当前模型值的基础层。在非表示层上调用时，返回接收者。产生表示层的事务完成后调用此方法的结果是不确定的。

&emsp;上面 `- presentationLayer` 和 `- modelLayer` 函数涉及到的 “表示树” 和 “模型树” 在后面的 Core Animation 文档中有详细解释。
### Accessing the Delegate（访问 CALayer 的代理）
#### delegate
&emsp;CALayer 的委托对象。（delegate 是遵循 CALayerDelegate 协议的 weak 属性）
```c++
@property(nullable, weak) id <CALayerDelegate> delegate;
```
&emsp;你可以使用委托对象来提供图层的内容，处理任何子图层的布局以及提供自定义操作以响应与图层相关的更改。你分配给此属性的对象应实现 CALayerDelegate 非正式协议的一种或多种方法。关于协议的更多信息，请参见上面 CALayerDelegate 协议分析。

&emsp;在 iOS 中，如果图层与 UIView 对象关联，则必须将此属性设置为拥有该 CALayer 的 UIView。（在 iOS 中 UIView 默认作为其 layer 属性的 delegate 对象，但是对于自己创建的 CALayer 对象其 delegate 属性默认是 nil）
### Providing the Layer’s Content（提供 CALayer 的内容）
#### contents
&emsp;提供 CALayer 内容的对象。可动画的。（strong 修饰的 id 类型的属性）
```c++
@property(nullable, strong) id contents;
```
&emsp;此属性的默认值为 nil。

&emsp;如果使用 CALayer 显示静态图像，则可以将此属性设置为 CGImageRef，其中包含要显示的图像。 （在 macOS 10.6 及更高版本中，你也可以将此属性设置为 NSImage 对象。）为该属性分配值会导致 CALayer 使用你的图像，而不是创建单独的 backing store。

&emsp;如果图层对象绑定到 UIView 对象，则应避免直接设置此属性的内容。视图和图层之间的相互作用通常会导致视图在后续更新期间替换此属性的内容。
#### contentsRect
&emsp;单位坐标空间中的矩形，用于定义应使用的 CALayer 内容部分。可动画的。
```c++
@property CGRect contentsRect;
```
&emsp;默认为单位矩形（0.0、0.0、1.0、1.0）。

&emsp;如果请求单位矩形之外的像素，则内容图像的边缘像素将向外扩展。

&emsp;如果提供了一个空矩形，则结果是不确定的。

> &emsp;标准化图像坐标中的矩形，定义了将被绘制到图层中的 contents 属性的子矩形。如果请求单位矩形之外的像素，则内容图像的边缘像素将向外扩展。如果提供了一个空矩形，则结果是不确定的。默认为单位矩形[0 0 1 1]。可动画的。

#### contentsCenter
&emsp;用于定义在调整图层内容大小时如何缩放图层内容。可动画的。
```c++
@property CGRect contentsCenter;
```
&emsp;可以使用此属性将图层内容细分为 3x3 网格。此属性中的值指定网格中中心矩形的位置和大小。如果层的 contentsGravity 属性设置为某个调整大小模式，则调整层的大小会导致网格的每个矩形中发生不同的缩放。中心矩形在两个维度上都拉伸，上中心和下中心矩形仅水平拉伸，左中心和右中心矩形仅垂直拉伸，四角矩形完全不拉伸。因此，你可以使用此技术使用三部分或九部分图像来实现可拉伸的背景或图像。

&emsp;默认情况下，此属性中的值设置为单位矩形（0.0,0.0）（1.0,1.0），这将导致整个图像在两个维度上缩放。如果指定的矩形超出单位矩形，则结果未定义。只有在将 contentsRect 属性应用于图像之后，才应用指定的矩形。

> &emsp;Note: 如果此属性中矩形的宽度或高度很小或为 0，则该值将隐式更改为以指定位置为中心的单个源像素的宽度或高度。

> &emsp;标准化图像坐标中的矩形定义了 contents 图像的缩放中心部分。
> &emsp;当图像由于其 contentsGravity 属性而调整大小时，其中心部分隐式定义了 3x3 网格，该网格控制如何将图像缩放到其绘制的大小。中心部分在两个方向上都拉伸。顶部和底部仅水平拉伸；左右部分仅垂直拉伸；四个角部分根本没有拉伸。 （这通常称为 "9-slice scaling"。）
> &emsp;矩形在应用了 contentsRect 属性的效果后被解释。默认为单位矩形 [0 0 1 1]，表示整个图像都会缩放。作为特殊情况，如果宽度或高度为零，则将其隐式调整为以该位置为中心的单个源像素的宽度或高度。如果矩形延伸到 [0 0 1 1] 单元矩形的外部，则结果不确定。可动画的。

#### - display
&emsp;重新加载该层的内容。
```c++
- (void)display;
```
&emsp;不要直接调用此方法。CALayer 会在适当的时候调用此方法以更新 CALayer 的内容。如果 CALayer 具有 delegate 对象，则此方法尝试调用 delegate 的 `displayLayer:` 方法，delegate 可使用该方法来更新 CALayer 的内容。如果 delegate 未实现 `displayLayer:` 方法，则此方法将创建 backing store 并调用 CALayer 的 `drawInContext:` 方法以将内容填充到该 backing store 中。新的 backing store 将替换该 CALayer 的先前内容。

&emsp;子类可以重写此方法，并使用它直接设置 CALayer 的 contents 属性。如果你的自定义 CALayer 子类对图层更新的处理方式不同，则可以执行此操作。

> &emsp;重新加载 CALayer 的内容。调用 `- drawInContext:` 方法，然后更新 CALayer 的 contents 属性。通常，不直接调用它。

#### - drawInContext:
&emsp;使用指定的图形上下文绘制 CALayer 的内容。
```c++
- (void)drawInContext:(CGContextRef)ctx;
```
&emsp;`ctx`: 在其中绘制内容的图形上下文。上下文可以被裁剪以保护有效的 CALayer 内容。希望找到要绘制的实际区域的子类可以调用 CGContextGetClipBoundingBox。

&emsp;此方法的默认实现本身不会进行任何绘制。如果 CALayer 的 delegate 实现了 `- drawLayer:inContext:` 方法，则会调用该方法进行实际绘制。

&emsp;子类可以重写此方法，并使用它来绘制 CALayer 的内容。绘制时，应在逻辑坐标空间中的点指定所有坐标。

> &emsp;当 contents 属性被更新时，通过 `- display` 方法调用。默认实现不执行任何操作。上下文可以被裁剪以保护有效的 CALayer 内容。希望找到要绘制的实际区域的子类可以调用 `CGContextGetClipBoundingBox()`。

### Modifying the Layer’s Appearance（修改 CALayer 的外观）
#### contentsGravity
&emsp;一个常数，指定 CALayer 的 contents 如何在其 bounds 内 positioned 或 scaled。
```c++
@property(copy) CALayerContentsGravity contentsGravity;
```
&emsp;Contents Gravity Values 中列出了此属性的可能值。

&emsp;此属性的默认值为 kCAGravityResize。（调整内容大小以适合整个 bounds 矩形，可能会被拉伸变形）

> &emsp;Important: 内容重力常数（contents gravity constants）的命名基于垂直轴的方向。如果将重力常数与垂直分量（例如 kCAGravityTop）一起使用，则还应检查层的 contentsAreFlipped。如果该选项为 YES，kCAGravityTop 将 contents 与 CALayer 的底部对齐，kCAGravityBottom 将内容与层的顶部对齐。
> 
> &emsp;macOS 和 iOS 中视图的默认坐标系在垂直轴的方向上不同：在 macOS 中，默认坐标系的原点位于绘图区域的左下角，正值从中向上延伸，在 iOS 中，默认坐标系的原点位于绘图区域的左上角，正值从该坐标系向下延伸。

&emsp;图1显示了四个示例，这些示例为图层的 contentsGravity 属性设置不同的值。

&emsp;Figure 1 Different effects of setting a layer's contents gravity

![layer_contents_gravity](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/8a686a27637c4a11a1177ac37b7acba9~tplv-k3u1fbpfcp-watermark.image)

1. Contents gravity is kCAGravityResize - the default
2. Contents gravity is kCAGravityCenter
3. Contents gravity is contentsAreFlipped ? kCAGravityTop : kCAGravityBottom
4. Contents gravity is contentsAreFlipped ? kCAGravityBottomLeft : kCAGravityTopLeft

> &emsp;一个字符串，定义了如何将 CALayer 的 contents 映射到其 bounds 矩形。选项为'center'，'top'，'bottom'，'left'，'right'，'topLeft'，'topRight'，'bottomLeft'，'bottomRight'，'resize'，'resizeAspect'，'resizeAspectFill'。默认值为`resize'。注意，'bottom' 始终表示 **最小 Y**，'top' 始终表示 **最大 Y**。

#### Contents Gravity Values
&emsp;当 CALayer bounds 大于内容对象的 bounds 时，内容重力常量指定内容对象的位置。它们由 contentsGravity 属性使用。
```c++
CA_EXTERN CALayerContentsGravity const kCAGravityCenter API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityTop API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityBottom API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityLeft API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityRight API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityTopLeft API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityTopRight API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityBottomLeft API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityBottomRight API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityResize API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityResizeAspect API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
CA_EXTERN CALayerContentsGravity const kCAGravityResizeAspectFill API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
+ kCAGravityCenter: 内容在 bounds 矩形中水平和垂直居中。
+ kCAGravityTop: 内容在 bounds 矩形的上边缘水平居中。
+ kCAGravityBottom: 内容在 bounds 矩形的下边缘水平居中。
+ kCAGravityLeft: 内容在 bounds 矩形的左边缘垂直居中。
+ kCAGravityRight: 内容在 bounds 矩形的右边缘垂直居中。
+ kCAGravityTopLeft: 内容位于 bounds 矩形的左上角。
+ kCAGravityTopRight: 内容位于 bounds 矩形的右上角。
+ kCAGravityBottomLeft: 内容位于 bounds 矩形的左下角。
+ kCAGravityBottomRight: 内容位于 bounds 矩形的右下角。
+ kCAGravityResize: 调整内容大小以适合整个 bounds 矩形。（可能会拉伸变形）
+ kCAGravityResizeAspect: 调整内容大小以适合 bounds 矩形，从而保留内容的外观（保留宽高比例）。如果内容没有完全填充 bounds 矩形，则内容将以部分轴为中心。
+ kCAGravityResizeAspectFill:调整内容大小以完全填充 bounds 矩形，同时仍保留内容的外观（保留宽高比例）。内容以其超过的轴为中心。

#### opacity
&emsp;receiver 的不透明度。可动画的。
```c++
@property float opacity;
```
&emsp;此属性的值必须在 0.0（透明）到 1.0（不透明）之间。超出该范围的值将被限制为最小值或最大值。此属性的默认值为1.0。

> &emsp;CALayer 的不透明度，介于 0 和 1 之间的值。默认为 1。指定超出 [0,1] 范围的值将产生不确定的结果。可动画的。

#### hidden
&emsp;指示是否隐藏 CALayer 的布尔值。可动画的。
```c++
@property(getter=isHidden) BOOL hidden;
```
&emsp;此属性的默认值为 NO。

> &emsp;如果为true，则不显示该图层及其子图层。默认为 NO。可动画的。

#### masksToBounds
&emsp;一个布尔值，指示是否将 sublayers 裁剪到该 CALayer 的 bounds。可动画的。
```c++
@property BOOL masksToBounds;
```
&emsp;当此属性的值为 YES 时，Core Animation 将创建一个隐含的剪贴 mask，该 mask 与 CALayer 的 bounds 匹配并包括任何 corner radius 效果。如果还指定了 mask 属性的值，则将两个 masks 相乘以获得最终的 mask 值。

&emsp;此属性的默认值为 NO。
#### mask
&emsp;可选图层，其 Alpha 通道用于屏蔽 CALaer 的内容。
```c++
@property(nullable, strong) __kindof CALayer *mask;
```
&emsp;CALayer 的 Alpha 通道决定了该 CALayer 的内容和背景可以显示多少。完全或部分不透明的像素允许基础内容显示出来，但是完全透明的像素会阻止该内容。

&emsp;此属性的默认值为 nil。配置 mask 时，请记住设置 mask 层的大小和位置，以确保其与 mask 的 CALayer 正确对齐。

&emsp;你分配给此属性的 CALayer 不得具有 superlayer。如果是这样，则行为是不确定的。

> &emsp;一个 CALayer，其 alpha 通道用作 mask，以在 CALayer 的背景和将其内容与其过滤的背景合成的结果之间进行选择。默认为 nil。当用作 mask 时，图层的 compositingFilter 和 backgroundFilters 属性将被忽略。将 mask 设置为 new layer 时，其 superlayer 必须为 nil，否则行为是不确定的。不支持嵌套 masks（具有自己的 mask 的 mask layer）。

#### doubleSided
&emsp;一个布尔值，指示当 facing away from the viewer，CALayer 是否显示其内容。可动画的。
```c++
@property(getter=isDoubleSided) BOOL doubleSided;
```
&emsp;当此属性的值为 NO 时，该层将背对查看器时隐藏其内容（when it faces away from the viewer）。此属性的默认值为 YES。

#### cornerRadius
&emsp;为 CAlayer 的背景绘制圆角时要使用的半径。可动画的。
```c++
@property CGFloat cornerRadius;
```
&emsp;将 radius 设置为大于 0.0 时 CALayer 可开始在其背景上绘制圆角。默认情况下，corner radius 不应用于 CALayer contents 属性中的图像；它仅应用于 CALayer 的背景色和边框。但是，将 masksToBounds 属性设置为 YES 会将内容剪裁到圆角。

&emsp;此属性的默认值为 0.0。
#### maskedCorners
&emsp;定义使用 cornerRadius 属性时四个角中的哪个角接受遮罩。默认为所有四个角。
```c++
@property CACornerMask maskedCorners API_AVAILABLE(macos(10.13), ios(11.0), watchos(4.0), tvos(11.0));
```
#### CACornerMask
&emsp;maskedCorners 属性的位定义。
```c++
typedef NS_OPTIONS (NSUInteger, CACornerMask)
{
  kCALayerMinXMinYCorner = 1U << 0,
  kCALayerMaxXMinYCorner = 1U << 1,
  kCALayerMinXMaxYCorner = 1U << 2,
  kCALayerMaxXMaxYCorner = 1U << 3,
};
```
#### borderWidth
&emsp;CALayer 边框的宽度。可动画的。
```c++
@property CGFloat borderWidth;
```
&emsp;当此值大于 0.0 时，CALayer 将使用当前的 borderColor 值绘制边框。边框是根据此属性中指定的值从接收者的 bounds 绘制的。它在接收者的内容和子层之上进行了合成，并包含 cornerRadius 属性的效果。

&emsp;此属性的默认值为 0.0。
#### borderColor
&emsp;CALayer 边框的颜色。可动画的。（类型是 CGColorRef）
```c++
@property(nullable) CGColorRef borderColor;
```
&emsp;此属性的默认值为不透明的黑色。

&emsp;使用 Core Foundation retain/release 语义保留此属性的值。尽管该属性声明似乎使用默认的 assign 语义进行对象保留，但仍会发生此行为。
#### backgroundColor
&emsp;CALayer 的背景色。可动画的。
```c++
@property(nullable) CGColorRef backgroundColor;
```
&emsp;此属性的默认值为 nil。

&emsp;使用 Core Foundation retain/release 语义保留此属性的值。尽管该属性声明似乎使用默认的 assign 语义进行对象保留，但仍会发生此行为。
#### shadowOpacity
&emsp;CALayer 阴影的不透明度。可动画的。
```c++
@property float shadowOpacity;
```
&emsp;此属性中的值必须在 0.0（透明）到 1.0（不透明）之间。此属性的默认值为 0.0。

> &emsp;阴影的不透明度。默认值为 0。指定 [0,1] 范围以外的值将得到不确定的结果。可动画的。

#### shadowRadius
&emsp;用于渲染 CALayer 阴影的模糊半径（以 points 为单位）。可动画的。
```c++
@property CGFloat shadowRadius;
```
&emsp;指定半径此属性的默认值为 3.0。

> &emsp;用于创建阴影的模糊半径。默认值为 3.0。可设置动画。

#### shadowOffset
&emsp;CALayer 阴影的偏移量（以 points 为单位）。可动画的。
```c++
@property CGSize shadowOffset;
```
&emsp;此属性的默认值为（0.0，-3.0）。
#### shadowColor
&emsp;CALayer 阴影的颜色。可动画的。
```c++
@property(nullable) CGColorRef shadowColor;
```
&emsp;使用 Core Foundation retain/release 语义保留此属性的值。尽管该属性声明似乎使用默认的 assign 语义进行对象保留，但仍会发生此行为。

> &emsp;阴影的颜色。默认为不透明黑色。当前不支持从图案创建的颜色。可动画的。

#### shadowPath
&emsp;CALayer 阴影的形状。可动画的。
```c++
@property(nullable) CGPathRef shadowPath;
```
&emsp;此属性的默认值为 nil，这会导致 CALayer 使用标准阴影形状（tandard shadow shape）。如果为此属性指定值，则 CALayer 将使用指定的路径而不是 CALayer 的合成 alpha 通道创建其阴影。你提供的路径定义了阴影的轮廓。使用 non-zero winding 规则和当前阴影颜色、不透明度和模糊半径填充。

&emsp;与大多数可设置动画的属性不同，此属性（与所有 CGPathRef 可设置动画的属性一样）不支持隐式动画。但是，可以使用 CAPropertyAnimation 的任何具体子类为路径对象设置动画。路径将插值为 "on-line" 点的线性混合；"off-line" 点可以非线性插值（以保持曲线导数的连续性）。如果两条路径具有不同数量的控制点或段，则结果未定义。如果路径延伸到层边界之外，它将不会自动剪裁到层，只有在正常的 masking rules 规则导致这种情况时。

&emsp;指定显式路径通常可以提高渲染性能。

&emsp;使用 Core Foundation retain/release 语义保留此属性的值。尽管该属性声明似乎使用默认的 assign 语义进行对象保留，但仍会发生此行为。

> &emsp;当为非 null 时，此路径定义用于构造图层阴影的轮廓，而不是使用图层的合成 Alpha 通道。使用非零缠绕规则渲染路径。使用此属性显式指定路径通常可以提高渲染性能，因为可以在多个图层之间共享相同的路径引用。分配后，路径将被复制。默认为 nil。可动画的。

##### Using Shadow Path for Special Effects（使用阴影路径进行特殊效果）
&emsp;你可以使用图层的阴影路径来创建特殊效果，例如模拟 Pages 中可用的阴影。

&emsp;清单1 显示了将椭圆阴影添加到图层底部以模拟 Pages Contact Shadow 效果所需的代码。

&emsp;Listing 1 Creating a contact shadow path
```c++
let layer = CALayer()
     
layer.frame = CGRect(x: 75, y: 75, width: 150, height: 150)
layer.backgroundColor = NSColor.darkGray.cgColor
layer.shadowColor = NSColor.gray.cgColor
layer.shadowRadius = 5
layer.shadowOpacity = 1
     
let contactShadowSize: CGFloat = 20
let shadowPath = CGPath(ellipseIn: CGRect(x: -contactShadowSize,
                                          y: -contactShadowSize * 0.5,
                                          width: layer.bounds.width + contactShadowSize * 2,
                                          height: contactShadowSize),
                        transform: nil)
     
layer.shadowPath = shadowPath
```
&emsp;Figure 1 Layer with contact shadow effect

![Layer_with_contact_shadow_effect](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ef7dae0dc97c4891863b84ab28ce2183~tplv-k3u1fbpfcp-watermark.image)

&emsp;清单 2 显示了如何创建路径来模拟 Pages Curved Shadow。路径的左侧，顶部和右侧是直线，而底部是凹曲线，如图 2 所示。

&emsp;Figure 2 Shadow path for curved shadow effect

![Shadow_path_for_curved_shadow_effect](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e7fb1edcb830450693b85583968ece0b~tplv-k3u1fbpfcp-watermark.image)

&emsp;Listing 2 Creating a curved shadow path
```c++
let layer = CALayer()
layer.frame = CGRect(x: 75, y: 75, width: 150, height: 150)
layer.backgroundColor = NSColor.darkGray.cgColor
     
layer.shadowColor = NSColor.black.cgColor
layer.shadowRadius = 5
layer.shadowOpacity = 1
     
let shadowHeight: CGFloat = 10
let shadowPath = CGMutablePath()
shadowPath.move(to: CGPoint(x: layer.shadowRadius,
                            y: -shadowHeight))
shadowPath.addLine(to: CGPoint(x: layer.shadowRadius,
                               y: shadowHeight))
shadowPath.addLine(to: CGPoint(x: layer.bounds.width - layer.shadowRadius,
                               y: shadowHeight))
shadowPath.addLine(to: CGPoint(x: layer.bounds.width - layer.shadowRadius,
                               y: -shadowHeight))
     
shadowPath.addQuadCurve(to: CGPoint(x: layer.shadowRadius,
                                    y: -shadowHeight),
                        control: CGPoint(x: layer.bounds.width / 2,
                                         y: shadowHeight))
     
layer.shadowPath = shadowPath
```
&emsp;Figure 3 Layer with curved shadow effect

![Layer_with_curved_shadow_effect](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/68a673c08d0b46149bb04743af64ab96~tplv-k3u1fbpfcp-watermark.image)

#### style
&emsp;可选 NSDictionary，用于存储未由 CALayer 明确定义的属性值
```c++
@property(nullable, copy) NSDictionary *style;
```

&emsp;该 NSDictionary 又可以具有 style key，从而形成默认值的层次结构。如果是分层样式字典（hierarchical style dictionaries），则使用属性的最浅值。例如，style.someValue 的值优先于 style.style.someValue。

&emsp;如果 style dictionary 未为属性定义值，则调用 CALayer 的 `+ defaultValueForKey:` 方法。此属性的默认值为 nil。

&emsp;下列关键字不参考 style dictionary：bounds、frame。

> &emsp;Warning: 如果修改了 style dictionary 或其任何 ancestors，则在重置样式属性之前，CALayer 属性的值是不确定的。

> &emsp;非 nil 时，dictionary 取消引用以查找 CALayer 未显式定义的属性值。 （此 dictionary 可能又具有 style 属性，形成默认值的层次结构。）如果 style dictionary 未为属性定义值，则调用 `+ defaultValueForKey:` 方法。默认为 nil。

#### allowsEdgeAntialiasing
&emsp;一个布尔值，指示是否允许该 CALayer 执行边缘抗锯齿。
```c++
@property BOOL allowsEdgeAntialiasing API_AVAILABLE(macos(10.10), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;值为 YES 时，允许 CALayer 按照其 edgeAntialiasingMask 属性中的值要求对其边缘进行抗锯齿。默认值是从 main bundle 的 Info.plist 文件中的 boolean UIViewEdgeAntialiasing 属性读取的。如果未找到任何值，则默认值为 NO。
#### allowsGroupOpacity
&emsp;一个布尔值，指示是否允许该 CALayer 将自身与其父级分开组合为一个组。
```c++
@property BOOL allowsGroupOpacity API_AVAILABLE(macos(10.10), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;当值为 YES 且 CALayer 的 opacity 属性值小于 1.0 时，允许 CALayer 将其自身组合为与其父级分开的组。当 CALayer 包含多个不透明组件时，这会给出正确的结果，但可能会降低性能。

&emsp;默认值是从 main bundle 的 Info.plist 文件中的 boolean UIViewGroupOpacity 属性读取的。如果未找到任何值，则对与 iOS 7 SDK 或更高版本链接的应用程序的默认值为 YES，对于与较早的 SDK 链接的应用程序的默认值为 NO。
### Layer Filters（图层过滤器）
#### filters
&emsp;一组 Core Image 过滤器，可应用于 CALayer 及其 sublayers 的内容。可动画的。
```c++
@property(nullable, copy) NSArray *filters;
```
&emsp;你添加到此属性的过滤器会影响 CALayer 的内容，包括其 border，填充的背景和 sublayers。此属性的默认值为 nil。

&emsp;在 CIFilter 对象附加到 CALayer 之后直接更改其输入会导致未定义的行为。可以在将过滤器附着到 CALayer 后修改过滤器参数，但必须使用图层的 `setValue:forKeyPath:` 方法执行此操作。此外，必须为过滤器指定一个名称，以便在数组中标识它。例如，要更改过滤器的 inputRadius 参数，可以使用类似以下代码：
```c++
CIFilter *filter = ...;
CALayer *layer = ...;
 
filter.name = @"myFilter";
layer.filters = [NSArray arrayWithObject:filter];
[layer setValue:[NSNumber numberWithInt:1] forKeyPath:@"filters.myFilter.inputRadius"];
```
&emsp;iOS 中的图层不支持此属性。
#### compositingFilter
&emsp;一个 Core Image 滤镜，用于合成 CALayer 及其背后的内容。可动画的。
```c++
@property(nullable, strong) id compositingFilter;
```
&emsp;此属性的默认值为 nil，这将导致图层使用源覆盖合成。尽管你可以将任何 Core Image 滤镜用作图层的合成滤镜，但为获得最佳效果，请使用 CICategoryCompositeOperation 类别中的滤镜。

&emsp;在 macOS 中，可以在将过滤器附加到图层后再对其参数进行修改，但是你必须使用该图层的 `setValue:forKeyPath:` 方法。例如，要更改过滤器的 inputRadius 参数，可以使用类似于以下代码：
```c++
CIFilter *filter = ...;
CALayer *layer = ...;
 
layer.compositingFilter = filter;
[layer setValue:[NSNumber numberWithInt:1] forKeyPath:@"compositingFilter.inputRadius"];
```
&emsp;在 CIFilter 对象附加到层之后直接更改其输入会导致未定义的行为。

&emsp;iOS 中的图层不支持此属性。
#### backgroundFilters
&emsp;一组 Core Image 过滤器，可应用于紧靠该图层后面的内容。可动画的。
```c++
@property(nullable, copy) NSArray *backgroundFilters;
```
&emsp;背景过滤器会影响显示在图层本身中的图层后面的内容。通常，此内容属于充当该层父级的 superlayer。这些滤镜不会影响图层本身的内容，包括图层的背景颜色和边框。

&emsp;此属性的默认值为 nil。

&emsp;在 CIFilter 对象附加到层之后直接更改其输入会导致未定义的行为。在 macOS 中，可以在将过滤器附加到图层后修改过滤器参数，但必须使用图层的 `setValue:forKeyPath:` 执行此操作的方法。此外，必须为过滤器指定一个名称，以便在数组中标识它。例如，要更改过滤器的 inputRadius 参数，可以使用类似以下代码：
```c++
CIFilter *filter = ...;
CALayer *layer = ...;
 
filter.name = @"myFilter";
layer.backgroundFilters = [NSArray arrayWithObject:filter];
[layer setValue:[NSNumber numberWithInt:1] forKeyPath:@"backgroundFilters.myFilter.inputRadius"];
```
&emsp;你可以使用图层的 masksToBounds 来控制其背景滤镜效果的程度。

> &emsp;应用于图层背景的滤镜数组。root layer 将忽略此属性。可动画的。

#### minificationFilter
&emsp;减小内容大小时使用的过滤器。
```c++
@property(copy) CALayerContentsFilter minificationFilter;
```
&emsp;Scaling Filters 中列出了此属性的可能值。

&emsp;此属性的默认值为 kCAFilterLinear。

> &emsp;呈现 CALayer 的 contents 属性时要使用的过滤器类型。缩小滤镜用于减小图像数据的大小，放大滤镜用于增大图像数据的大小。当前允许的值为 "nearest" 和 "linear"。这两个属性默认为 "linear"。

#### minificationFilterBias
&emsp;缩小过滤器用来确定详细程度的偏差因子。
```c++
@property float minificationFilterBias;
```
&emsp;当将此值设置为 kCAFilterTrilinear 时，minificationFilter 将使用此值。

&emsp;此属性的默认值为 0.0。

> &emsp;在确定使用三线性过滤最小化时使用的细节级别时添加的偏差因子。默认值为 0。可设置动画。

#### magnificationFilter
&emsp;增加内容大小时使用的过滤器。
```c++
@property(copy) CALayerContentsFilter magnificationFilter;
```
&emsp;Scaling Filters 中列出了此属性的可能值。

&emsp;此属性的默认值为 kCAFilterLinear。

&emsp;图 1 显示了当将一个10 x 10 点的圆图像放大 10 倍时，线性 filtering 和最近 filtering 之间的差异。

&emsp;Figure 1 Circle with different magnification filters

![Circle_with_different_magnification_filters](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/524d0e4609cf4fdf8dd14721e90fb785~tplv-k3u1fbpfcp-watermark.image)

&emsp;左侧的圆圈使用 kCAFilterLinear，右侧的圆圈使用 kCAFilterNearest。
### Configuring the Layer’s Rendering Behavior（配置图层的渲染行为）
#### opaque
&emsp;一个布尔值，指示该图层是否包含完全不透明的内容。
```c++
@property(getter=isOpaque) BOOL opaque;
```
&emsp;此属性的默认值为 NO。如果应用程序绘制的内容完全不透明，并且填充了层的 bounds，则将此属性设置为 YES 可使系统优化层的呈现行为。具体来说，当层为绘图命令创建 backing store 时，Core Animation 会忽略该 backing store 的 alpha 通道。这样做可以提高合成操作的性能。如果将此属性的值设置为 YES，则必须用不透明内容填充图层 bounds。

&emsp;设置此属性仅影响由 Core Animation 管理的 backing store。如果你将具有 Alpha 通道的图像分配给图层的 contents 属性，则该图像将保留其 Alpha 通道，而不管该属性的值如何。

> &emsp;提示 `- drawInContext:` 提供的图层内容是完全不透明的。默认为 NO。请注意，这不会直接影响对 contents 属性的解释。

#### edgeAntialiasingMask
&emsp;定义如何光栅化 CALayer 边缘的位掩码。
```c++
@property CAEdgeAntialiasingMask edgeAntialiasingMask;
```
&emsp;此属性指定层的哪些边缘被消除锯齿，并且是 CAEdgeAntialiasingMask 中定义的常量的组合。你可以分别为每个边缘（顶部，左侧，底部，右侧）启用或禁用抗锯齿。默认情况下，所有边缘均启用抗锯齿。

&emsp;通常，你将使用此属性为与其他层的边缘邻接的边缘禁用抗锯齿，以消除否则会发生的接缝。
#### - contentsAreFlipped
&emsp;返回一个布尔值，指示在渲染时是否隐式翻转图层内容。
```c++
- (BOOL)contentsAreFlipped;
```
&emsp;Return Value: 如果在渲染时隐式翻转了图层内容，则为 YES；否则为 NO。默认情况下，此方法返回 NO。

&emsp;此方法提供有关在绘制过程中是否翻转图层内容的信息。你不应尝试覆盖此方法并返回其他值。

&emsp;如果图层需要翻转其内容，则它会从此方法返回 YES，并将 y-flip 转换应用于图形上下文，然后再将其传递给图层的 `- drawInContext:` 方法。同样，该图层会将传递给其 `- setNeedsDisplayInRect:` 的所有矩形转换为翻转的坐标空间。

#### geometryFlipped
&emsp;一个布尔值，指示该层及其子层的 geometry 是否垂直翻转。
```c++
@property(getter=isGeometryFlipped) BOOL geometryFlipped;
```
&emsp;如果层为 layer-backed view 提供支持，则该视图负责管理此属性中的值。对于独立图层，此属性控制是使用标准坐标系还是翻转坐标系来解释图层的 geometry。此属性的值不影响图层内容的呈现。

&emsp;此属性的默认值为 NO。

> &emsp;图层（及其子图层）的几何形状是否垂直翻转。默认为 NO。请注意，即使在翻转几何图形时，图像方向也保持不变（即存储在 contents 属性中的 CGImageRef 将显示相同的内容，并假设在层上未进行任何变换时，flipd = NO 和 flipped = YES）。

#### drawsAsynchronously
&emsp;一个布尔值，指示是否在后台线程中延迟和异步处理绘制命令。
```c++
@property BOOL drawsAsynchronously API_AVAILABLE(macos(10.8), ios(6.0), watchos(2.0), tvos(9.0));
```
&emsp;当此属性设置为 YES 时，用于绘制图层内容的图形上下文将对绘制命令进行排队，并在后台线程上执行这些命令，而不是同步执行这些命令。异步执行这些命令可以提高某些应用程序的性能。但是，在启用此功能之前，你应该始终衡量实际的性能优势。

&emsp;此属性的默认值为 NO。

> &emsp;如果为 true，则传递给 `- drawInContext:` 方法的 CGContext 对象可以将提交给它的绘图命令排入队列，以便稍后执行它们（即，与 `- drawInContext:` 方法的执行异步）。这可以允许该层比同步执行时更快地完成其绘制操作。默认值为 NO。

#### shouldRasterize
&emsp;一个布尔值，指示在合成之前是否将图层渲染为位图。可动画的
```c++
@property BOOL shouldRasterize;
```
&emsp;当此属性的值为 YES 时，层将在其局部坐标空间中渲染为位图，然后与任何其他内容合成到目标。阴影效果和 filters 属性中的任何过滤器都将光栅化并包含在位图中。但是，层的当前不透明度未光栅化。如果光栅化位图在合成过程中需要缩放，则会根据需要应用 minificationFilter 和 magnificationFilter 属性中的过滤器。

&emsp;如果此属性的值为 NO，则在可能的情况下将图层直接复合到目标中。如果合成模型的某些功能（例如包含滤镜）需要，则在合成之前仍可以对图层进行栅格化。

&emsp;此属性的默认值为 NO。
#### rasterizationScale
&emsp;相对于图层的坐标空间栅格化内容的比例。可动画的。
```c++
@property CGFloat rasterizationScale;
```
&emsp;当 shouldRasterize 属性中的值为 YES 时，图层将使用此属性来确定是否缩放栅格化的内容（以及缩放多少）。此属性的默认值为 1.0，这表示应以当前大小对其进行栅格化。较大的值将放大内容，较小的值将缩小内容。
#### contentsFormat
&emsp;有关所需的图层内容存储格式的提示。
```c++
@property(copy) CALayerContentsFormat contentsFormat API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0));
```
&emsp;此属性的默认值为 kCAContentsFormatRGBA8Uint。

&emsp;UIView 和层备份 NSView 对象可能会将值更改为适合当前设备的格式。

> &emsp;`- drawLayerInContext` 提供的层内容的所需存储格式的提示。默认为 kCAContentsFormatRGBA8Uint。注意，这不会直接影响对 contents 属性的解释。

#### - renderInContext:
&emsp;将图层及其子图层渲染​​到指定的上下文中。
```c++
- (void)renderInContext:(CGContextRef)ctx;
```
&emsp;`ctx`: 用于渲染图层的图形上下文。

&emsp;此方法直接从图层树进行渲染，而忽略添加到渲染树的所有动画。在图层的坐标空间中渲染。

> &emsp;Important: 此方法的 OS X v10.5 实现不支持整个 Core Animation 合成模型。不呈现 QCCompositionLayer、CAOpenGLLayer 和 QTMovieLayer 图层。此外，不会渲染使用 3D 变换的图层，也不会渲染指定 backgroundFilters、filters、compositingFilter 或 mask 值的图层。未来的 macOS 版本可能会增加对渲染这些图层和属性的支持。

### Modifying the Layer Geometry（修改图层 Geometry）
#### frame
&emsp;图层的 frame 矩形。
```c++
@property CGRect frame;
```
&emsp;frame 矩形是在 superlayer 坐标空间中指定的层的位置和大小。对于图层，frame 矩形是从 bounds、anchorPoint 和 position 属性中的值派生的计算属性。为该属性分配新值时，图层将更改其 position 和 bounds 属性以匹配你指定的矩形。矩形中每个坐标的值以点为单位。

&emsp;如果 transform 属性应用的旋转变换不是 90 度的倍数，则不要设置 frame。

&emsp;有关 frame、bounds、anchorPoint 和 position 属性之间的关系的更多信息，请参见 Core Animation Programming Guide。

> &emsp;Note: frame 属性不能直接设置动画。相反，你应该为 bounds、anchorPoint 和 position 属性的适当组合设置动画，以实现所需的结果。

> &emsp;与 NSView 不同，层次结构中的每个图层都有一个隐式 frame 矩形，它是 position、bounds、anchorPoint 和 transform 属性的函数。设置 frame 时，将更改 position 和 bounds.size 以匹配给定的 frame 的值。

#### bounds
&emsp;图层的边界矩形。可动画的。
```c++
@property CGRect bounds;
```
&emsp;边界矩形是图层在其自身坐标空间中的原点和大小。创建新的独立图层时，此属性的默认值为一个空矩形，你必须在使用该图层之前对其进行更改。矩形中每个坐标的值以点为单位。

> &emsp;图层的边界。默认为 CGRectZero。可动画的。

#### position
&emsp;图层在其 superlayer 坐标空间中的位置。可动画的。
```c++
@property CGPoint position;
```
&emsp;此属性的值以磅为单位指定，并且始终相对于 anchorPoint 属性中的值指定。对于新的独立图层，默认位置设置为（0.0，0.0）。更改 frame 属性也会更新此属性中的值。

> &emsp;层边界的定位点 rect 对准的 superlayer 位置。默认为零。可动画的。

#### zPosition
&emsp;图层在 z 轴上的位置。可动画的。
```c++
@property CGFloat zPosition;
```
&emsp;该属性的默认值为 0。更改此属性的值将更改屏幕上各图层的前后顺序。较高的值比较低的值在视觉上更靠近该图层。这会影响 frame 矩形重叠的图层的可见性。

&emsp;此属性的值以点为单位。此属性的范围是单精度浮点 `-greatestFiniteMagnitude` 到 `greatFiniteMagnitude`。

> &emsp;图层在其 superlayer 中位置的 Z 分量。默认为 zero。可动画的。

#### anchorPointZ
&emsp;图层沿 z 轴位置的锚点。可动画的。
```c++
@property CGFloat anchorPointZ;
```
&emsp;此属性指定围绕 z 轴进行几何操作的锚点。该点表示为沿 z 轴的距离（以点为单位）。此属性的默认值为 0。

> &emsp;图层锚点（即位置和变换的参考点）的 Z 分量。默认为 zero。可动画的。

#### anchorPoint
&emsp;定义图层边界矩形的锚点。可动画的。
```c++
@property CGPoint anchorPoint;
```
&emsp;你可以使用单位坐标空间为此属性指定值。此属性的默认值为（0.5，0.5），它表示图层边界矩形的中心。视图的所有几何操作都在指定点附近发生。例如，对具有默认锚点的图层应用旋转变换会导致该图层绕其中心旋转。将锚点更改到其他位置将导致图层围绕该新点旋转。
#### contentsScale
&emsp;应用于图层的比例因子。
```c++
@property CGFloat contentsScale API_AVAILABLE(macos(10.7), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;此值定义层的逻辑坐标空间（以点为单位）和物理坐标空间（以像素为单位）之间的映射。比例因子越高，表示渲染时该层中的每个点都由一个以上的像素表示。例如，如果比例因子为 2.0，并且图层边界为 50 x 50 点，则用于显示图层内容的位图大小为 100 x 100 像素。

&emsp;此属性的默认值为 1.0。对于附加到视图的图层，视图将比例因子自动更改为适合当前屏幕的值。对于你自己创建和管理的图层，你必须根据屏幕的分辨率和所提供的内容自行设置此属性的值。 Core Animation 使用你指定的值作为提示来确定如何呈现内容。

> &emsp;定义应用于图层内容的比例因子。如果内容的物理大小为（w，h），则逻辑大小（即用于 contentGravity 计算）定义为（w / contentsScale，h / contentsScale）。适用于显式提供的图像和通过 `- drawInContext:` 提供的内容（即，如果 contentScale 为 two，则 `- drawInContext:` 将绘制为两倍于图层边界的缓冲区）。默认为 1。可动画的。

### Managing the Layer’s Transform（管理图层的转换（旋转））
#### transform
&emsp;转换应用于图层的内容。可动画的。
```c++
@property CATransform3D transform;
```
&emsp;默认情况下，此属性设置为标识转换。你应用于图层的所有变换都相对于图层的锚点进行。

> &emsp;相对于其 bounds rect 的锚点应用于图层的变换。默认为 identity 转换。可动画的。

#### sublayerTransform
&emsp;指定在渲染时应用于子层的变换。可动画的。
```c++
@property CATransform3D sublayerTransform;
```
&emsp;通常，你可以使用此属性为嵌入的图层添加 perspective 和其他 view 效果。你可以通过将子层变换设置为所需的投影矩阵（projection matrix）来添加 perspective。此属性的默认值为 identity 转换。

> &emsp;在将其内容呈现到 CALayer 的输出中时，将变换应用于 sublayers 数组的每个成员。通常用作 projection matrix，以将透视图和其他查看效果添加到模型中。默认为 identity。可动画的。

#### - affineTransform
&emsp;返回图层变换的仿射版本。
```c++
- (CGAffineTransform)affineTransform;
```
&emsp;仿射变换结构，对应于图层的 transform 属性中的值。

> &emsp;作为仿射变换访问 transform 属性的便捷方法。

#### - setAffineTransform:
&emsp;将图层的变换设置为指定的仿射变换。
```c++
- (void)setAffineTransform:(CGAffineTransform)m;
```
&emsp;`m`: 仿射变换，用于图层的变换。
### Managing the Layer Hierarchy（管理 CALayer 层次结构）
#### sublayers
&emsp;包含图层子图层的数组。
```c++
@property(nullable, copy) NSArray<__kindof CALayer *> *sublayers;
```
&emsp;子层以从前到后的顺序列出。此属性的默认值为 nil。

&emsp;将 sublayers 属性设置为填充有图层对象的数组时，该数组中的每个图层都必须尚未具有 superlayer -也就是说，其 superlayer 属性当前必须为 nil。

> &emsp;此 CALayer 的 sublayers 数组。层以从后到前的顺序列出。默认为 零。设置属性的值时，任何新添加的层都必须具有 nil superlayer，否则行为是不确定的。请注意，不能保证返回的数组 retain 其元素。

#### superlayer
&emsp;层的 superlayer。
```c++
@property(nullable, readonly) CALayer *superlayer;
```
&emsp;superlayer 管理其子层的布局。

#### - addSublayer:
&emsp;将图层添加到图层的子图层列表中。
```c++
- (void)addSublayer:(CALayer *)layer;
```
&emsp;`layer`: 要添加的层。

&emsp;如果 sublayers 属性中的数组为 nil，则调用此方法将为该属性创建一个数组，并将指定的图层添加到该数组。

> &emsp;在 CALayer 的子层数组的末尾添加 CALayer。如果 CALayer 已经具有 superlayer，则将其删除后再添加。（更改其 superlayer）

#### - removeFromSuperlayer
&emsp;从其父层移除该层。
```c++
- (void)removeFromSuperlayer;
```
&emsp;你可以使用此方法从图层层次结构中删除图层（及其所有子图层）。此方法会同时更新 superlayer 的子图层列表，并将该图层的 superlayer 属性设置为 nil。

> &emsp;从 CALayer 的 superlayer 移除该层，如果接收者位于其 superlayer 的 sublayers 数组中或设置为 mask 值，则两者均可工作。（CALayer 的 mask 也是作为该 CALayer 的子层存在的。）

#### - insertSublayer:atIndex:
&emsp;将指定的图层插入到 CALayer 的 sublayers 列表的指定索引处。
```c++
- (void)insertSublayer:(CALayer *)layer atIndex:(unsigned)idx;
```
&emsp;`layer`: 要插入当前层的子层。`idx`: 插入图层的索引。此值必须是 sublayers 数组中基于 0 的有效索引。

> &emsp;在 CALayer 的 sublayers 中的 idx 位置插入 layer 。如果 layer 已经具有 superlayer，则在插入之前将其删除。

#### - insertSublayer:below:
&emsp;将指定的子层插入已经属于 CALayer 的另一个子层下。
```c++
- (void)insertSublayer:(CALayer *)layer below:(nullable CALayer *)sibling;
```
&emsp;`layer`: 要插入当前层的子层。`sibling`: 当前层中的现有子层。图层中的图层在子图层阵列中插入到该图层的前面，因此在外观上看起来是在其后面。

&emsp;如果 sibling 不在接收者的子图层数组中，则此方法会引发异常。

#### - insertSublayer:above:
&emsp;将指定的子层插入到已经属于接收方的另一个子层之上。
```c++
- (void)insertSublayer:(CALayer *)layer above:(nullable CALayer *)sibling;
```
&emsp;`layer`: 要插入当前层的子层。`sibling`: 当前层中的现有子层。图层中的图层插入到子图层阵列中的该图层之后，因此从视觉上显示在其前面。
#### - replaceSublayer:with:
&emsp;将指定的子图层替换为其他图层对象。
```c++
- (void)replaceSublayer:(CALayer *)oldLayer with:(CALayer *)newLayer;
```
&emsp;`oldLayer`: 要替换的层。`newLayer`: 用来替换 oldLayer 的图层。

&emsp;如果 oldLayer 不在接收者的子图层数组中，则此方法的行为是不确定的。

> &emsp;从 CALayer 的 sublayers 中删除 oldLayer，并在其位置非零时插入 newLayer。如果 oldLayer 的 superlayer 不是接收者，则行为是不确定的。

### Updating Layer Display（更新 CALayer 显示）
#### - setNeedsDisplay
&emsp;将图层的内容标记为需要更新。
```c++
- (void)setNeedsDisplay;
```
&emsp;调用此方法将导致图层重新缓存其内容。这导致该层可能调用其 delegate 的 `displayLayer:` 或 `drawLayer:inContext:` 方法。图层的 contents 属性中的现有内容将被删除，以便为新内容腾出空间。

> &emsp;标记 `- display` 需要在下一次提交层之前被调用。如果指定了区域，则仅该层的该区域无效。

#### - setNeedsDisplayInRect:
&emsp;将指定矩形内的区域标记为需要更新。
```c++
- (void)setNeedsDisplayInRect:(CGRect)r;
```
&emsp;`r`: 标记为无效的图层的矩形区域。你必须在图层自己的坐标系中指定此矩形。
#### needsDisplayOnBoundsChange
&emsp;一个布尔值，指示当其 bounds 矩形更改时是否必须更新图层内容。
```c++
@property BOOL needsDisplayOnBoundsChange;
```
> &emsp;如果为 true，则在更改图层 bounds 时将自动调用 `- setNeedsDisplay`。默认值为 NO。
#### - displayIfNeeded
&emsp;如果图层当前被标记为需要更新，则启动该图层的更新过程。
```c++
- (void)displayIfNeeded;
```
&emsp;你可以根据需要调用此方法，以在正常更新周期之外强制对图层内容进行更新。但是，通常不需要这样做。更新图层的首选方法是调用 `- setNeedsDisplay`，并让系统在下一个周期更新图层。

> &emsp;如果接收方被标记为需要重绘，则调用 `- display`。

#### - needsDisplay
&emsp;返回一个布尔值，指示该图层是否已标记为需要更新。
```c++
- (BOOL)needsDisplay;
```
&emsp;YES，如果需要更新图层。

> &emsp;将图层标记为需要重绘时，返回 YES。

#### + needsDisplayForKey:
&emsp;返回一个布尔值，指示对指定 key 的更改是否需要重新显示该图层。
```c++
+ (BOOL)needsDisplayForKey:(NSString *)key;
```
&emsp;`key`: 一个字符串，它指定图层的属性。

&emsp;Return Value: 如果该图层需要重新显示，则为 YES。

&emsp;子类可以重写此方法，如果在指定属性的值更改时应重新显示该图层，则返回 YES。更改属性值的动画也会触发重新显示。

&emsp;此方法的默认实现返回 NO。

> &emsp;子类重写的方法。对于给定的属性，返回 YES 会导致更改属性时（包括通过附加到该图层的动画进行更改时）重绘该图层的内容。默认实现返回 NO。子类应为超类定义的属性调用超类。（例如，不要尝试对 CALayer 实现的属性返回YES，这样做会产生不确定的结果。）

### Layer Animations（CALayer 动画）
#### - addAnimation:forKey:
&emsp;将指定的动画对象添加到图层的渲染树（render tree）。（目前为止已经见到过 "表示树"、"模型树"、"渲染树"、"层级树"，在 Core Animation 文档里面都能得到解释）
```c++
- (void)addAnimation:(CAAnimation *)anim forKey:(nullable NSString *)key;
```
&emsp;`anim`: 要添加到渲染树的动画。该对象由渲染树复制，不引用（not referenced）。因此，对动画对象的后续修改不会传播到渲染树中。`key`: 标识动画的字符串。每个唯一键仅将一个动画添加到该层。特殊键 kCATransition 自动用于过渡动画。你可以为此参数指定 nil。

&emsp;如果动画的 duration 属性为零或负，则将 duration 更改为 kCATransactionAnimationDuration 事务属性的当前值（如果已设置）或默认值为 0.25 秒。

> &emsp;将动画对象附加到图层。通常，这是通过作为 CAAnimation 对象的 action 隐式调用的。（CAAnimation 遵循 CAAction 协议）

 > &emsp;key 可以是任何字符串，因此每个唯一 key 每个图层仅添加一个动画。特殊键 transition 会自动用于过渡动画（transition animations）。 nil 指针也是有效的键。

> &emsp;如果动画的 duration 属性为零或负数，则指定默认持续时间，否则为 animationDuration transaction 属性的值，否则为 0.25 秒。

> &emsp;在将动画添加到图层之前先对其进行复制，因此，除非对动画进行任何后续修改，否则将其添加到另一层都不会产生影响。

#### - animationForKey:
&emsp;返回具有指定标识符的动画对象。
```c++
- (nullable __kindof CAAnimation *)animationForKey:(NSString *)key;
```
&emsp;`key`: 一个字符串，指定动画的标识符。该字符串对应于你传递给 `- addAnimation:forKey:` 方法的标识符字符串。

&emsp;Return Value: 匹配标识符的动画对象；如果不存在这样的动画，则为 nil。

&emsp;你可以使用此字符串来检索已经与图层关联的动画对象。但是，你不得修改返回对象的任何属性。这样做将导致不确定的行为。

> &emsp;返回添加到带有标识符 key 的 CALyaer 的动画；如果不存在这样的动画，则返回 nil。尝试修改返回对象的任何属性将导致未定义的行为。

#### - removeAllAnimations
&emsp;删除所有附加到该图层的动画。
```c++
- (void)removeAllAnimations;
```
#### - removeAnimationForKey:
&emsp;使用指定的 key 删除动画对象。
```c++
- (void)removeAnimationForKey:(NSString *)key;
```
&emsp;`key`: 要删除的动画的标识符。
#### - animationKeys
&emsp;返回一个字符串数组，这些字符串标识当前附加到该图层的动画。
```c++
- (nullable NSArray<NSString *> *)animationKeys;
```
&emsp;Return Value: 标识当前动画的 NSString 对象数组。

&emsp;数组的顺序与将动画应用于图层的顺序匹配。

&emsp;可看到 layout 和 display 的一组方法的使用方式和命名方式基本相同。`- setNeedsDisplay/- setNeedsLayout` 标记在下一个周期需要进行 display/layout，`- displayIfNeeded/- layoutIfNeeded` 如果需要则立即执行 display/layout，`- needsDisplay/- needsLayout` 返回是否需要 display/layout，`- display/- layoutSublayers` 更新执行 display/layout。

### Managing Layer Resizing and Layout（管理图层调整大小和布局）
#### layoutManager
&emsp;负责管理图层的子图层布局的对象。
```c++
@property(strong) id<CALayoutManager> layoutManager;
```
&emsp;你分配给此属性的对象必须名义上实现 CALayoutManager 非正式协议非正式协议。如果图层的 delegate 不处理布局更新，则分配给此属性的对象将有机会更新图层的子图层的布局。

&emsp;在 macOS 中，如果你的图层使用 layer-based 的约束来处理布局更改，则将 CAConstraintLayoutManager 类的实例分配给此属性。

&emsp;此属性的默认值为nil。
#### - setNeedsLayout
&emsp;使图层的布局无效，并将其标记为需要更新。
```c++
- (void)setNeedsLayout;
```
&emsp;你可以调用此方法来指示图层的子图层的布局已更改，必须进行更新。通常，在更改图层 bounds 或添加或删除子图层时，系统会自动调用此方法。在 macOS 中，如果你图层的 layoutManager 属性包含一个实现 `invalidateLayoutOfLayer:` 方法的对象，则也将调用该方法。

&emsp;在下一个更新周期中，系统将调用需要布局更新的任何图层的 `- layoutSublayers` 方法。

> &emsp;只要修改了 sublayers 或 layoutManager 属性，便会在该层上自动调用此方法，并且只要修改其 bounds 或 transform 属性，便会在该层及其 superlayer 上自动调用此方法。如果图层当前正在执行其 `- layoutSublayers` 方法，则将跳过对 `- setNeedsLayout` 的隐式调用。

#### - layoutSublayers
&emsp;告诉图层更新其布局。
```c++
- (void)layoutSublayers;
```
&emsp;子类可以重写此方法，并使用它来实现自己的布局算法。你的实现必须设置由 CALayer 管理的每个子层的 frame。

&emsp;此方法的默认实现调用该图层的 delegate 的 `layoutSublayersOfLayer:` 方法。如果没有 delegate 对象，或者 delegate 没有实现该方法，则此方法在 layoutManager 属性中调用对象的 `layoutSublayersOfLayer:` 方法。

> &emsp;在图层需要布局时调用。默认实现会调用布局管理器（如果存在的话），并且会实现 `- layoutSublayersOfLayer:` 方法。子类可以重写此方法以提供自己的布局算法，该算法应设置每个子层的 frame。

#### - layoutIfNeeded
&emsp;如果需要，请重新计算 CALayer 的布局。
```c++
- (void)layoutIfNeeded;
```
&emsp;收到此消息后，将遍历图层的 super layers，直到找到不需要布局的祖先图层（沿着 superlayer 链一直找到不需要布局的 CALayer）。然后在该祖先下的整个层树上执行布局。

> &emsp;从图层向上遍历，而 superlayer 需要布局。然后将整个树布局在该祖先下。

#### - needsLayout
&emsp;返回一个布尔值，指示是否已将图层标记为需要布局更新
```c++
- (BOOL)needsLayout;
```
&emsp;如果已将图层标记为需要布局更新，则为 YES。

#### autoresizingMask
&emsp;一个位掩码，用于定义当其 superlayer  bounds 更改时如何调整其大小。
```c++
@property CAAutoresizingMask autoresizingMask;
```
&emsp;如果你的应用未使用布局管理器或约束来处理布局更改，则可以为该属性分配一个值，以响应 superlayer bounds 的更改来调整图层的大小。有关可能值的列表，请参见 CAAutoresizingMask。

&emsp;此属性的默认值为 kCALayerNotSizable。
#### - resizeWithOldSuperlayerSize:
&emsp;通知 CALayer 其 superlayer 大小已更改。
```c++
- (void)resizeWithOldSuperlayerSize:(CGSize)size;
```
&emsp;`size`: superlayer 的先前大小。

&emsp;当 autoresizingMask 属性用于调整大小并且层的 bounds 更改时，该层在其每个子层上调用此方法。子层使用此方法调整自己的 frame 矩形以反映新的 superlayer bounds，可以直接从 superlayer 检索。superlayer 的旧大小被传递给这个方法，这样子层就有了它必须进行的任何计算所需的信息。
#### - resizeSublayersWithOldSize:
&emsp;通知 CALayer 的子层接收者的尺寸已更改。
```c++
- (void)resizeSublayersWithOldSize:(CGSize)size;
```
&emsp;`size`: 当前 CALayer 的先前大小。

&emsp;当将 autoresizingMask 属性用于调整大小并且此层的 bounds 发生变化时，该层将调用此方法。默认实现会调用每个子层的 `resizeWithOldSuperlayerSize:` 方法，以使其知道其 superlayer 的 bounds 已更改。你不需要直接调用或重写此方法。
#### - preferredFrameSize
&emsp;返回其 superlayer 坐标空间中该层的首选大小。
```c++
- (CGSize)preferredFrameSize;
```
&emsp;Return Value: 图层的首选 frame 大小。

&emsp;在 macOS 中，此方法的默认实现调用其布局管理器的 `- preferredSizeOfLayer:` 方法，即 layoutManager 属性中的对象。如果该对象不存在或未实现该方法，则此方法返回映射到其 superlayer 坐标空间的层当前 bounds 矩形的大小。
### Managing Layer Constraints（管理 CALayer 约束）
#### constraints
&emsp;用于定位当前图层的子图层的约束。
```c++
@property(copy) NSArray<CAConstraint *> *constraints;
```
&emsp;macOS 应用程序可以使用此属性来访问其 layer-based 的约束。在应用约束之前，还必须将 CAConstraintLayoutManager 对象分配给图层的 layoutManager 属性。

&emsp;iOS 应用程序不支持基于图层的约束。
#### - addConstraint:
&emsp;将指定的约束添加到图层。
```c++
- (void)addConstraint:(CAConstraint *)c;
```
&emsp;`c`: 约束对象添加到 CALayer 的约束对象数组中。

&emsp;在 macOS 中，通常向层添加约束以管理该层子层的大小和位置。在应用约束之前，还必须将 CAConstraintLayoutManager 对象指定给层的 layoutManager 属性。有关管理基于层的约束的详细信息，请参见  Core Animation Programming Guide.。

&emsp;iOS 应用程序不支持基于图层的约束。
### Getting the Layer’s Actions（获取 CALayer 的 action）
#### - actionForKey:
&emsp;返回分配给指定 key 的 action 对象。
```c++
- (nullable id<CAAction>)actionForKey:(NSString *)event;
```
&emsp;`event`: action 的标识符。

&emsp;Return Value: 返回提供键操作的对象。该对象必须实现 CAAction 协议。

&emsp;此方法搜索层的给定 action 对象。action 定义层的动态行为。例如，层的可设置动画的属性通常具有相应的 action 对象来启动实际动画。当该属性更改时，层将查找与属性名称关联的 action 对象并执行它。还可以将自定义 action 对象与层关联，以实现特定于应用程序的动作。

> &emsp;返回与由字符串 event 关联的 action 对象。默认实现在以下位置搜索 action 对象：
> 
> 1. if defined, call the delegate method -actionForLayer:forKey:（如果已实现，则调用 delegate 方法 `- actionForLayer:forKey:`）
> 2. look in the layer's 'actions' dictionary（查看一下 CALayer 的 actions dictionaries）
> 3. look in any 'actions' dictionaries in the 'style' hierarchy（查看 style 层次结构中的所有 actions dictionaries）
> 4. call +defaultActionForKey: on the layer's class（在 layer 的类上调用 `+ defaultActionForKey:` 方法）
> 
> &emsp;如果这些步骤中的任何一个导致非空 action 对象，则以下步骤将被忽略。如果最终结果是 NSNull 的实例，则将其转换为 nil。

#### actions
&emsp;包含图层 action 的字典。
```c++
@property(nullable, copy) NSDictionary<NSString *, id<CAAction>> *actions;
```
&emsp;此属性的默认值为 nil。你可以使用此字典存储图层的自定义 action。搜索该 dictionary 的内容，作为 `- actionForKey:` 方法的标准实现的一部分。

> &emsp;dictionary 将 key 映射到实现 CAAction 协议的对象。默认值为 nil。

#### + defaultActionForKey:
&emsp;返回当前类的默认 action 对象。
```c++
+ (nullable id<CAAction>)defaultActionForKey:(NSString *)event;
```
&emsp;`event`: action 的标识符。

&emsp;Return Value: 返回给定 enent 的合适 action 对象，或者没有与该 event 关联的 action 对象时返回 nil。

&emsp;想要提供默认 action 的类可以重写此方法，并使用它返回那些 action。

/* An "action" is an object that responds to an "event" via the CAAction protocol (see below). Events are named using standard dot-separated key paths. Each layer defines a mapping from event key paths to action objects. Events are posted by looking up the action object associated with the key path and sending it the method defined by the CAAction protocol.

> &emsp;action 是通过 CAAction 协议响应 event 字符串的对象。使用标准的点分隔 key path 来命名事件。每一层都定义了从 event key path 到 action 对象的映射。通过查找与 key path 关联的 action 对象并向其发送 CAAction 协议定义的方法，可以发布事件（执行 action）。
> &emsp;调用 action 对象时，它会接收三个参数：命名事件的键路径，发生事件的对象（即图层）以及可选的特定于每个事件的命名参数字典。（即 CAAction 协议中的唯一一个协议函数：`- (void)runActionForKey:(NSString *)event object:(id)anObject arguments:(nullable NSDictionary *)dict;`）
> &emsp;为了给图层属性提供隐式动画，只要属性值被修改，就会发布一个与每个属性同名的事件。默认情况下，合适的 CAAnimation 对象与每个隐式事件关联（CAAnimation 实现动作协议）。
> 图层类还定义了以下未直接链接到属性的事件： 
> &emsp;onOrderIn
> &emsp;Invoked when the layer is made visible, i.e. either its superlayer becomes visible, or it's added as a sublayer of a visible layer
> &emsp;当该图层变为可见时调用，即该图层的 superlayer 变为可见，或将其添加为可见层的子层。
> &emsp;onOrderOut
> &emsp;Invoked when the layer becomes non-visible. 当图层变为不可见时调用。
> &emsp;返回与字符串 event 命名的事件关联的默认 action 对象。默认实现为可动画属性发布的事件返回合适的动画对象，否则为 nil。

### Mapping Between Coordinate and Time Spaces（坐标与时间空间的映射）
#### - convertPoint:fromLayer:
&emsp;将点从指定图层的坐标系转换为 receiver 的坐标系。
```c++
- (CGPoint)convertPoint:(CGPoint)p fromLayer:(nullable CALayer *)l;
```
&emsp;`p`: 指定l坐标系中位置的点。`l`: 在其坐标系中具有p的层。接收者和l和必须共享一个公共父层。此参数可以为nil。

&emsp;Return Value: 该点将转换为接收者的坐标系。

&emsp;如果为l参数指定nil，则此方法返回从图层帧的原点减去的原始点。

#### - convertPoint:toLayer:
&emsp;将点从接收者的坐标系转换为指定图层的坐标系。
```c++
- (CGPoint)convertPoint:(CGPoint)p toLayer:(nullable CALayer *)l;
```
&emsp;`p`: 指定 l 坐标系中位置的点。`l`: 要将坐标系 p 转换为的图层。接收者和 l 必须共享一个公共父层。此参数可以为 nil。

&emsp;Return Value: 点转换为图层的坐标系。

&emsp;如果为 l 参数指定 nil，则此方法返回添加到图层 frame 原点的原始点。
#### - convertRect:fromLayer:
&emsp;将矩形从指定图层的坐标系转换为接收者的坐标系。
```c++
- (CGRect)convertRect:(CGRect)r fromLayer:(nullable CALayer *)l;
```
&emsp;`r`: 指定 l 坐标系中位置的 CGRect。`l`: 在其坐标系中具有 r 的图层。接收者和 l 和必须共享一个公共父层。此参数可以为 nil。

&emsp;Return Value: 矩形将转换为接收者的坐标系。

&emsp;如果为 l 参数指定 nil，则此方法将返回原始 rect，其原点将从图层 frame 的原点中减去。
#### - convertRect:toLayer:
&emsp;将矩形从接收者的坐标系转换为指定图层的坐标系。
```c++
- (CGRect)convertRect:(CGRect)r toLayer:(nullable CALayer *)l;
```
&emsp;`r`: 指定 l 坐标系中位置的点。`l`: 要转换其坐标系 r 的图层。接收者和 l 必须共享一个公共父层。此参数可以为 nil。

&emsp;Return Value: 矩形转换为 l 的坐标系。

&emsp;如果为 l 参数指定 nil，则此方法将返回原始 rect，并将其原点添加到图层 frame 的原点。
#### - convertTime:fromLayer:
&emsp;将时间间隔从指定层的时间空间转换为接收者的时间空间。
```c++
- (CFTimeInterval)convertTime:(CFTimeInterval)t fromLayer:(nullable CALayer *)l;
```
&emsp;`t`: 将时间间隔从指定层的时间空间转换为接收者的时间空间。`l`: 时空为 t 的图层。接收者和 l 必须共享一个公共父层。

&emsp;Return Value: 时间间隔转换为接收者的时间空间。
#### - convertTime:toLayer:
&emsp;将时间间隔从接收者的时间空间转换为指定层的时间空间
```c++
- (CFTimeInterval)convertTime:(CFTimeInterval)t toLayer:(nullable CALayer *)l;
```
&emsp;`t`: 指定 l 坐标系中位置的点。`l`: 要将时间空间 t 转换为该层。接收者和 l 和必须共享一个公共父层。

&emsp;时间间隔转换为图层的时间空间。
### Hit Testing（命中测试）
#### - hitTest:
&emsp;返回包含指定点的图层层次结构中接收者的最远后代（包括自身）。（看来 iOS 中 UIView 的 Hit-Testing 内部完全是靠 CALayer 的 Hit-Testing 实现的）
```c++
- (nullable __kindof CALayer *)hitTest:(CGPoint)p;
```
&emsp;`p`: 接收者的 superlayer 坐标系中的一点。

&emsp;Return Value: 包含 thePoint 的图层；如果该点位于接收者的 bounds 矩形之外，则为 nil。

/* Returns the farthest descendant of the layer containing point 'p'. Siblings are searched in top-to-bottom order. 'p' is defined to be in the coordinate space of the receiver's nearest ancestor that isn't a CATransformLayer (transform layers don't have a 2D coordinate space in which the point could be specified). */
> &emsp;返回包含点 p 的层的最远后代。兄弟姐妹以自上而下的顺序搜索。p 被定义为位于接收者的最近祖先的坐标空间中，该坐标空间不是 CATransformLayer（转换层没有可以在其中指定点的 2D 坐标空间）。

#### - containsPoint:
&emsp;返回接收者是否包含指定点。
```c++
- (BOOL)containsPoint:(CGPoint)p;
```
&emsp;`p`: 接收者坐标系中的一个点。
### Scrolling（滚动）
#### visibleRect
&emsp;图层在其自己的坐标空间中的可见区域。
```c++
@property(readonly) CGRect visibleRect;
```
&emsp;可见区域是未被包含的滚动层剪切的区域。
#### - scrollPoint:
&emsp;在该层的最接近的祖先滚动层中启动滚动，以使指定点位于滚动层的原点。
```c++
- (void)scrollPoint:(CGPoint)p;
```
&emsp;`p`: 当前图层中应滚动到位置的点。

&emsp;如果 CAScrollLayer 对象不包含该图层，则此方法不执行任何操作。
#### - scrollRectToVisible:
&emsp;在该图层的最接近的祖先滚动图层中启动滚动，以使指定的矩形变为可见。
```c++
- (void)scrollRectToVisible:(CGRect)r;
```
&emsp;`r`: 要显示的矩形。

&emsp;如果 CAScrollLayer 对象不包含该图层，则此方法不执行任何操作。
### Identifying the Layer（标识 CALyaer）
#### name
&emsp;接收者的名字。
```c++
@property(nullable, copy) NSString *name;
```
&emsp;某些布局管理器使用图层名称来标识图层。此属性的默认值为 nil。
### Key-Value Coding Extensions（键值编码扩展）
#### - shouldArchiveValueForKey:
&emsp;返回一个布尔值，指示是否应归档指定键的值。
```c++
- (BOOL)shouldArchiveValueForKey:(NSString *)key;
```
&emsp;`key`: 接收者属性之一的名称。

&emsp;Return Value: 如果应将指定的属性归档，则为 YES；否则，则为 NO。

&emsp;默认实现返回 YES。

> &emsp;由对象的 `- encodeWithCoder:` 的实现调用，如果不应 archived 命名属性，则返回 false。基本实现返回 YES。子类应为未知属性调用 super。

#### + defaultValueForKey:
&emsp;指定与指定键关联的默认值。
```c++
+ (nullable id)defaultValueForKey:(NSString *)key;
```
&emsp;`key`: 接收者属性之一的名称。

&emsp;Return Value: 命名属性的默认值。如果未设置默认值，则返回 nil。

&emsp;如果为图层定义自定义属性，但未设置值，则此方法将基于键的预期值返回适当的 zero 默认值。例如，如果 key 的值是 CGSize 结构，则该方法返回一个包含（0.0,0.0）的大小结构，该结构封装在 NSValue 对象中。对于 CGRect，返回一个空矩形。对于 CGAffineTransform 和 CATransform3D，将返回相应的单位矩阵。

&emsp;如果 key 对于该类的属性未知，则该方法的结果不确定。

/* CALayer implements the standard NSKeyValueCoding protocol for all Objective C properties defined by the class and its subclasses. It dynamically implements missing accessor methods for properties declared by subclasses.

&emsp;CALayer 为该类及其子类定义的所有 Objective-C 属性实现标准的 NSKeyValueCoding 协议。它为子类声明的属性动态实现缺少的访问器方法。
 
When accessing properties via KVC whose values are not objects, the standard KVC wrapping conventions are used, with extensions to support the following types:

&emsp;通过值不是对象的 KVC 访问属性时，将使用标准的 KVC 包装约定，并带有扩展以支持以下类型：

| C Type | Class |
| --- | --- |
| CGPoint | NSValue |
| CGSize | NSValue |
| CGRect | NSValue |
| CGAffineTransform | NSValue |
| CATransform3D | NSValue | 

&emsp;返回指定属性的默认值，如果没有默认值，则返回 nil。重写此方法为自己的属性定义默认值的子类应为未知属性调用 super。

### Constants（常量）
#### CAAutoresizingMask
&emsp;这些常量由 autoresizingMask 属性使用。
```c++
typedef enum CAAutoresizingMask : unsigned int {
    ...
} CAAutoresizingMask;
```
+ kCALayerNotSizable = 0: 接收器无法调整大小。
+ kCALayerMinXMargin = 1U << 0: 接收者及其 superlayer 之间的左边界是灵活的。
+ kCALayerWidthSizable = 1U << 1: 接收器的宽度很灵活。
+ kCALayerMaxXMargin = 1U << 2: 接收者及其 superlayer 之间的右边距是灵活的。
+ kCALayerMinYMargin = 1U << 3: 接收器及其 superlayer 之间的底部边距很灵活。
+ kCALayerHeightSizable = 1U << 4: 接收器的高度是灵活的。
+ kCALayerMaxYMargin = 1U << 5: 接收者及其 superlayer 之间的上边界是灵活的。

#### Action Identifiers
&emsp;这些常量是预定义 action 标识符用于: `actionForKey:`、`addAnimation:forKey:`、`defaultActionForKey:`、`removeAnimationForKey:`、Layer Filters 和 CAAction 协议的 `runActionForKey:object:arguments:` 方法。
```c++
NSString *const kCAOnOrderIn;
NSString *const kCAOnOrderOut;
NSString *const kCATransition;
```

+ kCAOnOrderIn: 表示当某个图层变为可见时（由于将结果插入可见图层层次结构或将该图层不再设置为隐藏）而采取的 action 的标识符。
+ kCAOnOrderOut: 表示从图层层次结构中删除图层或隐藏图层时所采取的 action。
+ kCATransition: 代表过渡动画的标识符。

#### CAEdgeAntialiasingMask
&emsp;edgeAntialiasingMask 属性使用此掩码。
```c++
typedef NS_OPTIONS (unsigned int, CAEdgeAntialiasingMask)
{
  kCALayerLeftEdge      = 1U << 0,      /* Minimum X edge. */
  kCALayerRightEdge     = 1U << 1,      /* Maximum X edge. */
  kCALayerBottomEdge    = 1U << 2,      /* Minimum Y edge. */
  kCALayerTopEdge       = 1U << 3,      /* Maximum Y edge. */
};
```
+ kCALayerLeftEdge: 指定应该对接收者内容的左边缘进行抗锯齿处理。
+ kCALayerRightEdge: 指定应该对接收者内容的右边缘进行锯齿处理。
+ kCALayerBottomEdge: 指定应该对接收者内容的底部边缘进行锯齿处理。
+ kCALayerTopEdge: 指定应该对接收者内容的上边缘进行抗锯齿处理。

#### Identity Transform
&emsp;定义核心动画使用的身份转换矩阵。
```c++
const CATransform3D CATransform3DIdentity;
```
&emsp;CATransform3DIdentity.The identity transform: [1 0 0 0; 0 1 0 0; 0 0 1 0; 0 0 0 1].
#### Scaling Filters
&emsp;这些常量指定 magnificationFilter 和 minificationFilter 使用的缩放过滤器。
##### kCAFilterLinear
&emsp;线性插值滤波器。
```c++
const CALayerContentsFilter kCAFilterLinear;
```
##### kCAFilterNearest
&emsp;最近邻居插值滤波器。
```c++
const CALayerContentsFilter kCAFilterNearest;
```
##### kCAFilterTrilinear
&emsp;三线性缩小过滤器。启用 mipmap 生成。一些渲染器可能会忽略这一点，或施加其他限制，例如需要二维幂的源图像。
```c++
const CALayerContentsFilter kCAFilterTrilinear;
```
#### CATransform3D
&emsp;整个 Core Animation中 使用的标准转换矩阵。

&emsp;变换矩阵用于旋转，缩放，平移，倾斜和投影图层内容。提供了用于创建，连接和修改 CATransform3D 数据的功能。
```c++
struct CATransform3D
{
  CGFloat m11, m12, m13, m14;
  CGFloat m21, m22, m23, m24;
  CGFloat m31, m32, m33, m34;
  CGFloat m41, m42, m43, m44;
};
```
### Instance Properties
#### cornerCurve
&emsp;定义用于渲染层圆角的曲线。默认是 kCACornerCurveCircular。
```c++
typedef NSString * CALayerCornerCurve NS_TYPED_ENUM;

CA_EXTERN CALayerCornerCurve const kCACornerCurveCircular API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
CA_EXTERN CALayerCornerCurve const kCACornerCurveContinuous API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));

@property(copy) CALayerCornerCurve cornerCurve API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
```
### Type Methods
#### + cornerCurveExpansionFactor:
&emsp;使用特定拐角曲线时应用于圆角边界框大小的扩展比例因子。
```c++
+ (CGFloat)cornerCurveExpansionFactor:(CALayerCornerCurve)curveAPI_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
```

## 参考链接
**参考链接:🔗**
+ [CALayer](https://developer.apple.com/documentation/quartzcore/calayer?language=objc)
+ [Core Animation Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/CoreAnimation_guide/Introduction/Introduction.html#//apple_ref/doc/uid/TP40004514-CH1-SW1)
