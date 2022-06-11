# JavaScript 学习记录（九）：Meta programming

&emsp;从 ECMAScript 2015 开始，JavaScript 获得了 Proxy 和 Reflect 对象的支持，允许你拦截并定义基本语言操作的自定义行为（例如：属性查找、赋值、枚举、函数调用等）。借助这两个对象，你可以在 JavaScript 元级别进行编程。

## 代理（Proxies）

&emsp;在 ECMAScript 6 中引入的 Proxy 对象可以拦截某些操作并实现自定义行为。例如获取一个对象上的属性：

```javascript
let handler = {
  get: function(target, name){
    return name in target ? target[name] : 42; // 如果 name 是 target 的属性则直接返回 target 的 name 属性，如果不是的话则返回 42
}};

let p = new Proxy({}, handler);
p.a = 1;

console.log(p.a, p.b); // 1, 42
```

&emsp;Proxy 对象定义了一个目标（这里是一个空对象）和一个实现了 get 陷阱的 handler 对象。这里，代理的对象在获取未定义的属性时不会返回 undefined，而是返回 42。

&emsp;在讨论代理的功能时会用到以下术语。

+ handler 包含陷阱的占位符对象。
+ traps 提供属性访问的方法，这类似于操作系统中陷阱的概念。
+ target 代理虚拟化的对象。它通常用作代理的存储后端。根据目标验证关于对象不可扩展性或不可配置属性的不变量（保持不变的语义）。
+ invariants 实现自定义操作时保持不变的语义称为不变量。如果你违反处理程序的不变量，则会抛出一个 TypeError。

### 撤销 Proxy

&emsp;Proxy.revocable() 方法被用来创建可撤销的 Proxy 对象。这意味着 proxy 可以通过 revoke 函数来撤销，并且关闭代理。此后，代理上的任意的操作都会导致 TypeError。

```javascript
var revocable = Proxy.revocable({}, {
  get: function(target, name) {
    return "[[" + name + "]]";
  }
});
var proxy = revocable.proxy;
console.log(proxy.foo); // "[[foo]]"

revocable.revoke();

console.log(proxy.foo); // TypeError is thrown
proxy.foo = 1           // TypeError again
delete proxy.foo;       // still TypeError
typeof proxy            // "object", typeof doesn't trigger any trap
```

## 反射（Reflection）

&emsp;Reflect 是一个内置对象，它提供了可拦截 JavaScript 操作的方法。该方法和代理句柄类似，但 Reflect 方法并不是一个函数对象。

&emsp;Reflect 有助于将默认操作从处理程序转发到目标。

&emsp;以 Reflect.has() 为例，你可以将 in 运算符作为函数：

```javascript
Reflect.has(Object, "assign"); // true
```

### 更好的 apply 函数

&emsp;在 ES5 中，我们通常使用 Function.prototype.apply() 方法调用一个具有给定 this 值和 arguments 数组（或类数组对象）的函数。

```javascript
Function.prototype.apply.call(Math.floor, undefined, [1.75]);
```

&emsp;使用 Reflect.apply，这变得不那么冗长和容易理解：

```javascript
Reflect.apply(Math.floor, undefined, [1.75]); // 1;

Reflect.apply(String.fromCharCode, undefined, [104, 101, 108, 108, 111]); // "hello"

Reflect.apply(RegExp.prototype.exec, /ab/, ['confabulation']).index; // 4

Reflect.apply(''.charAt, 'ponies', [3]); // "i"
```

### 检查属性定义是否成功

&emsp;使用 Object.defineProperty，如果成功返回一个对象，否则抛出一个 TypeError，你将使用 try...catch 块来捕获定义属性时发生的任何错误。因为 Reflect.defineProperty 返回一个布尔值表示的成功状态，你可以在这里使用 if...else 块：

```javascript
if (Reflect.defineProperty(target, property, attributes)) {
  // success
} else {
  // failure
}
```

### Proxy

&emsp;Proxy 对象用于创建一个对象的代理，从而实现基本操作的拦截和自定义（如属性查找、赋值、枚举、函数调用等）。

&emsp;`handler`：包含捕捉器（trap）的占位符对象，可译为处理器对象。`traps`：提供属性访问的方法。这类似于操作系统中捕获器的概念。`target`：被 Proxy 代理虚拟化的对象。它常被作为代理的存储后端。根据目标验证关于对象不可扩展性或不可配置属性的不变量（保持不变的语义）。

&emsp;语法：

```javascript
const p = new Proxy(target, handler)
```

&emsp;参数：

+ target：要使用 Proxy 包装的目标对象（可以是任何类型的对象，包括原生数组，函数，甚至另一个代理）。
+ handler：一个通常以函数作为属性的对象，各属性中的函数分别定义了在执行各种操作时代理 p 的行为。

&emsp;Proxy.revocable() 创建一个可撤销的 Proxy 对象。

#### Proxy.revocable()

&emsp;Proxy.revocable() 方法可以用来创建一个可撤销的代理对象。

&emsp;语法：

```javascript
Proxy.revocable(target, handler);
```

+ target：将用 Proxy 封装的目标对象。可以是任何类型的对象，包括原生数组，函数，甚至可以是另外一个代理对象。
+ handler：一个对象，其属性是一批可选的函数，这些函数定义了对应的操作被执行时代理的行为。

&emsp;返回值：返回一个包含了代理对象本身和它的撤销方法的可撤销 Proxy 对象。

&emsp;该方法的返回值是一个对象，其结构为： {"proxy": proxy, "revoke": revoke}，其中：

+ proxy：表示新生成的代理对象本身，和用一般方式 new Proxy(target, handler) 创建的代理对象没什么不同，只是它可以被撤销掉。
+ revoke：撤销方法，调用的时候不需要加任何参数，就可以撤销掉和它一起生成的那个代理对象。

&emsp;一旦某个代理对象被撤销，它将变得几乎完全不可调用，在它身上执行任何的可代理操作都会抛出 TypeError 异常（注意，可代理操作一共有 14 种，执行这 14 种操作以外的操作不会抛出异常）。一旦被撤销，这个代理对象便不可能被直接恢复到原来的状态，同时和它关联的目标对象以及处理器对象都有可能被垃圾回收掉。再次调用撤销方法 revoke() 则不会有任何效果，但也不会报错。

```javascript
var revocable = Proxy.revocable({}, {
  get(target, name) {
    return "[[" + name + "]]";
  }
});
var proxy = revocable.proxy;
console.log(proxy.foo); // "[[foo]]"

revocable.revoke();

console.log(proxy.foo); // 抛出 TypeError
proxy.foo = 1           // 还是 TypeError
delete proxy.foo;       // 又是 TypeError
typeof proxy            // "object"，因为 typeof 不属于可代理操作
```

#### handler 对象的方法

&emsp;handler 对象是一个容纳一批特定属性的占位符对象。它包含有 Proxy 的各个捕获器（trap）。

&emsp;所有的捕捉器是可选的。如果没有定义某个捕捉器，那么就会保留源对象的默认行为。

&emsp;⏩

## 反射（Reflection）

&emsp;Reflect 是一个内置对象，它提供了可拦截 JavaScript 操作的方法。该方法和代理句柄类似，但 Reflect 方法并不是一个函数对象。

&emsp;Reflect 有助于将默认操作从处理程序转发到目标。

&emsp;以 Reflect.has() 为例，你可以将 in 运算符作为函数：

```javascript
Reflect.has(Object, "assign"); // true
```

&emsp;














## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
+ [你不知道的JavaScript——异步编程（下）生成器](https://blog.51cto.com/u_15080030/3505569)
+ [图解JavaScript生成器和迭代器](https://zhuanlan.zhihu.com/p/183124536)
