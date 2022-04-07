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

&emsp;在 JavaScript 中 `null` 表示 "什么都没有"。`null` 是一个只有一个值的特殊类型，表示一个空对象引用。用 `typeof` 检测 `null` 返回是 `object`。

#### undefined

&emsp;在 JavaScript 中，`undefined` 是一个没有设置值的变量。`typeof` 一个没有值的变量会返回 `undefined`。Null 和 Undefined 是其他任何类型（包括 `void`）的子类型，可以赋值给其它类型，如数字类型，此时，赋值后的类型会变成 `null` 或 `undefined`。而在 TypeScript 中启用严格的空校验（--strictNullChecks）特性，就可以使得 `null` 和 `undefined` 只能被赋值给 `void` 或本身对应的类型，示例代码如下：

```typescript
// 启用 --strictNullChecks
let x: number;
x = 1; // 运行正确
x = undefined; // 运行错误
x = null; // 运行错误
```

&emsp;上面的例子中变量 x 只能是数字类型。如果一个类型可能出现 `null` 或 `undefined`， 可以用 `|` 来支持多种类型，示例代码如下：

```c++
// 启用 --strictNullChecks
let x: number | null | undefined;
x = 1; // 运行正确
x = undefined; // 运行正确
x = null; // 运行正确
```

#### never 类型

&emsp;`never` 是其它类型（包括 `null` 和 `undefined`）的子类型，代表从不会出现的值。这意味着声明为 `never` 类型的变量只能被 `never` 类型所赋值，在函数中它通常表现为抛出异常或无法执行到终止点（例如无限循环），示例代码如下：

```c++
let x: never;
let y: number;

// 运行错误，数字类型不能转为 never 类型
x = 123; // 报错信息：.ts:22:1 - error TS2322: Type 'number' is not assignable to type 'never'.

// 运行正确，never 类型可以赋值给 never 类型
x = (()=>{ throw new Error('exception')})();

// 运行正确，never 类型可以赋值给 数字类型
y = (()=>{ throw new Error('exception')})();

// 返回值为 never 的函数可以是抛出异常的情况
function error(message: string): never {
    throw new Error(message);
}

// 返回值为 never 的函数可以是无法被执行到的终止点的情况
function loop(): never {
    while (true) {}
}
```

```typescript
const getValue = () => {
  return 0
}

enum List {
  A = getValue(),
  B = 2, // 此处必须要初始化值，不然编译不通过，B 和 C 两行都会报错：46:5 - error TS1061: Enum member must have initializer.
  C // 47:5 - error TS1061: Enum member must have initializer.
}
console.log(List.A) // 0
console.log(List.B) // 2
console.log(List.C) // 3
```

&emsp;`A` 的值是被计算出来的，注意注释部分，如果某个属性的值是计算出来的，那么它后面一位的成员必须要初始化值。

## TypeScript 变量声明

&emsp;变量是一种使用方便的占位符，用于引用计算机内存地址。我们可以把变量看做存储数据的容器。TypeScript 变量的命名规则：

+ 变量名称可以包含数字和字母。
+ 除了下划线 `_` 和美元 `$` 符号外，不能包含其他特殊字符，包括空格。
+ 变量名不能以数字开头。

&emsp;变量使用前必须先声明，我们可以使用 `var` 来声明变量。我们可以使用以下四种方式来声明变量：

1. 声明变量的类型及初始值：

```typescript
var [变量名] : [类型] = 值;
var uname: string = "Runoob";
```

2. 声明变量的类型，但没有初始值，变量值会设置为 `undefined`：

```typescript
var [变量名] : [类型];
var uname:string;
```

3. 声明变量并初始值，但不设置类型，该变量可以是任意类型：

```typescript
var [变量名] = 值;
var uname = "Runoob";
```

4. 声明变量没有设置类型和初始值，类型可以是任意类型，默认初始值为 `undefined`：

```typescript
var [变量名];
var uname;
```

&emsp;一些实例：

```typescript
var uname: string = "Runoob";
var score1: number = 50;
var score2: number = 42.5;
var sum = score1 + score2;
console.log("名字: " + uname);
console.log("第一个科目成绩: " + score1);
console.log("第二个科目成绩: " + score2);
console.log("总成绩: " + sum);
```

&emsp;变量名不要使用 `name` 否则会与 DOM 中的全局 `window` 对象下的 `name` 属性出现了重名。

&emsp;使用 `tsc` 命令编译以上代码，得到如下 JavaScript 代码：

```javascript
var uname = "Runoob";
var score1 = 50;
var score2 = 42.50;
var sum = score1 + score2;
console.log("名字: " + uname);
console.log("第一个科目成绩: " + score1);
console.log("第二个科目成绩: " + score2);
console.log("总成绩: " + sum);
```

&emsp;TypeScript 遵循强类型，如果将不同的类型赋值给变量会编译错误，如下实例：

```typescript
var num: number = "hello"; // 这个代码会编译错误，报错信息：Type 'string' is not assignable to type 'number'.
```

### 类型断言（Type Assertion）

&emsp;类型断言可以用来手动指定一个值的类型，即允许变量从一种类型更改为另一种类型。语法格式：

```c++
<类型>值 
// 或者：
值 as 类型
```

&emsp;实例：

```c++
var str = '1' 
var str2: number = <number> <any> str   // str、str2 是 string 类型
console.log(str2)
```

#### TypeScript 是怎么确定单个断言是否足够

&emsp;当 S 类型是 T 类型的子集，或者 T 类型是 S 类型的子集时，S 能被成功断言成 T。这是为了在进行类型断言时提供额外的安全性，完全毫无根据的断言是危险的，如果你想这么做，你可以使用 `any`。它之所以不被称为类型转换，是因为转换通常意味着某种运行时的支持。但是，**类型断言纯粹是一个编译时语法**，同时，它也是一种为编译器提供关于如何分析代码的方法。编译后，以上代码会生成如下 JavaScript 代码：

```javascript
var str = '1';
var str2 = str; // str、str2 是 string 类型
console.log(str2);
```

&emsp;执行打印 1。

### 类型推断

&emsp;当类型没有给出时，TypeScript 编译器利用类型推断来推断类型。如果由于缺乏声明而不能推断出类型，那么它的类型被视作默认的动态 `any` 类型。

```typescript
var num = 2; // 类型推断为 number
console.log("num 变量的值为：" + num); 
num = "12"; // 编译错误：Type 'string' is not assignable to type 'number'.
console.log(num);
```

&emsp;第一行代码声明了变量 `num` 并 = 设置初始值为 2。注意变量声明没有指定类型。因此，程序使用 **类型推断** 来确定变量的数据类型，第一次赋值为 2，`num` 设置为 `number` 类型。

&emsp;第三行代码，当我们再次为变量设置字符串类型的值时，这时编译会错误。因为变量已经设置为了 `number` 类型。

### 变量作用域

&emsp;变量作用域指定了变量定义的位置。程序中变量的可用性由变量作用域决定。TypeScript 有以下几种作用域：

+ 全局作用域 − 全局变量定义在程序结构的外部，它可以在你代码的任何位置使用。
+ 类作用域 − 这个变量也可以称为字段。类变量声明在一个类里头，但在类的方法外面。 该变量可以通过类的对象来访问。类变量也可以是静态的，静态的变量可以通过类名直接访问。
+ 局部作用域 − 局部变量，局部变量只能在声明它的一个代码块（如：方法）中使用。

&emsp;以下实例说明了三种作用域的使用：

```typescript
var global_num = 12; // 全局变量
class Numbers {
  num_val = 13; // 实例变量
  static sval = 10; // 静态变量

  storeNum(): void {
    var local_num = 14; // 局部变量
  }
}
console.log("全局变量为：" + global_num);
console.log("静态变量为：" + Numbers.sval);

var obj = new Numbers();
console.log("实例变量：" + obj.num_val);
```

&emsp;以上代码使用 tsc 命令编译为 JavaScript 代码为：

```javascript
var global_num = 12; // 全局变量
var Numbers = /** @class */ (function () {
    function Numbers() {
        this.num_val = 13; // 实例变量
    }
    Numbers.prototype.storeNum = function () {
        var local_num = 14; // 局部变量
    };
    Numbers.sval = 10; // 静态变量
    return Numbers;
}());
console.log("全局变量为：" + global_num);
console.log("静态变量为：" + Numbers.sval);
var obj = new Numbers();
console.log("实例变量：" + obj.num_val);
```

&emsp;执行以上代码结果：

```javascript
全局变量为：12
静态变量为：10
实例变量：13
```

&emsp;如果我们在方法外部调用局部变量 `local_num` 会报错：Could not find symbol 'local_num'.

## TypeScript 运算符

&emsp;运算符用于执行程序代码运算，会针对一个以上操作数项目来进行运算。考虑以下计算：

```c++
7 + 5 = 12
```

&emsp;以上实例中 7、5 和 12 是操作数。运算符 + 用于加值。运算符 = 用于赋值。

&emsp;TypeScript 主要包含以下几种运算：

+ 算术运算符
+ 逻辑运算符
+ 关系运算符
+ 按位运算符
+ 赋值运算符
+ 三元/条件运算符
+ 字符串运算符
+ 类型运算符

### 算术运算符

&emsp;`+`、`-`、`*`、`/`、`%`、`++`、`--`。

### 关系运算符

&emsp;关系运算符用于计算结果是否为 true 或者 false。`==`、`!=`、`>`、`<`、`>=`、`<=`。

### 逻辑运算符

&emsp;逻辑运算符用于测定变量或值之间的逻辑。`&&`、`||`、`!`。

#### 短路运算符(&& 与 ||)

&emsp;`&&` 与 `||` 运算符可用于组合表达式。`&&` 运算符只有在左右两个表达式都为 `true` 时才返回 `true`。考虑以下实例：

```typescript
var a = 10;
var result = a < 10 && a > 5;
```

&emsp;以上实例中 `a < 10` 与 `a > 5` 是使用了 `&&` 运算符的组合表达式，第一个表达式返回了 `false`，由于 `&&` 运算需要两个表达式都为 `true`，所以如果第一个为 `false`，就不再执行后面的判断（`a > 5` 跳过计算），直接返回 `false`。`||` 运算符只要其中一个表达式为 `true`，则该组合表达式就会返回 `true`。考虑以下实例：

```typescript
var a = 10;
var result = a > 5 || a < 10;
```

&emsp;以上实例中 `a > 5` 与 `a < 10` 是使用了 `||` 运算符的组合表达式，第一个表达式返回了 `true`，由于 `||` 组合运算只需要一个表达式为 `true`，所以如果第一个为 `true`，就不再执行后面的判断（`a < 10` 跳过计算），直接返回 `true`。

### 位运算符

&emsp;位操作是程序设计中对位模式按位或二进制数的一元和二元操作。`&`、`|`、`~`、`^`、`<<`、`>>`、`>>>`（无符号右移，与有符号右移位类似，除了左边一律使用 0 补位）。 

### 赋值运算符

&emsp;赋值运算符用于给变量赋值。`=`、`+=`、`-=`、`*=`、`/=`。

### 三元运算符 (?)

&emsp;三元运算有 3 个操作数，并且需要判断布尔表达式的值。该运算符的主要是决定哪个值应该赋值给变量。

```typescript
Test ? expr1 : expr2
```

&emsp;Test − 指定的条件语句，expr1 − 如果条件语句 Test 返回 true 则返回该值，expr2 − 如果条件语句 Test 返回 false 则返回该值。

### 类型运算符

&emsp;`typeof` 运算符：`typeof` 是一元运算符，返回操作数的数据类型。

&emsp;`instanceof` 运算符：`instanceof` 运算符用于判断对象是否为指定的类型。

### 其他运算符

&emsp;负号运算符(`-`)：更改操作数的符号。

&emsp;字符串运算符: 连接运算符 (`+`) 可以拼接两个字符串。

## TypeScript 条件语句

&emsp;条件语句用于基于不同的条件来执行不同的动作。TypeScript 条件语句是通过一条或多条语句的执行结果（True 或 False）来决定执行的代码块。

&emsp;switch 语句必须遵循下面的规则：

+ switch 语句中的 expression 是一个常量表达式，必须是一个整型或枚举类型。
+ 在一个 switch 中可以有任意数量的 case 语句。每个 case 后跟一个要比较的值和一个冒号。
+ case 的 constant-expression 必须与 switch 中的变量具有相同的数据类型，且必须是一个常量或字面量。
+ 当被测试的变量等于 case 中的常量时，case 后跟的语句将被执行，直到遇到 break 语句为止。
+ 当遇到 break 语句时，switch 终止，控制流将跳转到 switch 语句后的下一行。
+ 不是每一个 case 都需要包含 break。如果 case 语句不包含 break，控制流将会 继续 后续的 case，直到遇到 break 为止。
+ 一个 switch 语句可以有一个可选的 default case，出现在 switch 的结尾。default case 可用于在上面所有 case 都不为真时执行一个任务。default case 中的 break 语句不是必需的。

## TypeScript 循环

&emsp;有的时候，我们可能需要多次执行同一块代码。一般情况下，语句是按顺序执行的：函数中的第一个语句先执行，接着是第二个语句，依此类推。编程语言提供了更为复杂执行路径的多种控制结构。循环语句允许我们多次执行一个语句或语句组。

### for…of 、forEach、every 和 some 循环

&emsp;TypeScript 还支持 for…of 、forEach、every 和 some 循环。

&emsp;for...of 语句创建一个循环来迭代可迭代的对象。在 ES6 中引入的 for...of 循环，以替代 for...in 和 forEach()，并支持新的迭代协议。for...of 允许你遍历 Arrays（数组）， Strings（字符串），Maps（映射），Sets（集合）等可迭代的数据结构等。

&emsp;TypeScript for...of 循环

```typescript
let someArray = [1, "string", false];

for (let entry of someArray) {
  console.log(entry); // 1, "string", false
}
```

&emsp;forEach、every 和 some 是 JavaScript 的循环语法，TypeScript 作为 JavaScript 的语法超集，当然默认也是支持的。因为 forEach 在 iteration 中是无法返回的，所以可以使用 every 和 some 来取代 forEach。

&emsp;TypeScript forEach 循环

```typescript
let list = [4, 5, 6];
list.forEach((val, idx, array) => {
  // val: 当前值
  // idx：当前 index
  // array: Array
});
```

&emsp;TypeScript every 循环

```typescript
let list = [4, 5, 6];
list.every((val, idx, array) => {
  // val: 当前值
  // idx：当前 index
  // array: Array
  return true; // Continues
  // Return false will quit the iteration
});
```

&emsp;`break` 语句有以下两种用法：

+ 当 break 语句出现在一个循环内时，循环会立即终止，且程序流将继续执行紧接着循环的下一条语句。
+ 它可用于终止 switch 语句中的一个 case。

&emsp;如果使用的是嵌套循环（即一个循环内嵌套另一个循环），break 语句会停止执行最内层的循环，然后开始执行该块之后的下一行代码。

&emsp;continue 语句有点像 break 语句。但它不是强制终止，continue 会跳过当前循环中的代码，强迫开始下一次循环。对于 for 循环，continue 语句执行后自增语句仍然会执行。对于 while 和 do...while 循环，continue 语句重新执行条件判断语句。

## TypeScript 函数

&emsp;函数是一组一起执行一个任务的语句。可以把代码划分到不同的函数中。如何划分代码到不同的函数中是由你来决定的，但在逻辑上，划分通常是根据每个函数执行一个特定的任务来进行的。函数声明告诉编译器函数的名称、返回类型和参数。函数定义提供了函数的实际主体。

### 可选参数

&emsp;在 TypeScript 函数里，如果我们定义了参数，则我们必须传入这些参数，除非将这些参数设置为可选，可选参数使用问号标识 `?`。

```typescript
function buildName(firstName: string, lastName: string) {
  return firstName + " " + lastName;
}

let result1 = buildName("Bob"); // 错误，缺少参数：Expected 2 arguments, but got 1.
let result2 = buildName("Bob", "Adams", "Sr."); // 错误，参数太多了：Expected 2 arguments, but got 3.
let result3 = buildName("Bob", "Adams"); // 正确
```

&emsp;以下实例，我们将 `lastName` 设置为可选参数：

```typescript
function buildName(firstName: string, lastName?: string) {
  if (lastName) {
    return firstName + " " + lastName;
  } else {
    return firstName;
  }
}

let result1 = buildName("Bob"); // 正确
let result2 = buildName("Bob", "Adams", "Sr."); // 错误，参数太多了：Expected 1-2 arguments, but got 3.
let result3 = buildName("Bob", "Adams"); // 正确
```

&emsp;可选参数必须跟在必需参数后面。如果上例我们想让 `firstName` 是可选的，`lastName` 必选，那么就要调整它们的位置，把 `firstName` 放在后面。如果都是可选参数就没关系。

### 默认参数

&emsp;我们也可以设置参数的默认值，这样在调用函数的时候，如果不传入该参数的值，则使用默认参数，语法格式为：

```typescript
function function_name(param1[:type], param2[:type] = default_value) {
    // ...
}
```

&emsp;参数不能同时设置为可选和默认。

&emsp;以下实例函数的参数 `rate` 设置了默认值为 `0.50`，调用该函数时如果未传入参数则使用该默认值：

```typescript
function calculate_discount(price: number, rate: number = 0.5) {
  var discount = price * rate;
  console.log("计算结果：", discount);
}

calculate_discount(1000);
calculate_discount(1000, 0.3);
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
function calculate_discount(price, rate) {
    if (rate === void 0) { rate = 0.5; }
    var discount = price * rate;
    console.log("计算结果：", discount);
}
calculate_discount(1000);
calculate_discount(1000, 0.3);
```

&emsp;输出结果：

```c++
// 计算结果： 500
// 计算结果： 300
```

### 剩余参数

&emsp;有一种情况，我们不知道要向函数传入多少个参数，这时候我们就可以使用剩余参数来定义。剩余参数语法允许我们将一个不确定数量的参数作为一个数组传入。

```typescript
function buildName(firstName: string, ...restOfName: string[]) {
  return firstName + " " + restOfName.join(" ");
}

let employeeName = buildName("Joseph", "Samuel", "Lucas", "MacKinzie");
```

&emsp;对应的 JavaScript 代码：

```javascript
function buildName(firstName) {
    var restOfName = [];
    for (var _i = 1; _i < arguments.length; _i++) {
        restOfName[_i - 1] = arguments[_i];
    }
    return firstName + " " + restOfName.join(" ");
}
var employeeName = buildName("Joseph", "Samuel", "Lucas", "MacKinzie");
```

&emsp;函数的最后一个命名参数 `restOfName` 以 `...` 为前缀，它将成为一个由剩余参数组成的数组，索引值从 0（包括）到 `restOfName.length`（不包括）。

```typescript
function addNumbers(...nums: number[]) {
  var i: number;
  var sum: number = 0;
  for (i = 0; i < nums.length; i++) {
    sum = sum + nums[i];
  }
  console.log("和为：", sum);
}

addNumbers(1, 2, 3);
addNumbers(10, 10, 10, 10, 10);
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
function addNumbers() {
    var nums = [];
    for (var _i = 0; _i < arguments.length; _i++) {
        nums[_i] = arguments[_i];
    }
    var i;
    var sum = 0;
    for (i = 0; i < nums.length; i++) {
        sum = sum + nums[i];
    }
    console.log("和为：", sum);
}
addNumbers(1, 2, 3);
addNumbers(10, 10, 10, 10, 10);
```

### 匿名函数

&emsp;匿名函数是一个没有函数名的函数。匿名函数在程序运行时动态声明，除了没有函数名外，其他的与标准函数一样。我们可以将匿名函数赋值给一个变量，这种表达式就成为函数表达式。

&emsp;语法格式如下：

```typescript
var res = function( [arguments] ) { ... }
```

#### 实例

&emsp;不带参数匿名函数：

```typescript
var msg = function () {
  return "Hello, TypeScript";
}
console.log(msg());
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
var msg = function () {
    return "Hello, TypeScript";
};
console.log(msg());
```

&emsp;看到 TypeScript 和 JavaScript 二者一模一样。

&emsp;带参数匿名函数：

```typescript
var res = function (a: number, b: number) {
  return a * b;
};
console.log(res(12, 2));
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
var res = function (a, b) {
    return a * b;
};
console.log(res(12, 2));
```

#### 匿名函数自调用

&emsp;匿名函数自调用在函数后使用 `()` 即可：

```typescript
(function () {
  var x = "Hello, TypeScript";
  console.log(x);
})()
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
(function () {
    var x = "Hello, TypeScript";
    console.log(x);
})();
```

### 构造函数

&emsp;TypeScript 也支持使用 JavaScript 内置的构造函数 `Function()` 来定义函数：语法格式如下：

```typescript
var res = new Function ([arg1[, arg2[, ...argN]],] functionBody)
```

&emsp;参数说明：

+ arg1, arg2, ... argN：参数列表。
+ functionBody：一个含有包括函数定义的 JavaScript 语句的字符串。

```typescript
var myFunction = new Function("a", "b", "return a * b");
var x = myFunction(4, 3);
console.log(x);
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
var myFunction = new Function("a", "b", "return a * b");
var x = myFunction(4, 3);
console.log(x);
```

### 递归函数

&emsp;递归函数即在函数内调用函数本身。

```typescript
function factorial(number: number) {
  if (number <= 0) {
    return 1;
  } else {
    return (number * factorial(number - 1));
  }
}
console.log(factorial(6));
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
function factorial(number) {
    if (number <= 0) {
        return 1;
    }
    else {
        return (number * factorial(number - 1));
    }
}
console.log(factorial(6));
```

&emsp;除了参数类型，二者一模一样。

### Lambda 函数 

&emsp;Lambda 函数也称之为箭头函数。箭头函数表达式的语法比函数表达式更短。函数只有一行语句：

```javascript
( [param1, parma2,…param n] )=>statement;
```

&emsp;以下实例声明了 lambda 表达式函数，函数返回两个数的和：

```typescript
var foo = (x: number) => 10 + x;
console.log(foo(100));
```

&emsp;编译后得到如下 JavaScript 代码：

```javascript
var foo = function (x) { return 10 + x; };
console.log(foo(100));
```

&emsp;函数是一个语句块：

```typescript
( [param1, parma2,…param n] )=> {
 
    // 代码块
}
```

&emsp;以下实例声明了 lambda 表达式函数，函数返回两个数的和：

```typescript
var foo = (x: number) => {
  x = 10 + x;
  console.log(x);
}
foo(100);
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
var foo = function (x) {
    x = 10 + x;
    console.log(x);
};
foo(100);
```

&emsp;我们可以不指定函数的参数类型，通过函数内来推断参数类型：

```typescript
var func = (x: string | number) => {
  if (typeof x == "number") {
    console.log(x + " 是一个数字");
  } else if (typeof x == "string") {
    console.log(x + " 是一个字符串");
  }
};

func(12);
func("tom");
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
var func = function (x) {
    if (typeof x == "number") {
        console.log(x + " 是一个数字");
    }
    else if (typeof x == "string") {
        console.log(x + " 是一个字符串");
    }
};
func(12);
func("tom");
```

&emsp;单个参数时 `()` 是可选的：

```typescript
var display = x => {
  console.log("输出为 " + x);
}
display(12);
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
var display = function (x) {
    console.log("输出为 " + x);
};
display(12);
```

&emsp;无参数时可以设置空括号：

```typescript
var disp = () => {
  console.log("Function invoked");
}
disp();
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
var disp = function () {
    console.log("Function invoked");
};
disp();
```

### 函数重载

&emsp;重载是方法名字相同，而参数不同，返回类型可以相同也可以不同。每个重载的方法（或者构造函数）都必须有一个独一无二的参数类型列表。

&emsp;参数类型不同：

```typescript
function disp(string): void;
function disp(number): void;
```

&emsp;参数数量不同：

```typescript
function disp(n1: number): void;
function disp(x: number, y: number): void;
```
&emsp;参数类型顺序不同：

```typescript
function disp(n1: number, s1: string): void;
function disp(s: string, n: number): void;
```

&emsp;如果参数类型不同，则参数类型应设置为 any。参数数量不同你可以将不同的参数设置为可选。

&emsp;以下实例定义了参数类型与参数数量不同：

```typescript
function disp(s1: string): void;
function disp(n1: number, s1: string): void;

function disp(x: any, y?: any): void {
  console.log(x);
  console.log(y);
}
disp("abc");
disp(1, "xyz");
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
function disp(x, y) {
    console.log(x);
    console.log(y);
}
disp("abc");
disp(1, "xyz");
```

&emsp;打印结果：

```javascript
abc
undefined
1
xyz
```

## TypeScript Number

&emsp;TypeScript 与 JavaScript 类似，支持 Number 对象。Number 对象是原始数值的包装对象。

```typescript
var num = new Number(value);
```

&emsp;如果一个参数值不能转换为一个数字将返回 NaN (非数字值)。

### Number 对象属性

&emsp;Number 对象支持下列属性：

1. MAX_VALUE 可表示的最大的数，MAX_VALUE 属性值接近于 1.79E+308。大于 MAX_VALUE 的值代表 "Infinity"。
2. MIN_VALUE 可表示的最小的数，即最接近 0 的正数 (实际上不会变成 0)。最大的负数是 -MIN_VALUE，MIN_VALUE 的值约为 5e-324。小于 MIN_VALUE ("underflow values") 的值将会转换为 0。
3. NaN 非数字值（Not-A-Number）。
4. NEGATIVE_INFINITY 负无穷大，溢出时返回该值。该值小于 MIN_VALUE。
5. POSITIVE_INFINITY 正无穷大，溢出时返回该值。该值大于 MAX_VALUE。
6. prototype Number 对象的静态属性。使你有能力向对象添加属性和方法。
7. constructor 返回对创建此对象的 Number 函数的引用。

```typescript
console.log("TypeScript Number 属性: "); 
console.log("最大值为: " + Number.MAX_VALUE); 
console.log("最小值为: " + Number.MIN_VALUE); 
console.log("负无穷大: " + Number.NEGATIVE_INFINITY); 
console.log("正无穷大:" + Number.POSITIVE_INFINITY);

// 输出如下：
TypeScript Number 属性: 
最大值为: 1.7976931348623157e+308
最小值为: 5e-324
负无穷大: -Infinity
正无穷大:Infinity
```

### NaN 实例

&emsp;示例代码如下：

```typescript
var month = 0;
if (month <= 0 || month > 12) {
  month = Number.NaN;
  console.log("月份是：" + month);
} else {
  console.log("输入月份数值正确。");
}

// 编译为 JavaScript 后一模一样，打印结果是：
月份是：NaN
```

### prototype 实例

&emsp;示例代码如下：

```typescript
function employee(id: number, name: string) {
  this.id = id;
  this.name = name;
}

var emp = new employee(123, "admin");
employee.prototype.email = "admin@apple.com";
console.log(emp.id);
console.log(emp.name);
console.log(emp.email);
```

&emsp;编译以上 TypeScript 代码，得到如下 JavaScript 代码：

```javascript
function employee(id, name) {
    this.id = id;
    this.name = name;
}
var emp = new employee(123, "admin");
employee.prototype.email = "admin@apple.com";
console.log(emp.id);
console.log(emp.name);
console.log(emp.email);
```

&emsp;输出如下：

```typescript
123
admin
admin@apple.com
```

### Number 对象方法

&emsp;Number 对象支持以下方法：

1. toExponential() 把对象的值转换为指数计数法。

```typescript
var num1 = 1225.30; 
var val = num1.toExponential(); 
console.log(val);

// 输出：
1.2253e+3
```

2. toFixed() 把数字转换为字符串，并对小数点指定位数。

```typescript
var num3 = 177.234;
console.log("num3.toFixed()：" + num3.toFixed());
console.log("num3.toFixed(2)：" + num3.toFixed(2));
console.log("num3.toFixed(6)：" + num3.toFixed(6));

// 输出：
num3.toFixed()：177
num3.toFixed(2)：177.23
num3.toFixed(6)：177.234000
```

3. toLocaleString() 把数字转换为字符串，使用本地数字格式顺序。

```typescript
var num = new Number(177.1234); 
console.log(num.toLocaleString());

// 输出：177.1234
```

4. toPrecision() 把数字格式化为指定的长度。

```typescript
var num = new Number(7.123456); 
console.log(num.toPrecision());  // 输出：7.123456 
console.log(num.toPrecision(1)); // 输出：7
console.log(num.toPrecision(2)); // 输出：7.1
```

5. toString() 把数字转换为字符串，使用指定的基数。数字的基数是 2 ~ 36 之间的整数。若省略该参数，则使用基数 10。

```typescript
var num = new Number(10); 
console.log(num.toString());  // 输出 10 进制：10
console.log(num.toString(2)); // 输出 2 进制：1010
console.log(num.toString(8)); // 输出 8 进制：12
```

6. valueOf() 返回一个 Number 对象的原始数字值。

```typescript
var num = new Number(10); 
console.log(num.valueOf());

// 输出：
10
``` 

### toLocaleString() 与 toString() 的区别

1. toLocaleString()，当数字是四位数及以上时，从右往左数，每三位用分号隔开，并且小数点后只保留三位；而 toString() 单纯将数字转换为字符串。
2. toLocaleString()，当目标是标准时间格式时，输出简洁年月日，时分秒；而 toString() 输出国际表述字符串。

```typescript
var num = new Number(1777.123488); 
console.log(num.toLocaleString()); // 输出：1,777.123
console.log(num.toString()); // 输出：1777.123488

var dateStr = new Date();
console.log(dateStr.toLocaleString()); // 输出：2022/2/15 16:48:35
console.log(dateStr.toString()); // 输出：Tue Feb 15 2022 16:48:58 GMT+0800 (中国标准时间)
```

## TypeScript String（字符串）

&emsp;String 对象用于处理文本（字符串）。

```typescript
var txt = new String("string");
var txt = "string";
```

### String 对象属性

&emsp;String 对象支持的一些属性如下：

1. `constructor` 对创建该对象的函数的引用。

```typescript
var str = new String("This is string");
console.log("str.constructor:" + str.constructor);

// 输出：
str.constructor:function String() { [native code] }
```

2. `length` 返回字符串的长度。

```typescript
var uname = new String("Hello World!");
console.log("Length " + uname.length);

// 输出
Length 12
```

3. `prototype` 允许向对象添加属性和方法。

```typescript
function employee(id: number, name: String) {
  this.id = id;
  this.name = name;
}

var emp = new employee(123, "admin");
employee.prototype.email = "admin@apple.com";
console.log(emp.id);
console.log(emp.name);
console.log(emp.email);

// 输出
123
admin
admin@apple.com
```

### String 方法

&emsp;String 对象支持的一些方法如下：

1. `charAt()` 返回在指定位置的字符。

```typescript
var str = new String("Apple");
console.log("str.charAt(0) 为：" + str.charAt(0));
console.log("str.charAt(1) 为：" + str.charAt(1));
console.log("str.charAt(2) 为：" + str.charAt(2));
console.log("str.charAt(3) 为：" + str.charAt(3));
console.log("str.charAt(4) 为：" + str.charAt(4));
console.log("str.charAt(5) 为：" + str.charAt(5));

// 输出
str.charAt(0) 为：A
str.charAt(1) 为：p
str.charAt(2) 为：p
str.charAt(3) 为：l
str.charAt(4) 为：e
str.charAt(5) 为：
```

2. `charCodeAt()` 返回在指定的位置的字符的 Unicode 编码。

```typescript
var str = new String("Apple");
console.log("str.charAt(0) 为：" + str.charCodeAt(0));
console.log("str.charAt(1) 为：" + str.charCodeAt(1));
console.log("str.charAt(2) 为：" + str.charCodeAt(2));
console.log("str.charAt(3) 为：" + str.charCodeAt(3));
console.log("str.charAt(4) 为：" + str.charCodeAt(4));
console.log("str.charAt(5) 为：" + str.charCodeAt(5));

// 输出
str.charAt(0) 为：65
str.charAt(1) 为：112
str.charAt(2) 为：112
str.charAt(3) 为：108
str.charAt(4) 为：101
str.charAt(5) 为：NaN
```

3. `concat()` 连接两个或更多字符串，并返回新的字符串。

```typescript
var str1 = new String("Apple");
var str2 = " Park!"; // var str2 = new String("Park!"); str2 不能使用 String，否则报如下错误：

// error TS2345: Argument of type 'String' is not assignable to parameter of type 'string'.
//   'string' is a primitive, but 'String' is a wrapper object. Prefer using 'string' when possible.
// 3 var str3 = str1.concat(str2);
//                          ~~~~

var str3 = str1.concat(str2);
console.log("str1 + str2：" + str3);

// 输出
str1 + str2：Apple Park!
```

4. `indexOf()` 返回某个指定的字符串值在字符串中首次出现的位置。 

```typescript
var str1 = new String("Apple");
var index = str1.indexOf("l");
console.log("查找的字符串位置：" + index);

// 输出 
3
```

5. `lastIndexOf()` 从后向前搜索字符串，并从起始位置（0）开始计算返回字符串最后出现的位置。 

```typescript
var str1 = new String("This is string one and again string");
var index = str1.lastIndexOf("string");
console.log("lastIndexOf 查找到的最后字符串位置：" + index);
index = str1.lastIndexOf("one");
console.log("lastIndexOf 查找到的最后字符串位置：" + index);

// 输出 
lastIndexOf 查找到的最后字符串位置：29
lastIndexOf 查找到的最后字符串位置：15
```

6. `localeCompare()` 用本地特定的顺序来比较两个字符串。

```typescript
var str1 = new String("This is beautiful string");
var index = str1.localeCompare("This is beautiful string");
console.log("localeCompare first：" + index);

// 输出
localeCompare first：0
```

7. `match()` 查找找到一个或多个正则表达式的匹配。

```typescript
var str = "The rain in SPAIN stays mainly in the plain";
var n = str.match(/ain/g);
console.log(n);

// 输出
[ 'ain', 'ain', 'ain' ]
```

8. `replace()` 替换与正则表达式匹配的子串。

```typescript
var re = /(\w+)\s(\w+)/;
var str = "zara ali";
var newstr = str.replace(re, "$2, $1");
console.log(newstr);

// 输出
ali, zara
```

9. `search()` 检索与正则表达式相匹配的值。

```typescript
var re = /apples/gi;
var str = "Apples are round, and apples are juicy.";
if (str.search(re) == -1) {
    console.log("Does not contain Apples");
} else {
    console.log("Contains Apples");
} 

// 输出

Contains Apples
```

10. `slice()` 提取字符串的片断，并在新的字符串中返回被提取的部分。

```typescript
var str = "Apples are round, and apples are juicy.";
var sliceResult = str.slice(3, 5);
console.log(sliceResult);

// 输出
le
```

11. `split()` 把字符串分割为子字符串数组。

```typescript
var str = "Apples are round, and apples are juicy.";
var splitted = str.split(" ", 3);
console.log(splitted); // [ 'Apples', 'are', 'round,' ]

// 输出
[ 'Apples', 'are', 'round,' ]
```

12. `substr()` 从起始索引号提取字符串中指定数目的字符。

```typescript
var str = "Apples are round, and apples are juicy.";
var splitted = str.substr(3);
console.log(splitted); // [ 'Apples', 'are', 'round,' ]

// 输出
les are round, and apples are juicy.
```

13. `substring()` 提取字符串中两个指定的索引号之间的字符。

```typescript
var str = "Apples are round, and apples are juicy.";
console.log("(1,2): " + str.substring(1, 2)); // (1,2): p
console.log("(0,10): " + str.substring(0, 10)); // (0,10): Apples are
console.log("(5): " + str.substring(5)); // (5): s are round, and apples are juicy.
```

14. `toLocaleLowerCase()` 根据主机的语言环境把字符串转换为小写，只有几种语言（如土耳其语）具有地方特有的大小写映射。

```typescript
var str = "Apple Google"; 
console.log(str.toLocaleLowerCase( ));

// 输出
apple google
```

15. `toLocaleUpperCase()` 据主机的语言环境把字符串转换为大写，只有几种语言（如土耳其语）具有地方特有的大小写映射。

```typescript
var str = "Apple Google"; 
console.log(str.toLocaleUpperCase( ));

// 输出
APPLE GOOGLE
```

16. `toLowerCase()` 把字符串转换为小写。

```typescript
var str = "Runoob Google"; 
console.log(str.toLowerCase( ));

// 输出
APPLE GOOGLE
```

17. `toString()` 返回字符串。

```typescript
var str = "Apple"; 
console.log(str.toString( ));

// 输出
Apple
```

18. `toUpperCase()` 把字符串转换为大写。

```typescript
var str = "Apple Google";
console.log(str.toUpperCase());

// 输出
APPLE GOOGLE
```

19. `valueOf()` 返回指定字符串对象的原始值。

```typescript
var str = new String("Apple"); 
console.log(str.valueOf( ));

// 输出
Apple
```

## TypeScript Array(数组)

&emsp;数组对象是使用单独的变量名来存储一系列的值。

&emsp;TypeScript 声明数组的语法格式如下所示：

```typescript
var array_name[:datatype]; // 声明 
array_name = [val1,val2,valn..] // 初始化

// 示例
var sites: string[];
sites = ["Google", "Apple", "Taobao"];
```

&emsp;或者直接在声明时初始化：

```typescript
var sites: string[] = ["Google", "Runoob", "Taobao"];
```

&emsp;如果数组声明时未设置类型，则会被认为是 `any` 类型，在初始化时根据第一个元素的类型来推断数组的类型。

&emsp;创建一个 number 类型的数组：

```typescript
var numlist: number[] = [2, 4, 6, 8];
```

### Array 对象

&emsp;我们也可以使用 Array 对象创建数组。Array 对象的构造函数接受以下两种值：

+ 表示数组大小的数值。
+ 初始化的数组列表，元素使用逗号分隔值。

&emsp;指定数组初始化大小：

```typescript
var arr_names: number[] = new Array(4);

for (var i = 0; i < arr_names.length; i++) {
  arr_names[i] = i * 2;
  console.log(arr_names[i]);
}

// 输出
0
2
4
6
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
var arr_names = new Array(4);
for (var i = 0; i < arr_names.length; i++) {
    arr_names[i] = i * 2;
    console.log(arr_names[i]);
}
```

&emsp;直接初始化数组元素：

```javascript
var sites: string[] = new Array("Google", "Apple", "Taobao", "Facebook");
for (var i = 0; i < sites.length; i++) {
  console.log(sites[i]);
}

// 输出
Google
Apple
Taobao
Facebook
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
var sites = new Array("Google", "Apple", "Taobao", "Facebook");
for (var i = 0; i < sites.length; i++) {
    console.log(sites[i]);
}
```

### 数组解构

&emsp;也可以把数组元素赋值给变量，如下所示：

```typescript
var arr: number[] = [12, 13];
var [x, y] = arr; // 将数组的两个元素赋值给变量 x 和 y
console.log(x);
console.log(y);

// 输出
12
13
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
var arr = [12, 13];
var x = arr[0], y = arr[1]; // 将数组的两个元素赋值给变量 x 和 y
console.log(x);
console.log(y);
```

### 数组迭代

&emsp;可以使用 for 语句来循环输出数组的各个元素：

```typescript
var j: any;
var nums: number[] = [1001, 1002, 1003, 1004];

for (j in nums) {
  console.log(nums[j]);
}

// 输出
1001
1002
1003
1004
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
var j;
var nums = [1001, 1002, 1003, 1004];
for (j in nums) {
    console.log(nums[j]);
}
```

### 多维数组

&emsp;一个数组的元素可以是另外一个数组，这样就构成了多维数组（Multi-dimensional Array）。最简单的多维数组是二维数组，定义方式如下：

```typescript
var arr_name: datatype[][] = [
  [val1, val2, val3],
  [v1, v2, v3],
];
```

```typescript
var multi: number[][] = [
  [1, 2, 3],
  [23, 24, 25],
];
console.log(multi[0][0]);
console.log(multi[0][1]);
console.log(multi[0][2]);
console.log(multi[1][0]);
console.log(multi[1][1]);
console.log(multi[1][2]);

// 输出
1
2
3
23
24
25
```

&emsp;编译以上代码，得到以下 JavaScript 代码：

```javascript
var multi = [
    [1, 2, 3],
    [23, 24, 25],
];
console.log(multi[0][0]);
console.log(multi[0][1]);
console.log(multi[0][2]);
console.log(multi[1][0]);
console.log(multi[1][1]);
console.log(multi[1][2]);
```

### 数组在函数中的使用

&emsp;作为参数传递给函数：

```typescript
var sites: string[] = new Array("Google", "Apple", "Taobao", "Facebook");

function disp(arr_sites: string[]) {
  for (var i = 0; i < arr_sites.length; i++) {
    console.log(arr_sites[i]);
  }
}
disp(sites);

// 输出
Google
Apple
Taobao
Facebook
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
var sites = new Array("Google", "Apple", "Taobao", "Facebook");
function disp(arr_sites) {
    for (var i = 0; i < arr_sites.length; i++) {
        console.log(arr_sites[i]);
    }
}
disp(sites);
```

&emsp;作为函数的返回值：

```typescript
function disp(): string[] {
  return new Array("Google", "Apple", "Taobao", "Facebook");
}

var sites: string[] = disp();
for (var i in sites) {
  console.log(sites[i]);
}

// 输出
Google
Apple
Taobao
Facebook
```

&emsp;编译以上代码，得到如下 JavaScript 代码：

```javascript
function disp() {
    return new Array("Google", "Apple", "Taobao", "Facebook");
}
var sites = disp();
for (var i in sites) {
    console.log(sites[i]);
}
```

### 数组方法

&emsp;列出了一些常用的数组方法：

1. `concat()` 连接两个或更多的数组，并返回结果。

```typescript
var alpha = ["a", "b", "c"];
var numeric = ["1", "2", "3"];

var alphaNumeric = alpha.concat(numeric); // 注意这里 不能是 number[]，alpha 和 numeric 是同样类型的数组
console.log("alphaNumeric : " + alphaNumeric);

// 输出
alphaNumeric : a,b,c,1,2,3
```

2. `every()` 检测数值元素的每个元素是否都符合条件。

```typescript
function isBigEnough(element, index, array) {
  return element >= 10;
}

var passed = [12, 5, 8, 130, 44].every(isBigEnough);
console.log("Test Value : " + passed);

// 输出
Test Value : false
```

3. `filter()` 检测数值元素，并返回符合条件所有元素的数组。

```typescript
function isBigEnough(element, index, array) {
  return element >= 10;
}

var passed = [12, 5, 8, 130, 44].filter(isBigEnough);
console.log("Test Value : " + passed);

// 输出
Test Value : 12,130,44
```

4. `forEach()` 数组每个元素都执行一次回调函数。

```typescript
let num = [7, 8, 9];
num.forEach(function (value) {
  console.log(value);
});

// 输出
7
8
9

// 编译成 JavaScript 代码：
var num = [7, 8, 9];
num.forEach(function (value) {
    console.log(value);
});
```

5. `indexOf()` 搜索数组中的元素，并返回它所在的位置。如果搜索不到，返回值 -1，代表没有此项。

```typescript
var index = [12, 5, 8, 130, 44].indexOf(8);
console.log("index is : " + index);

// 输出
index is : 2
```

6. `join()` 把数组的所有元素放入一个字符串。

```typescript
var arr = new Array("Google", "Apple", "Taobao");
var str = arr.join();
console.log("str : " + str);
// 输出
str : Google,Apple,Taobao

var str = arr.join(", ");
console.log("str : " + str);
// 输出
str : Google, Apple, Taobao

var str = arr.join(" + ");
console.log("str : " + str);
// 输出
str : Google + Apple + Taobao
```

7. `lastIndexOf()` 返回一个指定的字符串值最后出现的位置，在一个字符串中的指定位置从后向前搜索。

```typescript
var index = [12, 5, 8, 130, 8, 44].lastIndexOf(8);
console.log("index is : " + index);

// 输出
index is : 4
```

8. `map()` 通过指定函数处理数组的每个元素，并返回处理后的数组。

```typescript
var numbers = [1, 4, 9];
var roots = numbers.map(Math.sqrt);
console.log("roots is : " + roots);

// 输出
roots is : 1,2,3
```

9. `pop()` 删除数组的最后一个元素并返回删除的元素。

```typescript
var numbers = [1, 4, 9];

var element = numbers.pop();
console.log("element is : " + element);

// 输出
element is : 9

var element = numbers.pop();
console.log("element is : " + element);

// 输出
element is : 4
```

10. `push()` 向数组的末尾添加一个或更多元素，并返回新的长度。

```typescript
var numbers = new Array(1, 4, 9);
var length = numbers.push(10);
console.log("new numbers is : " + numbers);

// 输出
new numbers is : 1,4,9,10

length = numbers.push(20);
console.log("new numbers is : " + numbers);

// 输出
new numbers is : 1,4,9,10,20
```

11. `reduce()` 将数组元素计算为一个值（从左到右）。

```typescript
var total = [0, 1, 2, 3].reduce(function (a, b) { return a + b; });
console.log("total is : " + total);

// 输出
total is : 6
```

12. `reduceRight()` 将数组元素计算为一个值（从右到左）。

```typescript
var total = [0, 1, 2, 3].reduceRight(function (a, b) { return a + b; });
console.log("total is : " + total);

// 输出
total is : 6
```

13. `reverse()` 反转数组的元素顺序。

```typescript
var arr = [0, 1, 2, 3].reverse();
console.log("Reversed array is : " + arr);

// 输出
Reversed array is : 3,2,1,0
```

14. `shift()` 删除并返回数组的第一个元素。

```typescript
var arr = [10, 1, 2, 3].shift();
console.log("Shifted value is : " + arr);

// 输出
Shifted value is : 10
```

15. `slice()` 选取数组的的一部分，并返回一个新数组。

```typescript
var arr = ["orange", "mango", "banana", "sugar", "tea"];
console.log("arr.slice( 1, 2) : " + arr.slice(1, 2));  // mango
console.log("arr.slice( 1, 3) : " + arr.slice(1, 3));  // mango,banana

// 输出
arr.slice( 1, 2) : mango
arr.slice( 1, 3) : mango,banana
```

16. `some()` 检测数组元素中是否有元素符合指定条件。

```typescript
function isBigEnough(element, index, array) {
    return (element >= 10);
}

var retval = [2, 5, 8, 1, 4].some(isBigEnough);
console.log("Returned value is : " + retval);

// 输出
Returned value is : false

var retval = [12, 5, 8, 1, 4].some(isBigEnough);
console.log("Returned value is : " + retval);

// 输出
Returned value is : true
```

17. `sort()` 对数组的元素进行排序。

```typescript
var arr = new Array("orange", "mango", "banana", "sugar");
var sorted = arr.sort();
console.log("Returned string is : " + sorted);

// 输出
Returned string is : banana,mango,orange,sugar
```

18. `splice()` 从数组中添加或删除元素。

```typescript
var arr = ["orange", "mango", "banana", "sugar", "tea"];
var removed = arr.splice(2, 0, "water");
console.log("After adding 1: " + arr);

// 输出
After adding 1: orange,mango,water,banana,sugar,tea

console.log("removed is: " + removed);

// 输出
removed is: 

removed = arr.splice(3, 1);
console.log("After removing 1: " + arr);

// 输出
After removing 1: orange,mango,water,sugar,tea

console.log("removed is: " + removed);

// 输出
removed is: banana
```

19. `toString()` 把数组转换为字符串，并返回结果。

```typescript
var arr = new Array("orange", "mango", "banana", "sugar");
var str = arr.toString();
console.log("Returned string is : " + str);

// 输出
Returned string is : orange,mango,banana,sugar
```

20. `unshift()` 向数组的开头添加一个或更多元素，并返回新的长度。

```typescript
var arr = new Array("orange", "mango", "banana", "sugar"); 
var length = arr.unshift("water"); 
console.log("Returned array is : " + arr );
console.log("Length of the array is : " + length );

// 输出
Returned array is : water,orange,mango,banana,sugar
Length of the array is : 5
```

## TypeScript Map 对象

&emsp;Map 对象保存键值对，并且能够记住键的原始插入顺序。任何值(对象或者原始值) 都可以作为一个键或一个值。Map 是 ES6 中引入的一种新的数据结构，可以参考 ES6 Map 与 Set。










## 参考链接
**参考链接:🔗**
+ [TypeScript 教程](https://www.runoob.com/typescript/ts-tutorial.html)
+ [一起来学习吧](https://segmentfault.com/u/wuweisen/articles)
