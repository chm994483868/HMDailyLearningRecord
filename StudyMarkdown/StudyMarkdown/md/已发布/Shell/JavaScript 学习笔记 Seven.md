# JavaScript 学习笔记

&emsp;JavaScript 是 web 开发人员必须学习的 3 门语言中的一门：

1. HTML 定义了网页的内容。
2. CSS 描述了网页的布局。
3. JavaScript 控制了网页的行为。

&emsp;JavaScript 是一种轻量级的编程语言。JavaScript 是可插入 HTML 页面的编程代码。JavaScript 插入 HTML 页面后，可由所有的现代浏览器执行。

&emsp;你会经常看到 document.getElementById("some id")。这个方法是 HTML DOM 中定义的。

&emsp;DOM(Document Object Model)（文档对象模型）是用于访问 HTML 元素的正式 W3C 标准。

&emsp;JavaScript 与 Java 是两种完全不同的语言，无论在概念上还是设计上。Java（由 Sun 发明）是更复杂的编程语言。ECMA-262 是 JavaScript 标准的官方名称。JavaScript 是由 Brendan Eich 发明。它于 1995 年出现在 Netscape 中（该浏览器已停止更新），并于 1997 年被 ECMA（一个标准协会）采纳。

&emsp;`document.write` 是直接写入到页面的内容流，如果在写之前没有调用 `document.open`，浏览器会自动调用 `open`。每次写完关闭之后重新调用该函数，会导致页面被重写。

&emsp;`innerHTML` 则是 DOM 页面元素的一个属性，代表该元素的 html 内容。你可以精确到某一个具体的元素来进行更改。如果想修改 document 的内容，则需要修改 `document.documentElement.innerElement`。

&emsp;`innerHTML` 很多情况下都优于 `document.write`，其原因在于其允许更精确的控制要刷新页面的那一个部分。

&emsp;DOM：你会经常看到 `document.getElementById("id")`，这个方法是 HTML DOM 中定义的。DOM (Document Object Model)（文档对象模型）是用于访问 HTML 元素的正式 W3C 标准。

&emsp;`window.alert(5+6)` 与 `window.alert("5+6")` 输出的值是不一样的。window.alert(5+6) 会输出 11，而window.alert("5+6") 会输出 5+6。这是因为当用引号时会认为引号中是字符串，从而直接将引号中的内容打印出来。

&emsp;JavaScript 是一个程序语言，语法规则定义了语言结构。JavaScript 是一个脚本语言。它是一个轻量级，但功能强大的编程语言。

&emsp;JavaScript 对大小写是敏感的。

&emsp;JavaScript 是弱类型编程语言。定义变量都使用 var 定义，与 Java 这种强类型语言有区别。在定义后可以通过 `typeOf()` 来获取 JavaScript 中变量的数据类型。

&emsp;有个特殊情况，`typeOf` 不能用来判断 Array 还是 Object。`var arr = []; typeOf(arr) === 'object' // true` 结果为 true。

&emsp;JavaScript 是脚本语言，浏览器会在读取代码时，逐行地执行脚本代码。而对于传统编程来说，会在执行前对所有代码进行编译。

## JavaScript 变量

&emsp;`let` 声明的变量只在其声明的块或子块中可用，这一点，与 `var` 相似。二者之间最主要的区别在于 `var` 声明的变量的作用域是整个封闭函数。

```c++
function myFunction() {
    var x = 1;
    if (true) {
        var x = 2; // 同样的变量
        console.log(x); // 2
    }
    console.log(x); // 2，这里即使出了 if 的范围，x 依然等于 2
}

function myFunction() {
    let x = 1;
    if (true) {
        let x = 2; // 不同的变量
        console.log(x); // 2
    }
    console.log(x); // 1，和上面相比，这里的 x 还是 if 上面声明的 x
}
```

&emsp;JavaScript 声明变量的时候，虽然用 `var` 关键字声明和不用关键字声明，很多时候运行并没有问题，但是这两种方式还是有区别的。可以正常运行的代码并不代表是合适的代码。

```c++
// num1 为全局变量，num2 为 window 的一个属性
var num1 = 1;
num2 = 2;

// delete num1; 无法删除
// delete num2; 删除
function model() {
var num1 = 1; // 本地变量
num2 = 2; // window 的属性

// 匿名函数
(function(){
    var num = 1; // 本地变量
    num1 = 2; // 继承作用域（闭包）
    num3 = 3; // window 的属性
}())
}
```

&emsp;`const` 关键字用来声明 JavaScript 中的常量（与变量相对，不可修改，但同样是用于存储信息的 "容器"。），常量的值不能通过重新赋值来改变，并且不能重新声明。

```c++
// 定义常量 a 并赋值为 0
const a = 0;

// 报错（不能重新赋值）
a = 1;

// 报错 (不能重新声明)
const a = 2;

// 输出 0
console.log("a is: " + a);
```

&emsp;**JavaScript 允许重复声明变量，后声明的覆盖之前的。（注意：这是和 OC、Swift、C、C++ 等语言相比的一个很明显的区别。）**

```c++
var a = 1;
var a = "x";
console.log(a); // 输出 x, 1 已经被覆盖了
```

&emsp;这里对 **覆盖** 还有一些解释。首先我们要把定义一个变量分为两个过程：**声明** 和 **赋值**。

&emsp;JavaScript 允许变量被重复声明，在声明变量时 JavaScript 会自行判断这个变量是否已经被声明了，如果已经被声明（即已经存在），那么重复声明（即除了变量的非首次声明）会被跳过，不再执行声明的操作。

&emsp;JavaScript 变量的值是可以被重复赋值的，最后的赋值是这个变量最后的结果。

&emsp;打印仅声明不赋值的变量会输出：undefined。

```c++
var a;
console.log(a); // 输出：undefined
```

```c++
var a;
console.log(a); // 输出：undefined
a = 2; // a 赋值 2

// var a 相当于什么都没做，a 在上面已经声明过了，直接打印 a 看到 a 的值是前面赋值的 2，并不是 undefined 
var a;
console.log(a); // 输出：2

// 这里则可以理解为赋值覆盖，把前面 a 赋值的 2 覆盖为 3
var a = 3;
console.log(a); // 输出 3
```

&emsp;JavaScript 除了同名变量会覆盖，同样的函数也会覆盖。

&emsp;JavaScript 允许重复定义函数，JavaScript 没有重载这个概念，它依据函数名来区分函数。后定义的同名函数覆盖之前的，与参数无关。

```c++
function test() {
    console.log("test");
}
test(); // 输出：test arg number: 0 arg1: undefined

function test(arg1) {
    console.log("test arg number: " + arguments.length + " arg1: " + arg1);
}
test(1, 2); // 输出：test arg number: 2 arg1: 1
```

&emsp;实参个数如果比形参少，那么剩下的默认赋值为 `undefined`，如果实参传的比形参数量多，那么是全部都会被传进去的，只不过没有对应的形参可以引用（但可以用 `arguments` 来获取剩下的参数）。

```c++
function test(arg1) {
    for(var i = 0; i < arguments.length; i++) {
        console.log(arguments[i]);
    }
}
test(1, 2);
// 输出：
// 1
// 2
```

&emsp;变量与函数重名的时候，变量生效。

&emsp;这涉及到变量和函数的预解析：

+ 变量声明会被顶置，函数声明也会被顶置且比变量更先声明。
+ 变量的声明和赋值语句一起写时，JS 引擎在解析时，会将其拆成声明和赋值两部分，声明置顶，赋值保留在原来位置。
+ 声明过的变量不会再重复声明。

```c++
var a = 100;

function a() {
    return "function";
}
console.log(a); // 输出 100
console.log(a()); // 报错：Uncaught TypeError: a is not a function
```

&emsp;JavaScript 中有两种函数，一种是普通函数，一种是函数对象。下面的这种就是 "函数对象"，它实际上是声明一个匿名函数，然后将该函数的 `init` 方法赋值给该变量。

```c++
var a = 100;
var a = function() {
    return "function";
}
console.log(a);
console.log(a());

// 分别输出：

ƒ () {
            return "function";
        }
        
function
```

&emsp;函数与内部变量重名。

&emsp;定义普通函数，即在 `window` 变量下，定义一个 `key`，它的名字为该函数名，值为该函数的地址。函数内部的 `this` 指向 `window` 对象。

```c++
function a() {
    console.log(this);
    this.a = 1;
    var a = 5;
    a = 10;
    var v = "value";
    return "function";
}
console.log(a); // 输出函数 a 的内容
console.log(a()); // 输出 window 和 "function" 字符串
console.log(a); // 输出 1 （此时执行了 a 函数以后，this.a = 1，已经把 a 覆盖为一个变量了，函数 a 已经不存在了）
console.log(v); // Uncaught ReferenceError: v is not defined
```

## JavaScript 数据类型

&emsp;**值类型（基本类型）：** 字符串（String）、数字（Number）、布尔（Boolean）、对空（Null）、未定义（Undefined）、Symbol。

&emsp;**引用数据类型：** 对象（Object）、数组（Array）、函数（Function）。

> 注：Symbol 是 ES6 引入了一种新的原始数据类型，表示独一无二的值。

&emsp;JavaScript 拥有动态类型，这意味着相同的变量可用作不同的类型。

```c++
var x; // x 为 undefined
console.log(x); // 输出 undefined
var x = 5; // 现在 x 为数字
console.log(x); // 输出 5
var x = "John"; // 现在 x 为字符串
console.log(x); // 输出 John
```

&emsp;`Undefined` 和 `Null`，`Undefined` 这个值表示变量不含有值，可以通过将变量的值设置为 `null` 来清空变量。

```c++
var person;
var car = "Volvo";

document.write(person + "<br>"); // undefined
document.write(car + "<br>"); // Volvo

var car = null;
document.write(car + "<br>"); // null
```

&emsp;**声明变量类型**

&emsp;当你声明新变量时，可以使用关键字 `new` 来声明其类型。

```c++
var carname = new String;
var x = new Number;
var y = new Boolean;
var cars = new Array;
var person = new Object;
```

> JavaScript 变量均为对象。当你声明一个变量时，就创建了一个新的对象。

## JavaScript 对象

&emsp;JavaScript 对象是拥有属性和方法的数据。

&emsp;JavaScript 对象是属性和方法的容器。

&emsp;JavaScript 对象是键值对的容器，"键" 必须为字符串，"值" 可以是 JavaScript 中包括 `null` 和 `undefined` 的任意数据类型。

## JavaScript 函数

&emsp;函数是由事件驱动的或者当它被调用时执行的可重复使用的代码块。

&emsp;JavaScript 变量的生命周期从它们被声明的时间开始。

&emsp;局部变量会在函数运行以后被删除。

&emsp;全局变量会在页面关闭后删除。

&emsp;向未声明的 JavaScript 变量分配值。

&emsp;如果把值赋值给尚未声明的变量，该变量将被自动作为 window 的一个属性。非严格模式下给未声明变量赋值创建的全局变量，是全局对象的可配置属性，可以删除。

```c++
var var1 = 1; // 不可配置全局属性
var2 = 2; // 没有使用 var 声明，可配置全局属性

console.log(this.var1); // 1
console.log(window.var1); // 1
console.log(window.var2); // 2

console.log(delete var1); // false（无法删除）
console.log(delete var2); // true（可以删除）
console.log(var2); // 已经删除，报错变量未定义（Uncaught ReferenceError: var2 is not defined）
```

## JavaScript 作用域

&emsp;作用域是可访问变量的合集。

&emsp;如果变量在函数内没有声明（没有使用 var 关键字），该变量为全局变量。如下示例代码，`carName`在 `myFunction` 函数内，但是为全局变量。

```c++
myFunction();
document.getElementById("demo").innerHTML = "我可以显示 " + carName;

function myFunction() {
    carName = "Volvo";
}
```

&emsp;JavaScript 变量生命周期在它声明时初始化。

&emsp;局部变量在函数执行完毕后销毁。

&emsp;全局变量在页面关闭后销毁。

&emsp;函数参数只在函数内起作用，是局部变量。

&emsp;在 HTML 中，全局变量是 window 对象；所有数据变量都属于 window 对象。

> 你的全局变量，或者函数，可以覆盖 window 对象的变量或者函数。局部变量，包括 window 对象可以覆盖全局变量和函数。

&emsp;作用域是指程序源代码中定义变量的区域。

&emsp;作用域规定了如何查找变量，也就是确定当前执行代码对变量的访问权限。

&emsp;JavaScript 采用词法作用域（lexical scoping），也就是静态作用域。

```c++
var value = 1;

function foo() {
    console.log(value);
}

function bar() {
    var value = 2;
    foo();
}

bar(); // 输出 1，并不是 2
```

&emsp;假设 JavaScript 采用静态作用域，让我们分析下执行过程：

&emsp;执行 `foo` 函数，先从 `foo` 函数内部查找是否有局部变量 `value`，如果没有，就根据书写位置，查找上面一层的代码，也就是 `value` 等于 1，所以结果会打印 1。

&emsp;假设 JavaScript 采用动态作用域，让我们分析下执行过程：

&emsp;执行 `foo` 函数，依然是从 `foo` 函数内部查找是否有局部变量 `value`。如果没有，就从调用函数的作用域，也就是 `bar` 函数内部查找 `value` 变量，所以结果会打印 2。

&emsp;因为 JavaScript 采用的是静态作用域，所以上面示例代码中打印的结果是 1。

## JavaScript 事件

## JavaScript 字符串

&emsp;JavaScript 字符串用于存储和处理文本。

&emsp;不要创建 String 对象。它会拖慢执行速度，并可能产生其他副作用。

```c++
var x = "John"; // x 是一个字符串
var y = new String("John"); // y 是一个对象
document.getElementById("demo").innerHTML = typeof x + " " + typeof y; // string object
```

```c++
var x = "John"; // x 是一个字符串
var y = new String("John"); // y 是一个对象
// document.getElementById("demo").innerHTML = typeof x + " " + typeof y;
document.getElementById("demo").innerHTML = x === y; // false，因为 x 是字符串，y 是对象
```

&emsp;`===` 为绝对相等，即数据类型与值都必须相等。

&emsp;JavaScript 中 `==` 与 `===` 区别。

1. 对于 `string`、`number` 等基础类型，`==` 和 `===` 是有区别的。

+ 不同类型间比较，`==` 之比较 "转化成同一类型后的值" 看 "值" 是否相等，`===` 如果类型不同，其结果就是不等。
+ 同类型比较，直接进行 "值" 比较，两者结果一样。

2. 对于 `Array`、`Object` 等高级类型，`==` 和 `===` 是没有区别的，进行 "指针地址" 比较。

3. 基础类型与高级类型，`==` 和 `===` 是有区别的。

+ 对于 `==`，将高级转化为基础类型，进行 "值" 比较。
+ 因为类型不同，`===` 结果为 false。

4. `!=` 为 `==` 的非运算，`!==` 为 `===` 的非运算。

&emsp;双引号 " " 中用单引号 ' ' 可以不用加反斜杠，例如：

```c++
var x = "my name 'is' xxx"; // 此处不需要加反斜杠
```

&emsp;双引号 " " 中用双引号 " " 需要加反斜杠，例如：

```c++
var x = "my name \"is\" xxx"; // 此处需要在两个上引号前各加一个加反斜杠
```

&emsp;单引号 ' ' 中用双引号 " " 不需要加反斜杠，当然加了也可以，例如：

```c++
var x1 = 'my name "is" xxx'; // 此处不需要加反斜杠（推荐）
var x2 = 'my name \"is\" xxx'; // 添加反斜杠效果也一样（不推荐）
```

## JavaScript 运算符

&emsp;如果把数字与字符串相加，结果将成为字符串。

```c++
var result1 = 5 + 5 + "abc";
console.log(result1); // 输出：10abc

var result2 = "" + 5 + 5 + "abc";
console.log(result2); // 输出：55abc
```

&emsp;空文本 + 数字得到的运算结果都是把数字转成字符串，无论文本有多少空格，但是空格会计入长度。

```c++
var result1 = "" + 5;
console.log(result1); // 输出："5"

var result2 = " " + 5;
console.log(result2); // 输出：" 5"

var result3 = "     " + 5;
console.log(result3); // 输出："     5"
```

&emsp;数字和布尔值相加，布尔值 false 转成 0，true 转成 1。

&emsp;字符串与布尔值相加，布尔值转化成字符串。

&emsp;取模运算的结果符号只与左边值的符号有关。

+ 如果 `%` 左边的操作数是正数，则模除的结果为正数或零。
+ 如果 `%` 左边的操作数是负数，则模除的结果为负数或零。

&emsp;数字与 `null` 相加，`null` 转化为数字 0，字符串与 `null` 相加，`null` 转化为字符串：

```c++
var car = null + 3 + 4;
console.log(car); // 输出：7

var car2 = null + 'a';
console.log(car2); // 输出：nulla
```

1. 字符串一个很能强大的数据类型；在执行加 `+` 时，将被加的对象统一处理为字符串。
2. `bool` 类型在与数字类型进行相加时，视为 0 或者 1 处理。
3. `null` 类型与数字类型进行累加时，视为 0 处理。
4. `bool` 类型与 `null` 类型进行累加时，视为其与整数类型累加处理。
5. `undefined` 除了与字符串进行累加时有效（`undefined` 视为字符串 "undefined" 处理），其他情况皆返回 `NaN`。

## JavaScript 比较 和 逻辑运算符

&emsp;`null`、`undefined`、`0`、`NaN`、空字符串转换为 `false`，其他转换为 `true`。

&emsp;**逻辑与 &&** 返回的不是单纯的 `true` 或 `false`，而是具体的值，若是第一个值转换布尔值为 `true`，就返回第二个值，反之，返回第一个值。

&emsp;**逻辑或 ||** 同 `&&` 类似，返回的不是单纯的 `true` 或 `false`，而是具体的值，它是从头开始，遇到能转换为 `true` 的值，就返回那个值，如果没有 `true` 的就返回最后一个值。

```c++
<script>
    var a = [1, 2, 3]
    var b = "hello";
    var obj = new Object();
    var d;

    console.log(true && 10); // 10
    console.log(false && b); // false
    console.log(100 && false); // false
    console.log(undefined && false); // undefined
    console.log(NaN && false); // NaN
    console.log(null && false); // null
    console.log('' && false); // ''
    console.log(0 && 100); // 0
    console.log(5 && 100); // 100
    console.log(a && b); // hello
    console.log(obj && 200); // 200

    console.log(true || 10); // true
    console.log(false || b); // b
    console.log(100 || false); // 100
    console.log(undefined || 9); // 9
    console.log(NaN || false); // false
    console.log(null || a); // a
    console.log('' || false); // false
    console.log(0 || 100); // 100
    console.log(5 || 100); // 5
    console.log(a || b); // a
    console.log(obj || 200); // obj
</script>
```

## JavaScript 条件语句

## JavaScript switch 语句

## JavaScript for 循环

&emsp;`while` 遍历数组的例子中，如果数组中有 0、null、false、undefined 或者空字符串等在 js 中被认为等价于 `false` 的值，会提前结束遍历，改成判断数组长度即可避免该问题。

```c++
while (cars[i]) {
    document.write(cars[i] + "<br>");
    i++;
}

// 修改为 

while (i < cars.length) {
    document.write(cars[i] + "<br>");
    i++;
}
```

## JavaScript break 和 continue 语句

&emsp;`break` 语句用于跳出循环。

&emsp;`continue` 用于跳出循环中的一个迭代。

## JavaScript 标签

&emsp;通过标签引用，`break` 语句可用于跳出任何 JavaScript 代码块：

```c++
cars = ["BMW", "Volvo", "Saab", "Ford"]
list: {
    document.write(cars[0] + "<br>");
    document.write(cars[1] + "<br>");
    document.write(cars[2] + "<br>");
    break list;
    document.write(cars[3] + "<br>");
    document.write(cars[4] + "<br>");
    document.write(cars[5] + "<br>");
}

// 仅执行到：Saab，后面的 Ford、undefined、undefined 不会写入
```

&emsp;关于 JavaScript 标签与 `break`、`continue` 一起使用的细节：

&emsp;`break` 的作用是 **跳出代码块**，所以 `break` 可以使用于循环和 `switch` 等。`continue` 的作用是进入下一个迭代，所以 `continue` 只能用于循环的代码块，代码块基本是指 `{}` 大括号之间的内容。 

&emsp;默认标签的情况，当 `break` 和 `continue` 同时用于循环时，没有加标签，此时默认标签为当前循环的代码块。当 `break` 用于 `switch` 时，默认标签为当前的 `switch` 代码块。

## JavaScript typeof、null、undefined

&emsp;使用 `typeof` 操作符来检测变量的数据类型，注意这里的 `typeof` 是一个操作符，直接在变量前面使用，并不是一个函数。

```c++
typeof "john" // string
typeof 3.14 // number
typeof false // boolean
typeof [1, 2, 3, 4] // object
typeof {name: 'john', age: 34} // object
```

&emsp;在 JavaScript 中，数组是一种特殊的对象类型，因此 `typeof [1, 2, 3, 4]` 返回的是 `object`。

### null

&emsp;在 JavaScript 中 `null` 表示：什么都没有。`null` 是一个只有一个值的特殊类型，表示一个空对象引用。用 `typeof` 检测 `null` 返回的是 `object`。

&emsp;可以把变量设置为 `null` 来清空对象。

```c++
var person = null; // 值为 null(空)，但类型是对象 
```

&emsp;可以把变量设置为 `undefined` 来清空对象。

```c++
var person = undefined; // 值为 undefined，类型为 undefined
```

### undefined

&emsp;在 JavaScript 中，`undefined` 是一个没有设置值的变量。`typeof` 一个没有值的变量会返回 `undefined`。

&emsp;`null` 和 `undefined` 的值相等，但类型不同。

```c++
typeof undefined // undefined
typeof null // object
(null === undefined) // false
(null == undefined) // true
```

&emsp;`undefined` 是所有没有赋值变量的默认值，自动赋值。

&emsp;`null` 主动释放一个变量引用的对象，表示一个变量不再指向任何对象地址。

## JavaScript 类型转换

&emsp;`Number()` 转换为数字，`String()` 转换为字符串，`Boolean()` 转换为布尔值。

### JavaScript 数据类型

&emsp;在 JavaScript 中有 6 种不同的数据类型：

+ `string`
+ `number`
+ `boolean`
+ `object`
+ `function`
+ `symbol`

&emsp;3 种对象类型：

+ `Object`
+ `Date`
+ `Array`

&emsp;2 个不包含任何值的数据类型：

+ `null`
+ `undefined`

&emsp;请注意：

+ `NaN` 的数据类型是 `number`
+ 数组(Array)的数据类型是 `object`
+ 日期(Date)的数据类型为 `object`
+ `null` 的数据类型是 `object`
+ 未定义变量的数据类型为 `undefined`

&emsp;如果对象是 JavaScript `Array` 或 JavaScript `Date`，我们就无法通过 `typeof` 来判断他们的类型，因为都是返回 `object`。

&emsp;`constructor` 属性返回所有 JavaScript 变量的构造函数。

```c++
<p>constructor 属性返回变量或对象的构造函数。</p>
<p id="demo"></p>
<script>
    document.getElementById("demo").innerHTML =
        "john".constructor + "<br>" +
        (3.14).constructor + "<br>" +
        false.constructor + "<br>" + [1, 2, 3, 4].constructor + "<br>" + {
            name: "john",
            age: 34
        }.constructor + "<br>" +
        new Date().constructor + "<br>" +
        function() {}.constructor;
</script>

// 输出：

function String() { [native code] }
function Number() { [native code] }
function Boolean() { [native code] }
function Array() { [native code] }
function Object() { [native code] }
function Date() { [native code] }
function Function() { [native code] }
``` 

&emsp;可以使用 `constructor` 属性来查看对象是否为数组（包含字符串 "Array"）:

```c++
function isArray(myArray) {
    return myArray.constructor.toString().indexOf("Array") > -1;
}
```

&emsp;可以使用 `constructor` 属性来查看对象是否为日期（包含字符串 "Date"）:

```c++
function isDate(myDate) {
    return myDate.constructor.toString().indexOf("Date") > -1;
}
```

&emsp;将字符串转换为数字：

&emsp;全局方法 `Number()` 可以将字符串转换为数字。字符串包含数字（如："3.14"）转换为数字（如：3.14）。

&emsp;空字符串转换为 0。

&emsp;其他的字符串会转换为 `NaN`（不是个数字）。

&emsp;Operator + 可用于将变量转换为数字：

```c++
var y = "5"; // y 是一个字符串
var x = + y; // x 是一个数字
```

&emsp;如果变量不能转换，它仍然会是一个数字，但值为 `NaN`（不是一个数字）；

&emsp;`instanceof` 可通过 `instanceof` 操作符来判断对象的具体类型，语法格式：

```c++
var result = objectName instanceof objectType
```

&emsp;返回布尔值，如果是指定类型则返回 `true`，否则返回 `false`。

```c++
arr = [1,2,3];

if (arr instanceof Array) {
    document.write("arr 是一个数组");
} else {
    document.write("arr 不是一个数组");
}
```

&emsp;`NaN` 是一个特殊的数值，`NaN` 即非数值（Not a Number），这个数值用于本来要返回数值的操作数未返回数值的情况。`NaN` 与任何值都不相等，包括 `NaN` 本身。

&emsp;可以通过 `isNaN()` 方法来判断某个数值是否是 `NaN` 这个特殊的数，使用 `isNaN()` 方法会将传入的数值如果是非数值的会将其自动转换成数值类型，若能转换成数值类型，那么这个函数返回 `false`，若不能转换成数值类型，则这个数就是 `NaN`，即返回 `true`。

&emsp;通常来讲，使用 `instanceof` 就是判断一个实例是否属于某种类型，更重要的是 `instanceof` 可以在继承关系中用来判断一个实例是否属于它的父类型。用 `instanceof` 来判断类型只能用于 **对象层面**，不是一个对象则不行，如下：

```c++
str = "asd";
if (str instanceof String) {
    console.log("str YES"); // 不打印 str YES
}

str2 = new String();
if (str2 instanceof String) {
    console.log("str2 YES"); // 打印 str2 YES
}
```

## JavaScript 正则表达式

## JavaScript 错误 - throw、try 和 catch

&emsp;`throw` 语句创建自定义错误。

&emsp;JavaScript 抛出（throw）错误：当错误发生时，当事情出问题时，JavaScript 引擎通常会停止，并生成一个错误消息。

## JavaScript 声明提升

&emsp;理解 "hoisting(声明提升)"。声明提升，函数声明和变量声明总是会被解释器悄悄的 "提升" 到方法体的最顶部。

&emsp;JavaScript 只有声明的变量会提升，初始化的不会。

&emsp;JavaScript 严格模式（strict mode）不允许使用未声明的变量。

&emsp;首先 JavaScript 在执行之前会有一个预编译过程，变量提升和函数提升就是发生在这里。在执行 JavaScript 时首先会创建一个 `AO` 对象（Activetion Object 执行期上下文）。然后会将形参和变量声明作为 `AO` 对象的属性名，属性的值为 `undefined`。 

&emsp;变量的声明和函数的声明提升，提升的时机发生在预解析过程中。预解析过程也就是创建 AO（Activation Object） 的过程。创建AO过程：

+ 创建 `AO` 对象。
+ 将形参和函数内变量声明作为 `AO` 对象的属性名，属性值统一为 `undefined`。
+ 将实参赋值给形参。
+ 找函数内的函数声明作为 `AO` 对象的属性名，属性值为函数体。

&emsp;什么叫变量提升？

&emsp;原则上变量应该先声明后使用，但是开发者可能常常忘记声明就使用了变量，这样做 JavaScript 代码在执行的时候不报错，只是返回了一个 `undefined`，这种情况就是变量提升。计算机执行的时候会把未声明就使用的变量隐式的放到代码的最顶端。需要注意的是变量虽然发生了提升，但是给变量赋的值是不会随之提升的，所以就会得到结果 `undefined`。

&emsp;什么是函数提升？

&emsp;与变量提升的意思差不多，先使用函数，后再声明函数，这种违背逻辑的事情在 JavaScript 中是允许的，这门语言就是这么灵活。与变量提升不同的是，函数的返回值也会随之提升，所以你会发现在 `<script>` 标签中的任何地方都能调用函数并且使用函数的返回值。

## JavaScript 严格模式（use strict）

&emsp;"use strict" 指令在 JavaScript 1.8.5（ECMAScript5）中新增。它不是一条语句，但是是一个字面量表达式，在 JavaScript 旧版本中会被忽略。"use strict" 的目的是指定代码在严格条件下执行。严格模式下不能使用未声明的变量。

&emsp;严格模式通过在脚本或函数的头部添加 `use strict;` 表达式来声明。

&emsp;为什么使用严格模式:

&emsp;消除 Javascript 语法的一些不合理、不严谨之处，减少一些怪异行为;

+ 消除代码运行的一些不安全之处，保证代码运行的安全。
+ 提高编译器效率，增加运行速度。
+ 为未来新版本的 Javascript 做好铺垫。

&emsp;"严格模式" 体现了 Javascript 更合理、更安全、更严谨的发展方向，包括 IE 10 在内的主流浏览器，都已经支持它，许多大项目已经开始全面拥抱它。另一方面，同样的代码，在 "严格模式" 中，可能会有不一样的运行结果；一些在 "正常模式" 下可以运行的语句，在 "严格模式" 下将不能运行。掌握这些内容，有助于更细致深入地理解 JavaScript。

&emsp;`use strict;` 指令只允许出现在脚本或函数的开头。

## JavaScript 使用误区

&emsp;赋值语句返回变量的值。

### 程序块作用域

&emsp;在每个代码块中 JavaScript 不会创建一个新的作用域，一般各个代码块的作用域都是全局的。

&emsp;以下代码的变量 i 打印 10，而不是 `undefined`:

```c++
for (var i = 0; i < 10; i++) {
    // some code
}
console.log(i);
```

&emsp;针对上面的代码，如何取 i 呢？

&emsp;在 ES6 中提出了 `let` 的概念，使用 `let` 声明的变量将具有作用域的限制，如：

```c++
for (let i = 0; i < 10; i++) {
    // some code
}
console.log(i);
```

&emsp;此时便打印：`Uncaught ReferenceError: i is not defined`

&emsp;需要注意的是这是可以向下兼容的，比如：

```c++
var i = 1; 
{
    let i = 0;
    {
        alert(i); // 能获取 i 变量值是 0
    }
}
console.log(i);
```

&emsp;此时最后获得的 i 变量是最前面声明的变量 i，打印 1。而二级作用域的 alert 可以获得上一级的值为 0 的变量 i，这就是所谓的 **上级作用域向下兼容**。

## JavaScript 表单

&emsp;HTML 表单验证可以通过 JavaScript 来完成。

## JavaScript 表单验证

## JavaScript 验证 API

## JavaScript 保留关键字

## JavaScript this 关键字

&emsp;面向对象语言中 `this` 表示当前对象的一个引用，但在 JavaScript 中 `this` 不是固定不变的，它会随着执行环境的改变而改变。

+ 在对象方法中，`this` 指向调用它所在方法的对象。

+ 单独使用 `this`，它指向全局（Global）对象。

```c++
var x = this; // [object Window]
```

+ 函数使用中，`this` 指向函数的所属者。
+ 严格模式下函数是没有绑定到 `this` 上，这时候 `this` 是 `undefined`。

```c++
"use strict";
function myFunction() {
  return this; // undefined
}
```

+ 在 HTML 事件句柄中，`this` 指向了接收事件的 HTML 元素。
+ `apply()` 和 `call()` 允许切换函数执行的上下文环境（context），即 `this` 绑定的对象，可以将 `this` 引用到任何对象。

&emsp;单独使用 `this`，则它指向全局（Global）对象。在浏览器中，`window` 就是该全局对象为 `[object Window]`。

&emsp;严格模式下，如果单独使用 `this` 也是指向全局（Global）对象。

```c++
"use strict";
var x = this; // [object Window]
```

&emsp;函数中使用 `this`（默认），在函数中，函数的所属者默认绑定到 this 上。在浏览器中，`window` 就是该全局对象为 `[object Window]`。

&emsp;严格模式下，函数是没有绑定到 `this` 上，这时候 `this` 是 `undefined`。

&emsp;在 HTML 事件句柄中，`this` 指向了接收事件的 HTML 元素：

```c++
<button onclick="this.style.display='none'">点我后我就消失了</button>
```

&emsp;显示函数绑定：

&emsp;在 JavaScript 中函数也是对象，对象则有方法，`apply` 和 `call` 就是函数对象的方法， 这两个方法异常强大，它们允许切换函数执行的上下文环境（context），即 `this` 绑定的对象。

&emsp;在下面的实例中，当我们使用 `person2` 作为参数来调用 `person1.fullName` 方法时，`this` 将指向 `person2`，即便它是 `person1` 的方法：

```c++
var person1 = {
    fullName: function() {
        return this.firstName + " " + this.lastName;
    }
}
var person2 = {
    firstName: "John",
    lastName: "Doe",
}
var x = person1.fullName.call(person2); // x 的值："John Doe"
```

&emsp;`this` 指向的是该 `this` 所在的最里层的 object 对象。

+ 函数不是 object 对象，所以没有写在 object 对象里的函数调用 `this` 会指向 `window`。
+ 构造函数是 object 对象，所以在构造函数中调用 `this` 会指向该构造函数。
+ HTML 元素是 object 元素，所以在 HTML 元素中调用 `this` 会指向该元素。

&emsp;函数1 `return 函数2`，函数2 `return this`，该 `this` 会指向 `window`。

```c++
let obj = {
    fun1: function() {
        return function() {
            return this;
        }
    },
};

console.log(obj.fun1()()); // window
```

&emsp;`this` 含义：解析器在调用函数时，每次都会向函数内部传递进隐藏的参数。根据函数的调用方式的不同，`this` 会指向不同的对象。

+ 以函数的形式调用时，`this` 永远都是 `window`。
+ 以方法的形式调用时，`this` 就是调用方法的那个对象。

## JavaScript let 和 const

&emsp;HTML 代码中使用全局变量：

+ 在 JavaScript 中，全局作用域是针对 JavaScript 环境。
+ 在 HTML 中，全局作用域是针对 window 对象。

&emsp;使用 `var` 关键字声明的全局作用域变量属于 `window` 对象。

```c++
var carName = "Volvo"; // 可以使用 window.carName 访问变量
``` 

&emsp;使用 `let` 关键字声明的全局作用域变量不属于 `window` 对象。

```c++
let carName = "Volvo"; // 不能使用 window.carName 访问变量（undefined）
```

&emsp;在相同的作用域或块级作用域中，不能使用 `let` 关键字来重置 `var` 关键字声明的变量：

```c++
var x = 2;
let x = 3; // Uncaught SyntaxError: Identifier 'x' has already been declared
```

```c++
let x = 2;
let x = 3; // Uncaught SyntaxError: Identifier 'x' has already been declared
```

```c++
let x = 2;
var x = 3; // Uncaught SyntaxError: Identifier 'x' has already been declared
```

&emsp;`let` 关键字定义的变量则不可以在使用后声明，也就是变量需要先声明再使用。

&emsp;`const` 用于声明一个或多个常量，声明时必须进行初始化，且初始化后值不可再修改。

&emsp;`const` 的本质: `const` 定义的变量并非常量，并非不可变，它定义了一个常量引用一个值。使用 `const` 定义的对象或者数组，其实是可变的。下面的代码并不会报错：

```c++
// 创建常量对象
const car = {type:"Fiat", model:"500", color:"white"};
 
// 修改属性:
car.color = "red";
 
// 添加属性
car.owner = "Johnson";
```

&emsp;但是我们不能对常量对象重新赋值。

&emsp;`const` 定义的变量并非不可改变，比如使用 `const` 声明对象，可以改变对象值。那么什么情况能彻底 "锁死" 变量呢？可以使用 `Object.freeze()` 方法来 **冻结变量**，如：

```c++
const obj = {
  name:"1024kb"
};

Object.freeze(obj) // 此时对象 obj 被冻结，返回被冻结的对象
```

&emsp;需要注意的是，被冻结后的对象不仅仅是不能修改值，同时也：

+ 不能向这个对象添加新的属性
+ 不能修改其已有属性的值
+ 不能删除已有属性
+ 不能修改该对象已有属性的可枚举性、可配置性、可写性

&emsp;建议判断清除情况再进行使用。

## JavaScript JSON

&emsp;JSON 是用于存储和传输数据的格式。JSON 通常用于服务端向网页传递数据。

&emsp;什么是 JSON? JSON 英文全称 JavaScript Object Notation。

&emsp;JSON 是一种轻量级的数据交换格式。

&emsp;JSON 是独立的语言，JSON 易于理解。

> &emsp;JSON 使用 JavaScript 语法，但是 JSON 格式仅仅是一个文本，文本可以被任何编程语言读取及作为数据格式传递。

&emsp;JSON 格式在语法上与创建 JavaScript 对象代码是相同的。由于它们很相似，所以 JavaScript 程序可以很容易的将 JSON 数据转换为 JavaScript 对象。

## JavaScript void

&emsp;`javascript:void(0)` 中最关键的是 `void` 关键字，`void` 是 JavaScript 中非常重要的关键字，该操作符指定要计算一个表达式但是不返回值。

&emsp;

```c++
<a href="javascript:void(alert('Warning!!!'))">点我弹出警告框</a>
<a href="javascript:void(0)">点我什么也不会发生</a>
```

```c++
function getValue() {
    var a, b, c;
    a = void( b = 5, c = 7);
    document.write('a = ' + a + ' b = ' + b + ' c = ' + c);
}

// a = undefined b = 5 c = 7
```

&emsp;href="#" 与 href="javascript:void(0)" 的区别：

&emsp;# 包含了一个位置信息，默认的锚是 `#top` 也就是网页的上端，而 `javascript:void(0)`，仅仅表示一个死链接。

&emsp;在页面很长的时候会使用 # 来定位页面的具体位置，格式为: # + id。

```c++
<a href="javascript:void(0);">点我没有反应的!</a>
<a href="#pos">点我定位到指定位置!</a>
<br>
...
// 可以在这里插入很多 <br> 让页面高度超出屏幕，方便跳转测试
<br>
<p id="pos">尾部定位点</p>
```

&emsp;`void()` 仅仅是代表不返回任何值，但是括号内的表达式还是要运行，如：

```c++
void(alert("Warnning!"))
```

```c++
    <!-- // 阻止链接跳转，URL 不会有任何变化 -->
    <a href="javascript:void(0)" rel="nofollow ugc">点击此处</a>

    <!-- 虽然阻止了链接跳转，但 URL 尾部会多个 #，改变了当前 URL。（# 主要用于配合 location.hash） -->
    <a href="#" rel="nofollow ugc">点击此处</a>

    <!-- 同理，# 可以的话，？也能达到阻止页面跳转的效果，但也相同的改变了 URL。（？主要用于配合 location.search） -->
    <a href="?" rel="nofollow ugc">点击此处</a>

    <!-- Chrome 中即使 javascript:0; 也没变化，firefox 中会变成一个字符串 0 -->
    <a href="javascript:0" rel="nofollow ugc">点击此处</a>
```

## JavaScript 异步编程

&emsp;异步（Asynchronous，async）是与同步（Synchronous，sync）相对的概念。

&emsp;JavaScript 中的异步操作函数往往通过回调函数来实现异步任务的结果处理。

## JavaScript Promise

&emsp;Promise 是一个 ECMAScript 6 提供的类，目的是更加优雅地书写复杂的异步任务。

## JavaScript 代码规范

&emsp;代码规范通常包括以下几个方面：

+ 变量和函数的命名规则
+ 空格、缩进、注释的使用规则
+ 其他常用规范...

&emsp;通常使用 4 个空格符号来缩进代码块。

>  &emsp;不推荐使用 TAB 键来缩进，因为不同编辑器 TAB 键的解析一样。

&emsp;每行代码字符小于 80。为了便于阅读每行字符建议小于 80 个。如果一个 JavaScript 语句超过了 80 个字符，建议在运算符或者逗号后换行。

## JavaScript 函数定义

&emsp;JavaScript 使用关键字 function 定义函数。函数可以通过声明定义，也可以是一个表达式。

&emsp;`Function()` 构造函数：函数除了通过关键字 function 定义，函数同样可以通过内置的 JavaScript 函数构造器（Function()）定义。

```c++
var myFunction = new Function("a", "b", "return a * b");
document.getElementById("demo").innerHTML = myFunction(4, 3);
```

> &emsp;在 JavaScript 中，很多时候，你需要避免使用 `new` 关键字。

### 函数提升（Hoisting）

&emsp;提升（Hoisting）是 JavaScript 默认将当前作用域提升到前面去的行为。提升（Hoisting）应用在变量的声明与函数的声明。因此，函数可以在声明之前调用：

```c++
myFunction(5);

function myFunction(y) {
    return y * y;
}
```

&emsp;使用表达式定义函数时无法提升。

&emsp;函数是对象。在 JavaScript 中使用 `typeof` 操作符判断函数类型将返回 `function`，但是 JavaScript 函数描述为一个对象更加准确。JavaScript 函数有属性和方法。

&emsp;`arguments.length` 属性返回函数调用过程接收到的参数个数。

```c++
function myFunction(a, b) {
    return arguments.length;
}
```

&emsp;`toString()` 方法将函数作为一个字符串返回。

```c++
function myFunction(a, b) {
    return a * b;
}
document.getElementById("demo").innerHTML = myFunction.toString();
// function myFunction(a, b) { return a * b; }
```

> &emsp;函数定义作为对象的属性，称之为对象方法。函数如果用于创建新的对象，称之为对象的构造函数。

### 箭头函数

&emsp;ES6 新增了箭头函数。箭头函数表达式的语法比普通函数表达式更简洁。箭头函数不需要使用 function、return 关键字及大括号 {}。

```c++
// ES5
var x = function(x, y) {
     return x * y;
}
 
// ES6
const x = (x, y) => x * y;
```

&emsp;有的箭头函数都没有自己的 `this`，不适合定义一个对象的方法。

&emsp;当我们使用箭头函数的时候，箭头函数会默认帮我们绑定外层 `this` 的值，所以在箭头函数中 `this` 的值和外层的 `this` 是一样的。箭头函数是不能提升的，所以需要在使用之前定义。

&emsp;使用 `const` 比使用 `var` 更安全，因为函数表达式始终是一个常量。

&emsp;如果函数部分只是一个语句，则可以省略 `return` 关键字和大括号 `{}`，这样做是一个比较好的习惯。

## JavaScript 函数参数

&emsp;JavaScript 函数对参数的值没有进行任何的检查。

&emsp;
