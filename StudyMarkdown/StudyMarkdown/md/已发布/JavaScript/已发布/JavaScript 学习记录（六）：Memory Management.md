# JavaScript 学习记录（六）：Memory Management

&emsp;像 C 语言这样的底层语言一般都有底层的内存管理接口，比如 malloc() 和 free()。相反，JavaScript 是在创建变量（对象，字符串等）时自动进行了分配内存，并且在不使用它们时 "自动" 释放。 释放的过程称为垃圾回收。这个 "自动" 是混乱的根源，并让 JavaScript（和其他高级语言）开发者错误的感觉他们可以不关心内存管理。 

## 内存生命周期

&emsp;不管什么程序语言，内存生命周期基本是一致的：   

1. 分配你所需要的内存
2. 使用分配到的内存（读、写）
3. 不需要时将其释放/归还

&emsp;所有语言第二部分都是明确的。第一和第三部分在底层语言中是明确的，但在像 JavaScript 这些高级语言中，大部分都是隐含的。

### JavaScript 的内存分配

#### 值的初始化

&emsp;为了不让程序员费心分配内存，JavaScript 在定义变量时就完成了内存分配。

```javascript
var n = 123; // 给数值变量分配内存
var s = "azerty"; // 给字符串分配内存

var o = {
  a: 1,
  b: null
}; // 给对象及其包含的值分配内存

// 给数组及其包含的值分配内存（就像对象一样）
var a = [1, null, "abra"];

function f(a){
  return a + 2;
} // 给函数（可调用的对象）分配内存

// 函数表达式也能分配一个对象
someElement.addEventListener('click', function(){
  someElement.style.backgroundColor = 'blue';
}, false);
```

#### 通过函数调用分配内存

&emsp;有些函数调用结果是分配对象内存：

```javascript
var d = new Date(); // 分配一个 Date 对象

var e = document.createElement('div'); // 分配一个 DOM 元素
```

&emsp;有些方法分配新变量或者新对象：

```javascript
var s = "azerty";
var s2 = s.substr(0, 3); // s2 是一个新的字符串
// 因为字符串是不变量，JavaScript 可能决定不分配内存，只是存储了 [0-3] 的范围。

var a = ["ouais ouais", "nan nan"];
var a2 = ["generation", "nan nan"];
var a3 = a.concat(a2);
// 新数组有四个元素，是 a 连接 a2 的结果
```

### 使用值

&emsp;使用值的过程实际上是对分配内存进行读取与写入的操作。读取与写入可能是写入一个变量或者一个对象的属性值，甚至传递函数的参数。

### 当内存不再需要使用时释放

&emsp;大多数内存管理的问题都在这个阶段。在这里最艰难的任务是找到 "哪些被分配的内存确实已经不再需要了"。它往往要求开发人员来确定在程序中哪一块内存不再需要并且释放它。

&emsp;高级语言解释器嵌入了 "垃圾回收器"，它的主要工作是跟踪内存的分配和使用，以便当分配的内存不再使用时，自动释放它。这只能是一个近似的过程，因为要知道是否仍然需要某块内存是无法判定的（无法通过某种算法解决）。

## 垃圾回收

&emsp;如上文所述自动寻找是否一些内存 "不再需要" 的问题是无法判定的。因此，垃圾回收实现只能有限制的解决一般问题。本节将解释必要的概念，了解主要的垃圾回收算法和它们的局限性。

### 引用

&emsp;垃圾回收算法主要依赖于引用的概念。在内存管理的环境中，一个对象如果有访问另一个对象的权限（隐式或者显式），叫做一个对象引用另一个对象。例如，一个 Javascript 对象具有对它原型的引用（隐式引用）和对它属性的引用（显式引用）。

&emsp;在这里，"对象" 的概念不仅特指 JavaScript 对象，还包括函数作用域（或者全局词法作用域）。

### 引用计数垃圾收集

&emsp;这是最初级的垃圾收集算法。此算法把 "对象是否不再需要" 简化定义为 "对象有没有其他对象引用到它"。如果没有引用指向该对象（零引用），对象将被垃圾回收机制回收。

```javascript
var o = {
  a: {
    b:2
  }
};
// 两个对象被创建，一个作为另一个的属性被引用，另一个被分配给变量 o，很显然，没有一个可以被垃圾收集

var o2 = o; // o2 变量是第二个对 "这个对象" 的引用

o = 1;      // 现在，"这个对象" 只有一个 o2 变量的引用了，"这个对象" 的原始引用 o 已经没有

var oa = o2.a; // 引用 "这个对象" 的 a 属性，现在，"这个对象" 有两个引用了，一个是 o2，一个是 oa

o2 = "yo"; // 虽然最初的对象现在已经是零引用了，可以被垃圾回收了，但是它的属性 a 的对象还在被 oa 引用，所以还不能回收

oa = null; // a 属性的那个对象现在也是零引用了，它可以被垃圾回收了
```

#### 限制：循环引用

&emsp;该算法有个限制：无法处理循环引用的事例。在下面的例子中，两个对象被创建，并互相引用，形成了一个循环。它们被调用之后会离开函数作用域，所以它们已经没有用了，可以被回收了。然而，引用计数算法考虑到它们互相都有至少一次引用，所以它们不会被回收。

```javascript
function f() {
  var o = {};
  var o2 = {};
  o.a = o2; // o 引用 o2
  o2.a = o; // o2 引用 o

  return "azerty";
}

f();
```

#### 实际例子

&emsp;IE 6, 7 使用引用计数方式对 DOM 对象进行垃圾回收。该方式常常造成对象被循环引用时内存发生泄漏：

```javascript
var div;
window.onload = function() {
  div = document.getElementById("myDivElement");
  div.circularReference = div;
  div.lotsOfData = new Array(10000).join("*");
};
```

&emsp;在上面的例子里，myDivElement 这个 DOM 元素里的 circularReference 属性引用了 myDivElement，造成了循环引用。如果该属性没有显示移除或者设为 null，引用计数式垃圾收集器将总是且至少有一个引用，并将一直保持在内存里的 DOM 元素，即使其从 DOM 树中删去了。如果这个 DOM 元素拥有大量的数据 (如上的 lotsOfData 属性)，而这个数据占用的内存将永远不会被释放。

### 标记 - 清除算法

&emsp;这个算法把 "对象是否不再需要" 简化定义为 "对象是否可以获得"。

&emsp;这个算法假定设置一个叫做根（root）的对象（在 Javascript 里，根是全局对象）。垃圾回收器将定期从根开始，找所有从根开始引用的对象，然后找这些对象引用的对象……从根开始，垃圾回收器将找到所有可以获得的对象和收集所有不能获得的对象。

&emsp;这个算法比前一个要好，因为 "有零引用的对象" 总是不可获得的，但是相反却不一定，参考 "循环引用"。

&emsp;从 2012 年起，所有现代浏览器都使用了标记 - 清除垃圾回收算法。所有对 JavaScript 垃圾回收算法的改进都是基于标记 - 清除算法的改进，并没有改进标记 - 清除算法本身和它对 "对象是否不再需要" 的简化定义。

#### 循环引用不再是问题了

&emsp;在上面的示例中，函数调用返回之后，两个对象从全局对象出发无法获取。因此，他们将会被垃圾回收器回收。第二个示例同样，一旦 div 和其事件处理无法从根获取到，他们将会被垃圾回收器回收。

#### 限制：那些无法从根对象查询到的对象都将被清除

&emsp;尽管这是一个限制，但实践中我们很少会碰到类似的情况，所以开发者不太会去关心垃圾回收机制。

## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
