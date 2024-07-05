# Flutter 源码梳理系列（十四）：Element：updateChildren

# updateChildren

```dart
@protected
List<Element> updateChildren(
    List<Element> oldChildren,
    List<Widget> newWidgets,
    {Set<Element>? forgottenChildren,
        List<Object?>? slots}
)
```

&emsp;前面说到在 framework.dart 中看到 updateChildren 函数调用出现在了 MultiChildRenderObjectElement 的更新函数中：

```dart
 _children = updateChildren(_children, multiChildRenderObjectWidget.children, forgottenChildren: _forgottenChildren);
```

&emsp;可看到 updateChildren 使用一组新的 widget 来更新此 element 的现有的 `_children` 列表（子级 Element 列表）。

&emsp;OK，有点懵，毕竟 MultiChildRenderObjectWidget/MultiChildRenderObjectElement 都分别有一组子级，牵涉的内容比较多，下面我们先看一下 updateChildren 函数的文档，然后再看它的内部代码。

&emsp;updateChildren 函数尝试使用给定的 newWidgets（一组不同类型的 Widget 对象列表）更新给定的 oldChildren（一组不同类型的 Element 对象列表，每个 Element 对象也可以理解为一颗 Element 子树），根据需要删除过时的 element 并引入新 element，最终返回与 newWidgets 中新 Widget 一一对应的新的 element 对象列表。

&emsp;在此函数执行期间，不得修改 oldChildren 列表（它内部是旧的 element 对象）。如果调用方希望在此函数在栈上时从 oldChildren 中移除元素，调用方可以提供一个 forgottenChildren 参数。每当此函数从 oldChildren 中读取 element 对象时，此函数首先检查该 element 对象是否在 forgottenChildren 集合中，如果是，则该函数会假设该 element 对象不在 oldChildren 中，直接 break 结束 while 循环。

&emsp;上面一段描述有点晕，我们来分析一下，通过代码可以看出 oldChildren 中的 element 对象，如果也出现在 forgottenChildren 集合中的话，则会直接结束对 oldChildren 的 while 循环，通过函数内部的 replaceWithNullIfForgotten 函数进行判断。

&emsp;同时还有一个操作，从 newWidgets 和 oldChildren 的同一个下标位分别取得 widget 对象和 element 对象，如果这个 widget 对象不可以更新 element 对象时（通过 Widget.canUpdate 判断返回 false）也会直接结束对 oldChildren 的 while 循环。）

&emsp;updateChildren 函数是对 updateChild 的方便包装，后者会更新每个单独的 element 对象。如果入参 slots 非空，则调用 updateChild 函数时的 newSlot 参数的值将从该 slots 列表中按照当前处理的 widget 在 newWidget 列表中对应的索引检索（newWidget 和 slot 必须具有相同的长度）。如果 slots 为空的话，则将新建一个 IndexedSlot 对象作为 newSlot 参数的值，在这种情况下，IndexedSlot 对象的 index 字段设置为当前处理的 widget 在 newWidget 列表中的索引值（index），而 IndexedSlot 对象的 value 字段设置为当前处理的 widget 的前一个 widget 对象的 Element 对象（若当前处理的 widget 是第一个的话，IndexedSlot 对象的 value 属性则设置为 null）。

&emsp;意思就是给 updateChildren 函数返回的 Element 列表中的每个 element 对象设置它们的 slot 值，这个 slot 值是 IndexedSlot 类型的，而这个 IndexedSlot 类其实也是蛮简单的，它共有两个字段，一个字段是 index 标识每个 element 对象在返回的 Element 列表中的索引，还有一个 value 字段，可标识每个 element 对象前面的那个 element 对象。

 &emsp;当一个 element 对象的 slot 发生变化时，其关联的 renderObject 需要移动到其父级的 children 中的新位置。如果该 RenderObject 将其子元素组织成链表（如 ContainerRenderObjectMixin 所做的），则可以通过在与 slot 对象中的 IndexedSlot.value 相关的 Element 的 RenderObject 之后重新将子 RenderObject 插入到列表中来实现这一点。(关于 RenderObject 的内容后续再进行补充)
 
 &emsp;下面我们先分心一下，看一下 IndexedSlot 这个泛型类的内容。
 
## IndexedSlot
 
&emsp;`IndexedSlot<T extends Element?>` 用于表示 MultiChildRenderObjectElement.children 中的 Element 对象的 `slot` 值。

&emsp;对于 MultiChildRenderObjectElement.children 中的 Element 对象，它们的 `slot` 属性的值都是 IndexedSlot 类型的。IndexedSlot 包括一个索引 `index`，用来标识该 Element 对象在 MultiChildRenderObjectElement.children 列表中的索引，以及一个任意值，可以进一步定义该 Element 对象在 MultiChildRenderObjectElement.children 中的位置（如在 updateChildren 中，把此任意值设置为了此 Element 对象前面那个 Element 对象，如果是第一个 Element 对象的话，则此任意值为 null）。

&emsp;直接看 IndexedSlot 类的代码，它还是特别简单的，有一个 int value 字段和一个泛型 T value 字段，然后重写了 operator == 和 hashCode，由 index 和 value 共同决定 IndexedSlot 对象的判等和生成哈希码。其它就没有任何内容了。 

```dart
@immutable
class IndexedSlot<T extends Element?> {
  const IndexedSlot(this.index, this.value);

  final T value;
  final int index;

  @override
  bool operator ==(Object other) {
    if (other.runtimeType != runtimeType) {
      return false;
    }
    
    return other is IndexedSlot
        && index == other.index
        && value == other.value;
  }

  @override
  int get hashCode => Object.hash(index, value);
}
```

&emsp;然后下面还有一段较复杂的官方注释，讲解 Flutter 中关于管理子元素顺序的问题，一起来看一下：

&emsp;在 MultiChildRenderObjectElement 中 子元素的顺序不仅取决于前一个兄弟节点，还取决于它在列表中的位置。当子元素在列表中的位置发生变化时，即使它的前一个兄弟节点没有变化，它的渲染位置也可能需要移动。举个例子，原来顺序是 `[e1, e2, e3, e4]`，变为 `[e1, e3, e4, e2]` 时，元素 e4 的前一个兄弟依然是 e3，但是由于在列表中的位置变了，它的渲染位置需要移动到 e2 之前。为了触发这种移动，当子元素在父节点的子元素列表中的位置发生变化时，需要为元素分配一个新的位置，即 `IndexedSlot<Element>` 类型的对象。这样可以确保父节点的渲染对象知道子元素需要移动到链表的哪个位置，同时确定它的新的前一个兄弟节点。

&emsp;Ok，IndexedSlot 的内容看完了，下面我们继续回到 updateChildren 函数。

&emsp;再拓展一点东西，其实之所以要看 updateChildren 函数，还有一个原因，就是如果我们想要搞清楚当一组位于同一个父级的 Widget（如 Column.children 和 Row.children 中的 Widget），它们都有除了 GlobalKey 之外的其它 Key 时，它们是怎么复用的，答案就在 updateChildren 函数中。

&emsp;前面我们学习在当前 BuildOwner 下的全局非活动 Element 列表时，看到想要 Element 复用时需要它们对应的 Widget 有 GlobalKey。即在 Element.inflateWidget 函数中可看到只有 newWidget 有 GlobalKey 才有机会去复用 Element 对象。

&emsp;那么其它情况的 key 怎么复用呢？例如我们最常见的滚动列表，我们给列表中的样子相同的每个 Widget 添加一个 ValueKey 或者 ObjectKey，当列表进行重建，那么每个有 Key 的   Widget 它们对应的 Element 是怎么复用的呢？然后最出名的就是官方的这个视频：[When to Use Keys - Flutter Widgets 101 Ep. 4](https://www.youtube.com/watch?v=kn0EOS-ZiIc)，当给 StatefulWidget 添加了 key 之后，它们就可以复用 Element 和 State 对象了，它们是怎么做到的呢？其实答案都在 updateChildren 函数中。 

&emsp;updateChildren 函数内容有点复杂哦，然后我们再分析一下，我们可以带着下面几个准则去看 updateChildren 函数的话会更好理解一些：

1️⃣. oldChildren 列表中是之前的旧 Element 对象，它内部的 Element 对象如果不能被复用的话最后都需要做失活处理。
2️⃣. newWidgets 是一组新的 Widget 对象，而 updateChildren 函数的宗旨就是为了返回与这一组新的 Widget 对象一一对应的 Element 对象。
3️⃣. **updateChildren 返回值列表的长度必是要与 newWidgets 列表长度相等的。**
4️⃣. **newWidgets 列表的长度与 oldChildren 列表的长度不一定是相等的。**
5️⃣. **尽最大努力的让 oldChildren 中的旧的 Element 对象得到复用。**
6️⃣. **不管怎么折腾都要保证：updateChildren 返回的 Element 列表中的 Element 对象要和 newWidgets 列表中的 Widget 对象一一对应。**
7️⃣. **不管怎么折腾 newWidgets 列表都是不能变的。**
7️⃣. **Key 相等的两个 Widget 对象，并不一定 runtimeType 也相等，所以它们调用 Widget.canUpdate 可能返回 true 或 false。**
8️⃣. **拿着 Key 相等的 Element 对象和 Widget 对象执行 Element.updateChild 函数，内部还有 Widget.canUpdate 会进一步判断 Widget 对象是否可以更新 Element 对象。**
9️⃣. 当 Widget.canUpdate 返回 true 时，可以直接复用 Element 对象，此时的复用效果和以 Key 取得对应的 Element 的复用效果是一样的。
1️⃣0️⃣. 自始至终说过很多次 Element 复用，也可以说是 Element 子树复用，但是在实际复用执行过程中，我们可以当下立即确定的是当下这个 Element 对象得到了复用，而后续它的子树中的 Element 节点能不能得到复用只能看它们对应的 Widget 的实际情况来确定。例如：所有的 Element 节点都得到了复用、从这一个 Element 节点后，后续全部不能复用了只能新建、从这一个 Element 节点到某个 Element 节点得到了复用，其它的还是要新建。等等，具体能复用到 Widget tree 末端，只能跟 Widget 的实际情况有关。
1️⃣1️⃣. 为了减轻理解负担，初次看时可以把 forgottenChildren 和 slots 参数作为 null 看待。

```dart
  @protected
  List<Element> updateChildren(List<Element> oldChildren, List<Widget> newWidgets, { Set<Element>? forgottenChildren, List<Object?>? slots }) {
  
    // 嵌套函数，如果 child 入参包含在 forgottenChildren 集合中，则返回 null
    Element? replaceWithNullIfForgotten(Element child) {
      return forgottenChildren != null && forgottenChildren.contains(child) ? null : child;
    }
    
    // 如果 slots 入参不为 null，则返回 newChildIndex 索引处的 slot，否则创建一个 IndexedSlot 对象返回，
    // IndexedSlot 类也贼简单，仅仅是有两个 final 字段的类：final T value 和 final int index，
    // 把两个入参 newChildIndex 和 previousChild 绑定在一起
    Object? slotFor(int newChildIndex, Element? previousChild) {
      return slots != null
          ? slots[newChildIndex]
          : IndexedSlot<Element?>(newChildIndex, previousChild);
    }
    
    // 上面是两个嵌套函数的定义，下面开始看 updateChildren 函数的主要内容。
    
    // 这段代码尝试将新的 widget 列表（newWidgets）与旧的 element 列表（oldChildren）进行差异比较，并生成一个新的 element 列表返回，
    // 而返回的 Element 列表则可作为 Element 的新子元素列表，例如 MultiChildRenderObjectElement 的 _children 属性，
    // 调用此方法的 Element 对象相应也更新其 render object。

    // 新旧 Children 列表索引分别从 0 开始
    int newChildrenTop = 0;
    int oldChildrenTop = 0;
    
    // 新的 Children 列表的长度与新 Widget 列表的长度相等
    int newChildrenBottom = newWidgets.length - 1;
    
    // 旧的 Children 列表的长度还是自己的
    int oldChildrenBottom = oldChildren.length - 1;
    
    // _NullElement 是一个很简单的 Element 子类，主要用于：当实际 Element 尚未确定时，在 List<Element> 对象中用作占位符的

    // 准备一个与 newWidgets 长度相同，用 _NullElement.instance 占位的 List，待存入新的 Element，最后作为 updateChildren 函数的返回值。
    // 这里对应我们的宗旨 3️⃣. **updateChildren 返回值列表的长度必是要与 newWidgets 列表长度相等的。**
    final List<Element> newChildren = List<Element>.filled(newWidgets.length, _NullElement.instance);
    
    // 指向前一个 Element，主要用于给当前 Element 生成 IndexedSlot 时使用
    Element? previousChild;
    
    // Update the top of the list.
    // While 1️⃣
    while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
    
      // oldChildrenTop 和 newChildrenTop 依次从 0 开始，分别取出相同索引的新 Widget 对象和 旧 Element 对象 
      final Element? oldChild = replaceWithNullIfForgotten(oldChildren[oldChildrenTop]);
      final Widget newWidget = newWidgets[newChildrenTop];
      
      // 这里进行判断，如果新 Widget 对象能直接更新旧 Element 对象，那么直接进行更新即可，然后把更新结果装进 newChildren 列表即可，
      // 这样更新一个就代表 newWidgets 列表中的一个新 Widget 对象完成了任务。
      // 但是如果对应索引的新 Widget 对象不能直接更新旧 Element 对象的话，则直接跳出此 while 循环，
      // 即这个 While 循环只处理：新 Widget 对象能直接更新旧 Element 对象的情况，其它情况不归它管，它直接跳出循环即可往下执行即可。
      // 这个操作是为了后续能最大可能的复用旧 Element 对象，例如旧 Element 情况是：[a, b, c, d] 新 Widget 情况是：[a, b, e, f, c, d]，
      // 这前面的 Widget a 和 b 能如约更新 Element a 和 b，但是到了 Widget e 它不能更新 Element c，
      // 可是呢后续还是有 Widget c 和 d 可以更新 Element c 和 d 呢，只是它们的下标索引不同了，
      // 那此时呢就跳出这个 Wihile 循环，让后续的代码来处理，争取最大的努力让旧 Element c 和 d 对象得到复用。
      if (oldChild == null || !Widget.canUpdate(oldChild.widget, newWidget)) {
        break;
      }
      
      // 到这里，仅处理 oldChild 有值并且 newWidget 能直接更新 oldChild 的情况
      final Element newChild = updateChild(oldChild, newWidget, slotFor(newChildrenTop, previousChild))!;
      
      // 这样就得到了一个可用的情况，添加到 newChildren 列表中
      newChildren[newChildrenTop] = newChild;
      
      // 每次有 Element 对象放在 newChildren 中都代表着：previousChild 和 newChildrenTop 要更新
      
      // 记录下此 Element 对象，为它后面的 Element 对象创建 IndexedSlot 提供参数
      previousChild = newChild;
      
      // 分别 + 1，进入下一个循环
      newChildrenTop += 1;
      oldChildrenTop += 1;
    }

    // Scan the bottom of the list.
    // While 2️⃣
    while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
    
      // 这个循环只是为了缩小 oldChildrenBottom 和 newChildrenBottom 值，
      // 并不会对旧 Element 对象直接更新，即使在判断到新 Widget 可以更新旧 Element 对象。
      
      // 从后往前进行判断，取最后一个新 Widget 对象看是否可以更新最后一个旧 Element 对象
      final Element? oldChild = replaceWithNullIfForgotten(oldChildren[oldChildrenBottom]);
      final Widget newWidget = newWidgets[newChildrenBottom];
      
      // 如果遇到不能直接更新的就跳出此 While 循环
      if (oldChild == null || !Widget.canUpdate(oldChild.widget, newWidget)) {
        break;
      }
      
      // 遇到可以直接更新的情况，就给 bottom 们减 1
      oldChildrenBottom -= 1;
      newChildrenBottom -= 1;
    }

    // Scan the old children in the middle of the list.
    // 如果旧 element 列表中还有旧的 Element 对象没有判断，
    // 那么对于余下的无法通过索引来判断新 Widget 是否可以更新旧 Element 的情况，
    // 先收集旧 Element 的 Widget key 再说，这样可以增大旧 Element 对象被复用的概率。
    // 毕竟 key 相等时，Widget.canUpdate 函数返回 true 的概率已经提高到 50% 了。
    final bool haveOldChildren = oldChildrenTop <= oldChildrenBottom;
    
    // 收集余下的旧 Element 列表中有 Key 的旧 Element 对象  
    Map<Key, Element>? oldKeyedChildren;
    
    if (haveOldChildren) {
      oldKeyedChildren = <Key, Element>{};
      
      // 遍历余下的旧 Element 对象，把有 key 的收集起来
      // While 3️⃣
      while (oldChildrenTop <= oldChildrenBottom) {
        final Element? oldChild = replaceWithNullIfForgotten(oldChildren[oldChildrenTop]);
        if (oldChild != null) {
          if (oldChild.widget.key != null) {
          
            // 有 key 的旧 Element 对象收集起来，等到后面尝试复用
            oldKeyedChildren[oldChild.widget.key!] = oldChild;
            
          } else {
            // 无 key 的 Element 对象用不到饿，可以进行失活了，
            // 把 oldChild 为根节点的整个子树从 Element Tree 上卸载下来。
            deactivateChild(oldChild);
          }
        }
        
        // 递增
        oldChildrenTop += 1;
      }
    }

    // Update the middle of the list.
    
    // 这里继续对 newWidgets 进行循环。
    // 看到这里的小伙伴可能有点懵哦，我们可以停下来看一下，搬出我们的主旨 6️⃣：
    
    // 6️⃣. **不管怎么折腾都要保证：updateChildren 返回的 Element 列表中的 Element 对象要和 newWidgets 列表中的 Widget 对象一一对应。**
    
    // 就是从上面走到这里，newChildrenTop 值的改变只是在 While 1️⃣ 中有一处，
    // 所以代码走到这里，我们对 newWidgets 的遍历还是停在 newChildrenTop 的位置，
    // 即我们只对 newWidgets 列表中 newChildrenTop 索引前面的 Widget 对象取得了对应的 Element 对象，并存放在 newChildren 中，
    // 下面我们还要继续对剩余的 newWidgets 中的新 widget 对象进行遍历，获得对应的 Element 对象。
    
    // 这里循环边界用的 newChildrenBottom，在上面它跟着 oldChildrenBottom 做过递减操作，
    // 所以目前它的值可能是小于 newWidgets.length - 1 的。
    
    while (newChildrenTop <= newChildrenBottom) {

      Element? oldChild;
      
      // 从 newWidgets 中取新的 Widget 对象 
      final Widget newWidget = newWidgets[newChildrenTop];
      
      // 然后就是尝试对剩余的有 key 的旧 Element 对象进行复用
      if (haveOldChildren) {
        final Key? key = newWidget.key;
        
        if (key != null) {
          oldChild = oldKeyedChildren![key];
          if (oldChild != null) {
          
            // 如果刚好找到了与新 Widget key 相等的旧 Element 对象，继而调用 Widget.canUpdate 继续判断，
            // 这旧 Element 对象是否可以复用，
            if (Widget.canUpdate(oldChild.widget, newWidget)) {
              // we found a match! remove it from oldKeyedChildren so we don't unsync it later
              
              // 如果可以复用的话，把这旧 Element 从 oldKeyedChildren 中移除，
              // 因为等到最后要对 oldKeyedChildren 中确实无法复用对 Element 对象进行集体失活。
              oldKeyedChildren.remove(key);
              
            } else {
              // Not a match, let's pretend we didn't see it for now.
              oldChild = null;
            }
          }
        }
      }
      
      // 然后是执行到这里了：oldChild 可能有值也可能为 null，但是 newWidget 一定是有值，所以 updateChild 会根据情况，
      // 返回新创建的 Element 或者是更新过的 oldChild，
      // 所以执行到这里，已经对无法通过索引直接判断是否可复用的旧 Element 进行了最大限度的复用尝试。 
      final Element newChild = updateChild(oldChild, newWidget, slotFor(newChildrenTop, previousChild))!;
      
      // 把新 Widget 对象取得的 Element 对象放在 newChildren 中等待返回
      newChildren[newChildrenTop] = newChild;
      
      // 每次有 Element 对象放在 newChildren 中都代表着：previousChild 和 newChildrenTop 要更新
      previousChild = newChild;
      newChildrenTop += 1;
    }
    
    // We've scanned the whole list.
    
    // 然后最后就是对 While 2️⃣ 中对 oldChildren 和 newWidgets 的反向判断了，
    // 在 While 2️⃣ 中从后往前，对新 Widget 是否能更新旧 Element 进行了判断，
    // 此处则是要对这些 newWidgets 末尾剩余的新 Widget 对象获取对应的 Element 对象了。
    
    // 把两个 bottom 更新为原始的 bottom
    newChildrenBottom = newWidgets.length - 1;
    oldChildrenBottom = oldChildren.length - 1;
    
    // Update the bottom of the list.
    while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
      // 分别取得剩余的新 Widget 和旧 Element 
      final Element oldChild = oldChildren[oldChildrenTop];
      final Widget newWidget = newWidgets[newChildrenTop];
      
      // 取得新 Widget 对应的 Element（复用的 oldChildren 中的旧 Element），然后放在 newChildren 列表中等待返回
      final Element newChild = updateChild(oldChild, newWidget, slotFor(newChildrenTop, previousChild))!;
      newChildren[newChildrenTop] = newChild;
      
      // 同样的，每次有 Element 对象放在 newChildren 中都代表着：previousChild 和 newChildrenTop 要更新
      previousChild = newChild;
      newChildrenTop += 1;
      
      oldChildrenTop += 1;
    }
    
    // Clean up any of the remaining middle nodes from the old list.
    
    // 然后就是 oldKeyedChildren 中剩余的有 key 的旧 Element 了，它们都用不到了，都需要失活
    if (haveOldChildren && oldKeyedChildren!.isNotEmpty) {
      for (final Element oldChild in oldKeyedChildren.values) {
        if (forgottenChildren == null || !forgottenChildren.contains(oldChild)) {
          
          // 对这些指定的旧 Element 对象进行失活，
          // 把 oldChild 为根节点的整个子树从 Element Tree 上卸载下来。
          deactivateChild(oldChild);
        }
      }
    }
    
    return newChildren;
```

## updateChildren 总结

&emsp;OK，updateChildren 看懂了吗，确实有点复杂的一个函数！但是尽管内容看起来比较复杂但是它的函数功能还是异常清晰的：

&emsp;updateChildren 函数主打一个能复用就不会新建。使用给定的 newWidgets 中的新 Widget 对象构建一组新的 Element 对象并返回。在过程中尽最大努力的对给定的 oldChildren 中的旧 Element 对象进行复用，当新 Widget 对象无法复用旧的 Element 对象时会构建新的 Element 对象，并最终对那些无法复用的旧 Element 对象进行失活处理。

&emsp;在过程中先从 newWidgets 和 oldChildren 列表的两头进行复用性匹配，然后再收集 oldChildren 中有 key 的 Element 进行尝试复用，最终完成沿着 newWidgets 列表，构建一组对应的 Element 对象并返回。

&emsp;特别是其中一段对 oldChildren 的遍历把有 key 的 Element 对象收集起来，然后与新 Widget 对象的 key 进行比较，然后尝试进行复用的内容。只有真正看懂了这个，我们才能理解 GlobalKey 之外的其它 key（ValueKey、ObjectKey）的存在意义。Flutter 真的处处体现着对 Element 子树重建的优化！

&emsp;和 updateChild 函数比较的话：虽然 updateChildren 内部是对 updateChild 的包装，但是 updateChildren 函数的功能更加单一，但是由于 updateChildren 中一堆花式循环，看的人眼花撩乱，不过跟着它的主流程：遍历 newWidgets 列表中的新 Widget 对象，分别一一构建新 Widget 对应的 Element 对象，就这么一个核心逻辑，沿着看的话是完全能看懂的。然后它的主旨也是特别明确的：始终都是围绕着最大限度的对旧 Element 对象（旧 Element 子树）进行复用，我们在日常开发中也应该严格遵循此主旨！⛽️

&emsp;OK，接下来是 Element 类下面其它的较简单的 Api。

## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
