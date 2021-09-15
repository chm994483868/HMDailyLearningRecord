# iOS App 启动优化(十三)：chisel 命令列举

&emsp;上篇学习了 LLDB 的一些基础命令，然后也学习了 chisel 的安装以及执行原理，本篇就看一下 chisel 都给我们提供了哪些便捷的命令吧！

```c++
Current user-defined commands:
  alamborder    -- Put a border around views with an ambiguous layout
  alamunborder  -- Removes the border around views with an ambiguous layout
  bdisable      -- Disable a set of breakpoints for a regular expression
  benable       -- Enable a set of breakpoints for a regular expression
  binside       -- Set a breakpoint for a relative address within the
                   framework/library that's currently running. This does the
                   work of finding the offset for the framework/library and
                   sliding your address accordingly.
  bmessage      -- Set a breakpoint for a selector on a class, even if the
                   class itself doesn't override that selector. It walks the
                   hierarchy until it finds a class that does implement the
                   selector and sets a conditional breakpoint there.
  border        -- Draws a border around <viewOrLayer>. Color and width can be
                   optionally provided. Additionally depth can be provided in
                   order to recursively border subviews.
  caflush       -- Force Core Animation to flush. This will 'repaint' the UI
                   but also may mess with ongoing animations.
  copy          -- Copy data to your Mac.
  dcomponents   -- Set debugging options for components.
  dismiss       -- Dismiss a presented view controller.
  fa11y         -- Find the views whose accessibility labels match labelRegex
                   and puts the address of the first result on the clipboard.
  findinstances -- Find instances of specified ObjC classes.
  flicker       -- Quickly show and hide a view to quickly help visualize where
                   it is.
  fv            -- Find the views whose class names match classNameRegex and
                   puts the address of first on the clipboard.
  fvc           -- Find the view controllers whose class names match
                   classNameRegex and puts the address of first on the
                   clipboard.
  heapfrom      -- Show all nested heap pointers contained within a given
                   variable.
  hide          -- Hide a view or layer.
  mask          -- Add a transparent rectangle to the window to reveal a
                   possibly obscured or hidden view or layer's bounds
  mwarning      -- simulate a memory warning
  pa11y         -- Print accessibility labels of all views in hierarchy of
                   <aView>
  pa11yi        -- Print accessibility identifiers of all views in hierarchy of
                   <aView>
  pactions      -- Print the actions and targets of a control.
  paltrace      -- Print the Auto Layout trace for the given view. Defaults to
                   the key window.
  panim         -- Prints if the code is currently execution with a UIView
                   animation block.
  pbcopy        -- Print object and copy output to clipboard
  pblock        -- Print the block`s implementation address and signature
  pbundlepath   -- Print application's bundle directory path.
  pcells        -- Print the visible cells of the highest table view in the
                   hierarchy.
  pclass        -- Print the inheritance starting from an instance of any class.
  pcomponents   -- Print a recursive description of components found starting
                   from <aView>.
  pcurl         -- Print the NSURLRequest (HTTP) as curl command.
  pdata         -- Print the contents of NSData object as string.
  pdocspath     -- Print application's 'Documents' directory path.
  pinternals    -- Show the internals of an object by dereferencing it as a
                   pointer.
  pinvocation   -- Print the stack frame, receiver, and arguments of the
                   current invocation. It will fail to print all arguments if
                   any arguments are variadic (varargs).
  pivar         -- Print the value of an object's named instance variable.
  pjson         -- Print JSON representation of NSDictionary or NSArray object
  pkp           -- Print out the value of the key path expression using
                   -valueForKeyPath:
  pmethods      -- Print the class and instance methods of a class.
  poobjc        -- Print the expression result, with the expression run in an
                   ObjC++ context. (Shortcut for "expression -O -l ObjC++ -- " )
  pproperties   -- Print the properties of an instance or Class
  present       -- Present a view controller.
  presponder    -- Print the responder chain starting from a specific responder.
  psjson        -- Print JSON representation of Swift Dictionary or Swift Array
                   object
  ptv           -- Print the highest table view in the hierarchy.
  pvc           -- Print the recursion description of <aViewController>.
  pviews        -- Print the recursion description of <aView>.
  rcomponents   -- Synchronously reflow and update all components.
  sequence      -- Run commands in sequence, stopping on any error.
  setinput      -- Input text into text field or text view that is first
                   responder.
  settext       -- Set text on text on a view by accessibility id.
  show          -- Show a view or layer.
  slowanim      -- Slows down animations. Works on the iOS Simulator and a
                   device.
  taplog        -- Log tapped view to the console.
  uikit         -- Imports the UIKit module to get access to the types while in
                   lldb.
  unborder      -- Removes border around <viewOrLayer>.
  unmask        -- Remove mask from a view or layer
  unslowanim    -- Turn off slow animations.
  visualize     -- Open a UIImage, CGImageRef, UIView, CALayer, or
                   CVPixelBuffer in Preview.app on your Mac.
  vs            -- Interactively search for a view by walking the hierarchy.
  wivar         -- Set a watchpoint for an object's instance variable.
  xdebug        -- Print debug description the XCUIElement in human readable
                   format.
  xnoid         -- Print XCUIElement objects with label but without identifier.
  xobject       -- Print XCUIElement details.
  xtree         -- Print XCUIElement subtree.
  zzz           -- Executes specified lldb command after delay.
For more information on any command, type 'help <command-name>'.
```

## Ambiguous Layouts

&emsp;首先我们对 Auto Layout 中的 Ambiguous Layouts 知识点进行学习：[Ambiguous Layouts](https://developer.apple.com/library/archive/documentation/UserExperience/Conceptual/AutolayoutPG/AmbiguousLayouts.html#//apple_ref/doc/uid/TP40010853-CH18-SW1)。

&emsp;熟悉使用 Auto Layout 的小伙伴大概会在 Xcode 控制台见过约束添加存在歧义时打印的一些警告信息（一般在 TableView 和 CollectionView 的 cell 布局中比较常见约束不全的问题）。 

&emsp;[hasAmbiguousLayout](https://developer.apple.com/documentation/uikit/uiview/1622517-hasambiguouslayout?language=objc)

```c++
// Debugging

/* Everything in this section should be used in debugging only, never in shipping code.  These methods may not exist in the future - no promises.
 * 本节中的所有内容都应仅用于调试，切勿用于交付代码。这些方法将来可能不存在 - 无法保证。
 */
@interface UIView (UIConstraintBasedLayoutDebugging)

/* This returns a list of all the constraints that are affecting the current location of the receiver.  The constraints do not necessarily involve the receiver, they may affect the frame indirectly.
 Pass UILayoutConstraintAxisHorizontal for the constraints affecting [self center].x and CGRectGetWidth([self bounds]), and UILayoutConstraintAxisVertical for the constraints affecting[self center].y and CGRectGetHeight([self bounds]).
 */
 
 // 这将返回影响 UIView 对象当前位置的所有约束的列表。约束不一定涉及 UIView 对象，它们可能间接影响 UIView 的 frame。
 // 为影响 [self center].x 和 CGRectGetWidth([self bounds]) 的约束传递 UILayoutConstraintAxisHorizo​​ntal，
 // 为影响 [self center].y 和 CGRectGetHeight([self bounds]) 的约束传递 UILayoutConstraintAxisVertical。
- (NSArray<__kindof NSLayoutConstraint *> *)constraintsAffectingLayoutForAxis:(UILayoutConstraintAxis)axis API_AVAILABLE(ios(6.0));

/* If there aren't enough constraints in the system to uniquely determine layout, we say the layout is ambiguous.  For example, if the only constraint in the system was x = y + 100, then there are lots of different possible values for x and y.  This situation is not automatically detected by UIKit, due to performance considerations and details of the algorithm used for layout.  
 The symptom of ambiguity is that views sometimes jump from place to place, or possibly are just in the wrong place.
 -hasAmbiguousLayout runs a check for whether there is another center and bounds the receiver could have that could also satisfy the constraints.
 -exerciseAmbiguousLayout does more.  It randomly changes the view layout to a different valid layout.  Making the UI jump back and forth can be helpful for figuring out where you're missing a constraint.  
 */

// 如果系统中没有足够的约束来唯一地确定 layout，我们说 layout 是 ambiguous。
// 例如，如果系统中唯一的约束是 x = y + 100，那么 x 和 y 有很多不同的可能值。由于性能考虑和用于布局的算法的细节，UIKit 不会自动检测到这种情况。
// ambiguity 的症状是视图有时会从一个地方跳到另一个地方，或者可能只是在错误的地方。
@property(nonatomic, readonly) BOOL hasAmbiguousLayout API_AVAILABLE(ios(6.0));

- (void)exerciseAmbiguityInLayout API_AVAILABLE(ios(6.0)); 
@end
```

## alamborder/alamunborder

&emsp;如果熟悉 Python 的话可以直接在 /opt/homebrew/Cellar/chisel/2.0.1/libexec/commands/FBAutoLayoutCommands.py 路径（）下找到 `alamborder/alamunborder` 对应的 python 脚本，

&emsp;首先我们打开自己的终端，然后输入 lldb 回车，此时进入 lldb 环境，然后我们输入：`help alamborder` 可看到如下帮助信息，alamborder 命令的功能是递归 `[[UIApplication sharedApplication] keyWindow]` 的子 View，并为 `hasAmbiguousLayout` 返回 YES 的子 View 添加一个边框，同时我们可以使用 `--color/-c` 和 `--width/-w` 选项为边框设置颜色和宽度。

```c++
(lldb) help alamborder
     Put a border around views with an ambiguous layout  Expects 'raw' input (see 'help raw-input'.)

Syntax: alamborder
Put a border around views with an ambiguous layout

Options:
  --color/-c <color>; Type: string; A color name such as 'red', 'green', 'magenta', etc.
  --width/-w <width>; Type: CGFloat; Desired width of border.

Syntax: alamborder [--color=color] [--width=width]

This command is implemented as FBAutolayoutBorderAmbiguous in
/opt/homebrew/Cellar/chisel/2.0.1/libexec/commands/FBAutoLayoutCommands.py.
```

&emsp;ambiguous：adj. 引起歧义的；模棱两可的，含糊不清的。

```c++
def setBorderOnAmbiguousViewRecursive(view, width, color):
    if not fb.evaluateBooleanExpression(
        "[(id)%s isKindOfClass:(Class)[UIView class]]" % view
    ):
        return

    isAmbiguous = fb.evaluateBooleanExpression("(BOOL)[%s hasAmbiguousLayout]" % view)
    if isAmbiguous:
        layer = viewHelpers.convertToLayer(view)
        fb.evaluateEffect("[%s setBorderWidth:(CGFloat)%s]" % (layer, width))
        fb.evaluateEffect(
            "[%s setBorderColor:(CGColorRef)[(id)[UIColor %sColor] CGColor]]"
            % (layer, color)
        )

    subviews = fb.evaluateExpression("(id)[%s subviews]" % view)
    subviewsCount = int(fb.evaluateExpression("(int)[(id)%s count]" % subviews))
    if subviewsCount > 0:
        for i in range(0, subviewsCount):
            subview = fb.evaluateExpression("(id)[%s objectAtIndex:%i]" % (subviews, i))
            setBorderOnAmbiguousViewRecursive(subview, width, color)
```


## 参考链接
**参考链接:🔗**
+ [Auto Layout Guide](https://developer.apple.com/library/archive/documentation/UserExperience/Conceptual/AutolayoutPG/AmbiguousLayouts.html#//apple_ref/doc/uid/TP40010853-CH18-SW1)
+ [Debugging with Xcode](https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/debugging_with_xcode/chapters/about_debugging_w_xcode.html#//apple_ref/doc/uid/TP40015022-CH10-SW1)
+ [LLDB Quick Start Guide](https://developer.apple.com/library/archive/documentation/IDEs/Conceptual/gdb_to_lldb_transition_guide/document/Introduction.html#//apple_ref/doc/uid/TP40012917-CH1-SW1)
+ [Advanced Debugging with Xcode and LLDB](https://developer.apple.com/videos/play/wwdc2018/412/)
+ [Xcode 10.2 Release Notes](https://developer.apple.com/documentation/xcode-release-notes/xcode-10_2-release-notes)
+ [小笨狼的LLDB技巧:chisel](https://www.jianshu.com/p/afaaacc55460)
+ [iOS开发中如何调试更高效：LLDB与Chisel](https://juejin.cn/post/6844904017416421390)
+ [与调试器共舞 - LLDB 的华尔兹](https://objccn.io/issue-19-2/)
+ [Chisel-LLDB命令插件，让调试更Easy](https://blog.cnbluebox.com/blog/2015/03/05/chisel/)
+ [LLDB高级调试+Cycript](https://juejin.cn/post/6844903990296068110#heading-5)
+ [iOS逆向 玩转LLDB调试](https://juejin.cn/post/6847902223926722573)
+ [iOS 常用调试方法：LLDB命令](https://juejin.cn/post/6844903794493358093)
+ [LLDB学习笔记](https://www.jianshu.com/p/e5cc0f83a4f0)
+ [iOS调试之chisel](https://www.jianshu.com/p/3eef81bf146d)
+ [iOS调试进阶-更高效的使用Xcode和LLDB](https://juejin.cn/post/6844903866345996296)
+ [Xcode10.2中LLDB增加的新特性](https://juejin.cn/post/6844903848771846157)

+ [WWDC 2018：效率提升爆表的 Xcode 和 LLDB 调试技巧](https://juejin.cn/post/6844903620329078791#heading-28)
+ [lldb-入坑指北（1）-给xcode添加批量启用禁用断点功能](https://cloud.tencent.com/developer/article/1729078)
+ [XCode LLDB调试小技巧基础篇提高篇汇编篇](https://cloud.tencent.com/developer/article/1013359)
+ [(轉)Xcode中LLDB的基本命令的使用（Swift）](https://www.itread01.com/p/357976.html)
+ [LLDB 不是只有 po](https://medium.com/@yenchiayou/lldb-不是只有-po-563597c78faf)
+ [facebook/chisel](https://github.com/facebook/chisel/tree/master)

https://developer.apple.com/documentation/xcode-release-notes/xcode-10_2-release-notes
https://juejin.cn/post/6847902223926722573
https://juejin.cn/post/6844903647277481998
https://www.jianshu.com/p/7e2a61585352
https://www.jianshu.com/p/b2371dd4443b
https://www.jianshu.com/p/afaaacc55460
https://www.jianshu.com/p/e5cc0f83a4f0
https://www.jianshu.com/p/3eef81bf146d
https://juejin.cn/post/6844904017416421390#heading-27
https://lldb.llvm.org/use/map.html
https://objccn.io/issue-19-2/
https://juejin.cn/post/6844903990296068110#heading-5
https://github.com/facebook/chisel
https://blog.cnbluebox.com/blog/2015/03/05/chisel/

+ [Step Over Thread和Step Into Thread命令有什么用处？](http://cn.voidcc.com/question/p-tjpujtpj-bez.html)
+ [对 Xcode 菜单选项的详细探索](https://blog.csdn.net/zhanglizhi111/article/details/52326744)
