# Dart 学习笔记

> &emsp;Dart 是谷歌开发的计算机编程语言，后来被 Ecma (ECMA-408)认定为标准。它被用于 web、服务器、移动应用（Flutter） 和物联网等领域的开发。它是宽松开源许可证（修改的 BSD 证书）下的开源软件。(Web 编程语言 Dart)
>
> &emsp;Dart 是面向对象的、类定义的、单继承的语言。它的语法类似 C 语言，可以转译为 JavaScript，支持接口（interfaces）、混入（mixins）、抽象类（abstract classes）、具体化泛型（reified generics）、可选类型（optional typing）和 sound type system（sound null safety：Dart 中所有变量引用的都是 对象，每个对象都是一个类的实例。数字、函数以及 null 都是对象。除去 null 以外（如果你开启了空安全），所有的类都继承于 Object 类）。
> 
> &emsp;Dart 亮相于 2011 年 10 月 10 日至 12 日在丹麦奥尔胡斯举行的 GOTO 大会上。该项目由 Lars bak 和 kasper lund 创建。
>
> &emsp;Ecma 国际组织组建了技术委员会 TC52 来开展 Dart 的标准化工作，并且在 **Dart 可以编译为标准 JavaScript** 的情况下，它可以在任何现代浏览器中有效地工作。Ecma 国际组织于 2014 年 7 月第 107 届大会批准了 Dart 语言规范第一版，并于 2014 年 12 月批准了第二版。
> 
> &emsp;2015 年 5 月 Dart 开发者峰会上，亮相了基于 Dart 语言的移动应用程序开发框架 Sky，后更名为 Flutter。
>
> &emsp;2018 年 2 月，Dart2 成为强类型语言。[DART（计算机编程语言）--百度百科](https://baike.baidu.com/item/DART/22500518?fr=aladdin)

&emsp;Dart 开发环境：

+ [DartPad](https://dartpad.cn) 是一个可以让你在任何现代化的浏览器中体验 Dart 编程语言线上工具，它是开源的。即在网页上写 Dart，那么我们还是选择 VSCode 吧！
+ 使用 VS Code 安装 Dart 扩展，然后新建一个 .dart 文件直接开始 Dart 编程！ 

## main 

&emsp;每个应用都有一个 `main()` 函数。可以使用顶层（top-level）函数 `print()` 来将一段文本输出显示到控制台：

```dart
void main() {
  print('Hello, World!');
}
```

## 变量

&emsp;虽然 Dart 是代码类型安全的语言，但是由于其 **支持类型推断**，因此大多数变量不需要显式地指定类型：(代码类型安全和支持类型推断)

```dart
  var name = 'Voyager I';
  var year = 1977;
  var antennaDiameter = 3.7;
  var flybyObjects = ['Jupiter', 'Saturn', 'Uranus', 'Neptune'];
  var image = {
    'tags': ['saturn'],
    'url': '//path/to/saturn.jpg'
  };
  print(name);
  print(year);
  print(antennaDiameter);
  print(flybyObjects);
  print(image);
```


&emsp;可以使用 `+` 运算符或 **并列放置多个字符串来连接字符串**：这里和我们学的其他语言的字符串拼接不同，Dart 只要把字符串并列放置也能进行字符串拼接。

```c++
  var s1 = 'String '
      'concatenation'
      " works even over line breaks.";

  var s2 = 'String ' 'concatenation' " works even over line breaks.";
```

&emsp;使用 **三个单引号** 或者 **三个双引号** 能创建多行字符串：

```c++
  var s3 = '''
You can create
multi-line strings like this one.
''';

  var s4 = """This is also a
multi-line string.""";
```

&emsp;在字符串前加上 `r` 作为前缀创建 "raw" 字符串，会保留字符串串中的特殊字符串，比如 `\n` 被保留为 `\n` 字符，而不是作换行使用（即不会被做任何处理（比如转义）的字符串）：

```c++
var s5 = r'In a raw string, not even \n gets special treatment.';
```

&emsp;使用 VS Code 安装 Dart 扩展，然后新建一个 .dart 文件直接开始 Dart 编程！ 

&emsp;每个应用都有一个 `main()` 函数。可以使用顶层（top-level）函数 `print()` 来将一段文本输出显示到控制台：

```dart
void main() {
  print('Hello, World!');
}
```

&emsp;虽然 Dart 是代码类型安全的语言，但是由于其支持类型推断，因此大多数变量不需要显式地指定类型：

```dart
  var name = 'Voyager I';
  var year = 1977;
  var antennaDiameter = 3.7;
  var flybyObjects = ['Jupiter', 'Saturn', 'Uranus', 'Neptune'];
  var image = {
    'tags': ['saturn'],
    'url': '//path/to/saturn.jpg'
  };
```

&emsp;Mixin 是一种在多个类层次结构中重用代码的方法。下面的是声明一个 Mixin 的做法：

```c++
mixin Piloted {
  int astronauts = 1;

  void describeCrew() {
    print('Number of astronauts: $astronauts');
  }
}

class Orbiter extends Spacecraft with Piloted {
    // 
}
```



## 参考链接
**参考链接:🔗**
+ [Dart 编程语言概览](https://dart.cn/samples)
+ [Dart 开发语言概览](https://dart.cn/guides/language/language-tour)

