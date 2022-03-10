# TypeScript 学习笔记

&emsp;TypeScript 是 JavaScript 的一个超集，支持 ECMAScript 6 标准（ES6 教程）。TypeScript 由微软开发的自由和开源的编程语言。TypeScript 设计目标是开发大型应用，它可以编译成纯 JavaScript，编译出来的 JavaScript 可以运行在任何浏览器上。（Dart 也可以转换为 JavaScript）

## 语言特性

&emsp;TypeScript 是一种给 JavaScript 添加特性的语言扩展。增加的功能包括：

+ 类型批注和编译时类型检查
+ 类型推断
+ 类型擦除
+ 接口
+ 枚举
+ Mixin
+ 泛型编程
+ 名字空间
+ 元组
+ Await

&emsp;以下功能是从 ECMA 2015 反向移植而来：

+ 类
+ 模块
+ lambda 函数的箭头语法
+ 可选参数以及默认参数

## JavaScript 与 TypeScript 的区别

&emsp;TypeScript 是 JavaScript 的超集，扩展了 JavaScript 的语法，因此现有的 JavaScript 代码可与 TypeScript 一起工作无需任何修改，TypeScript 通过类型注解提供编译时的静态类型检查。TypeScript 可处理已有的 JavaScript 代码，并只对其中的 TypeScript 代码进行编译。

```TypeScript
const hello: string = "Hello TypeScript!"
console.log(hello)
```

## TypeScript 安装

&emsp;使用 `npm install -g typescript` 命令安装 TypeScript。

```c++
➜  ~ npm install -g typescript
/usr/local/bin/tsc -> /usr/local/lib/node_modules/typescript/bin/tsc
/usr/local/bin/tsserver -> /usr/local/lib/node_modules/typescript/bin/tsserver
+ typescript@4.6.2
added 1 package from 1 contributor in 1.751s
➜  ~ tsc -v
Version 4.6.2
➜  ~ node -v
v14.16.0
```

&emsp;使用 `tsc xxx.ts` 命令将 TypeScript 转换为 JavaScript 代码。

&emsp;使用 `node xxx.js` 命令来执行 xxx.js 文件。

## TypeScript 基础语法

&emsp;TypeScript 程序由以下几个部分组成：

+ 模块
+ 函数
+ 变量
+ 语句和表达式
+ 注释

### 第一个 TypeScript 程序

&emsp;使用以下 TypeScript 程序来输出 "Hello TypeScript!" ：

```typescript
const hello : string = "Hello TypeScript!"
console.log(hello)
```

&emsp;把以上代码保存在 FirstTypeScript.ts 文件中，然后在 VS Code 的左侧资源管理器中选中该文件右键选择 open in command prompt（在终端中打开），然后便可看到 VS Code 底部的终端页面显示进入了我们 FirstTypeScript.ts 文件所在的文件夹，然后在终端中执行：`tsc FirstTypeScript.ts` 命令，便可在当前文件夹下面得到一个 `FirstTypeScript.js` 文件，然后执行：`node FirstTypeScript.js` 命令，终端便会打印：Hello TypeScript! 字符串。

&emsp;打开 `FirstTypeScript.js` 文件看到上面的 TypeScript 代码被编译转换为了如下的 JavaScript 代码：

```javascript
var hello = "Hello TypeScript!";
console.log(hello);
```

#### 空白和换行

&emsp;TypeScript 会忽略程序中出现的空格、制表符和换行符。空格、制表符通常用来缩进代码，使代码易于阅读和理解。

#### TypeScript 区分大小写

&emsp;TypeScript 区分大写和小写字符。

#### 分号是可选的

&emsp;每行指令都是一段语句，你可以使用分号或不使用，分号在 TypeScript 中是可选的，建议使用。以下代码都是合法的：

```typescript
console.log("Runoob")
console.log("Google");
```

&emsp;如果语句写在同一行则一定需要使用分号来分隔，否则会报错，如：

```typescript
console.log("Runoob");console.log("Google");
```

#### TypeScript 注释

&emsp;注释是一个良好的习惯，虽然很多程序员讨厌注释，但还是建议你在每段代码写上文字说明。注释可以提高程序的可读性。注释可以包含有关程序一些信息，如代码的作者，有关函数的说明等。编译器会忽略注释。

&emsp;TypeScript 支持两种类型的注释

+ 单行注释 ( // ) − 在 // 后面的文字都是注释内容。
+ 多行注释 (/* */) − 这种注释可以跨越多行。

&emsp;注释实例：

```typescript
// 这是一个单行注释
 
/* 
 这是一个多行注释 
 这是一个多行注释 
 这是一个多行注释 
*/
```

#### TypeScript 与面向对象

&emsp;面向对象是一种对现实世界理解和抽象的方法。TypeScript 是一种面向对象的编程语言。

&emsp;面向对象主要有两个概念：对象和类。

+ 对象：对象是类的一个实例，有状态和行为。例如，一条狗是一个对象，它的状态有：颜色、名字、品种；行为有：摇尾巴、叫、吃等。
+ 类：类是一个模板，它描述一类对象的行为和状态。
+ 方法：方法是类的操作的实现步骤。

&emsp;TypeScript 面向对象编程实例：

```c++
class Site { 
   name():void { 
      console.log("Runoob") 
   } 
}

var obj = new Site(); 
obj.name();
```

&emsp;以上实例定义了一个类 `Site`，该类有一个方法 `name()`，该方法在终端上输出字符串 `Runoob`。`new` 关键字创建类的对象，该对象调用方法 `name()`。编译后生成的 JavaScript 代码如下：

```javascript
var Site = /** @class */ (function () {
    function Site() {
    }
    Site.prototype.name = function () {
        console.log("Runoob");
    };
    return Site;
}());
var obj = new Site();
obj.name();
```

## TypeScript 基础类型

+ 任意类型 any 声明为 any 的变量可以赋予任意类型的值
+ 数字类型 number 双精度 64 位浮点值。它可以用来表示整数和分数

```typescript
let binaryLiteral: number = 0b1010; // 0b 开头 二进制
let octalLiteral: number = 0o744; // 0o 开头 八进制
let decLiteral: number = 6; // 十进制
let hexLiteral: number = 0xf00d; // 0x 十六进制
```

+ 字符串类型 string 一个字符系列，使用单引号（'）或双引号（"）来表示字符串类型，反引号（`）来定义多行文本和内嵌表达式

```typescript
let name: string = "Runoob";
let years: number = 5;
let words: string = `您好，今年是 ${ name } 发布 ${ years + 1} 周年`;
```

+ 布尔类型 boolean 表示逻辑值：true 和 false

```typescript
let flag: boolean = true;
```

+ 数组类型 - 声明变量为数组：

```typescript
// 在元素类型后面加上[]
let arr: number[] = [1, 2];

// 或者使用数组泛型
let arr: Array<number> = [1, 2];
```

+ 元组 - 元组类型用来表示已知元素数量和类型的数组，各元素的类型不必相同，对应位置的类型需要相同：

```typescript
let x: [string, number];
x = ['Runoob', 1];    // 运行正常
x = [1, 'Runoob'];    // 报错
console.log(x[0]);    // 输出 Runoob
```

+ 枚举 enum 枚举类型用于定义数值集合：

```typescript
enum Color { Red, Green, Blue };
let c: Color = Color.Blue;
console.log(c);    // 输出 2
```

+ void void 用于标识方法返回值的类型，表示该方法没有返回值

```c++
function hello(): void {
    alert("Hello Runoob");
}
```

+ null null 表示对象值缺失
+ undefined undefined 用于初始化变量为一个未定义的值
+ never never never 是其它类型（包括 null 和 undefined）的子类型，代表从不会出现的值

> &emsp;TypeScript 和 JavaScript 没有整数类型。

### Any 类型

&emsp;任意值是 TypeScript 针对编程时类型不明确的变量使用的一种数据类型，它常用于以下三种情况。

1. 变量的值会动态改变时，比如来自用户的输入，任意值类型可以让这些变量跳过编译阶段的类型检查，示例代码如下：

```typescript
let x: any = 1;    // 数字类型
x = 'I am who I am';    // 字符串类型
x = false;    // 布尔类型
```

2. 改写现有代码时，任意值允许在编译时可选择地包含或移除类型检查，示例代码如下：

```c++
let x: any = 4;
x.ifItExists();    // 正确，ifItExists 方法在运行时可能存在，但这里并不会检查
x.toFixed();    // 正确
```

3. 定义存储各种类型数据的数组时，示例代码如下：

```c++
let arrayList: any[] = [1, false, 'fine'];
arrayList[1] = 100;
```

### Null 和 Undefined

#### null

&emsp;在 JavaScript 中 `null` 表示 "什么都没有"。`null` 是一个只有一个值的特殊类型。表示一个空对象引用。用 `typeof` 检测 `null` 返回是 `object`。

#### undefined

&emsp;在 JavaScript 中，`undefined` 是一个没有设置值的变量。`typeof` 一个没有值的变量会返回 `undefined`。Null 和 Undefined 是其他任何类型（包括 `void`）的子类型，可以赋值给其它类型，如数字类型，此时，赋值后的类型会变成 `null` 或 `undefined`。而在 TypeScript 中启用严格的空校验（--strictNullChecks）特性，就可以使得 `null` 和 `undefined` 只能被赋值给 `void` 或本身对应的类型，示例代码如下：




























## 参考链接
**参考链接:🔗**
+ [TypeScript 教程](https://www.runoob.com/typescript/ts-tutorial.html)
