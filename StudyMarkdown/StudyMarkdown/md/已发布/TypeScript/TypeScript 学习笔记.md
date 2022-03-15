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
























## 参考链接
**参考链接:🔗**
+ [TypeScript 教程](https://www.runoob.com/typescript/ts-tutorial.html)
+ [一起来学习吧](https://segmentfault.com/u/wuweisen/articles)
