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

&emsp;后面我们会学习更多 Dart 中变量相关的知识点，包括变量的默认值，final 和 const 关键字以及静态类型等。

## 流程控制语句

&emsp;和其它语言的流程控制语句基本一样。

```dart
  if (year >= 2001) {
    print('21st century');
  } else if (year >= 1901) {
    print('20th century');
  }

  for (final object in flybyObjects) {
    print(object);
  }

  for (int month = 1; month <= 12; month++) {
    print(month);
  }

  while (year < 2016) {
    year += 1;
  }
```

## 函数

&emsp;建议为每个函数的参数以及返回值都指定类型：

```dart
int fibonacci(int n) {
  if (n == 0 || n == 1) {
    return n;
  }

  return fibonacci(n - 1) + fibonacci(n - 2);
}
```

&emsp;=> (胖箭头) 简写语法用于仅包含一条语句的函数。该语法在将匿名函数作为参数传递时非常有用：

```dart
void main(List<String> args) {
  var flybyObjects = ['Jupiter', 'Saturn', 'Uranus', 'Neptune'];
  flybyObjects.where((element) => element.contains('turn')).forEach(print);
}
```

&emsp;上面的示例展示了匿名函数：传入 `where()` 函数的参数即是一个匿名函数，还展示了将函数作为参数使用的方式：将顶层函数 `print()` 作为参数传给了 `forEach()` 函数。

## 注释

&emsp;Dart 通常使用双斜杠 `//` 作为注释的开始。

```dart
/// 这是一个文档注释。
/// 文档注释用于为库、类以及类的成员添加注释。
/// 像 IDE 和 dartdoc 这样的工具可以专门处理文档注释。

/* 也可以像这样使用单斜杠和星号的注释方式 */
```

## 导入（ Import ）

&emsp;使用 `import` 关键字来访问在其它库中定义的 API。

```dart
// Importing core libraries
import 'dart:math';

// Importing libraries from external packages
import 'package:test/test.dart';

// Importing files
import 'path/to/my_other_file.dart';
```

## 类（ Class ）

&emsp;下面的示例展示了一个包含三个属性、两个构造函数以及一个方法的类。其中一个属性不能直接赋值，因此它被定义为一个 getter 方法（而不是变量）。

```dart
class Spacecraft {
  String name;
  DateTime? launchDate;

  // Read-only non-final property
  int? get launchYear => launchDate?.year;

  // Constructor，with syntactic sugar for assignment to members.
  Spacecraft(this.name, this.launchDate) {
    // Initialization code goes here.
  }

  // Named constructor that forwards to the default one.
  Spacecraft.unlaunched(String name) : this(name, null);

  // Method.
  void describe() {
    print('Spacecraft: $name');
    // Type promotion doesn't work on getters.
    var launchDate = this.launchDate;
    if (launchDate != null) {
      int years = DateTime.now().difference(launchDate).inDays ~/ 365;
      print('Launched: $launchYear ($years years ago)');
    } else {
      print('Unlaunched');
    }
  }
}
```

&emsp;可以像下面这样使用 Spacecraft 类：

```dart
var voyager = Spacecraft('Voyager I', DateTime(1977, 9, 5));
voyager.describe();

var voyager3 = Spacecraft.unlaunched('Voyager III');
voyager3.describe();
```

&emsp;打印如下：

```dart
Spacecraft: Voyager I
Launched: 1977 (44 years ago)
Spacecraft: Voyager III
Unlaunched
```

## 扩展类（继承）

&emsp;Dart 支持单继承。

```dart
class Orbiter extends Spacecraft {
  double altitude;

  Orbiter(String name, DateTime launchDate, this.altitude)
      : super(name, launchDate);
}
```

## Mixins

&emsp;Mixin 是一种在多个类层次结构中重用代码的方法。下面的是声明一个 Mixin 的做法：

```dart
mixin Piloted {
  int astronauts = 1;

  void describeCrew() {
    print('Number of astronauts: $astronauts');
  }
}
```

&emsp;现在只需使用 Mixin 的方式继承这个类就可将该类中的功能添加给其它类。

```dart
class PilotedCraft extends Spacecraft with Piloted {
  PilotedCraft(String name, DateTime? launchDate) : super(name, launchDate);
  //...
}
```

&emsp;自此，PilotedCraft 类中就包含了 astronauts 字段以及 describeCrew() 方法。

## 接口和抽象类

&emsp;Dart 没有 interface 关键字。相反，所有的类都隐式定义了一个接口。因此，任意类都可以作为接口被实现。

```dart
class MockSpaceship implements Spacecraft {
  @override
  DateTime? launchDate;

  @override
  late String name;

  @override
  void describe() {
    // TODO: implement describe
  }

  @override
  // TODO: implement launchYear
  int? get launchYear => throw UnimplementedError();
  // ...
}
```

&emsp;可以创建一个被任意具体类扩展（或实现）的抽象类。抽象类可以包含抽象方法（不含方法体的方法）。

```dart
abstract class Describable {
  void describe();

  void describeWithEmphasis() {
    print('======');
    describe();
    print('======');
  }
}
```

&emsp;任意一个扩展了 `Describable` 的类都拥有 `describeWithEmphasis()` 方法，这个方法又会去调用实现类中实现的 `describe()` 方法。

## 异步

&emsp;使用 async 和 await 关键字可以避免回调地狱（Callback Hell）并使代码更具可读性。

```dart
const oneSecond = Duration(seconds: 1);
// ...
Future<void> printWithDelay(String message) async {
  await Future.delayed(oneSecond);
  print(message);
}
```

&emsp;上面的方法相当于：

```dart
Future<void> printWithDelay(String message) {
  return Future.delayed(oneSecond).then((_) => print(message));
}
```

&emsp;如下一个示例所示，async 和 await 关键字有助于使异步代码变得易于阅读。

```dart
Future<void> createDescriptions(Iterable<String> objects) async {
  for (final object in objects) {
    try {
      var file = File('$object.txt');
      if (await file.exists()) {
        var modified = await file.lastModified();
        print('File for $object already exists. It was modified on $modified.');
        continue;
      }
      await file.create();
      await file.writeAsString('Start describing $object in this file.');
    } on IOException catch (e) {
      print('Cannot create description for $object: $e');
    }
  }
}
```

&emsp;也可以使用 `async*` 关键字，可以提供一个可读性更好的方式去生成 Stream。

```dart
Stream<String> report(Spacecraft craft, Iterable<String> objects) async* {
  for (final object in objects) {
    await Future.delayed(oneSecond);
    yield '${craft.name} files by $object';
  }
}
```

## 异常

&emsp;使用 throw 关键字抛出一个异常：

```dart
if (astronauts == 0) {
  throw StateError('No astronauts.');
}
```

&emsp;使用 try 语句配合 on 或 catch（两者也可同时使用）关键字来捕获一个异常:

```dart
try {
  for (final object in flybyObjects) {
    var description = await File('$object.txt').readAsString();
    print(description);
  }
} on IOException catch (e) {
  print('Could not describe object: $e');
} finally {
  flybyObjects.clear();
}
```

&emsp;注意上述代码是异步的；同步代码以及异步函数中的代码都可以使用 try 捕获异常。




































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

