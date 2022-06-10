# JavaScript 学习记录（八）：Iterators and generators

&emsp;处理集合中的每个项是很常见的操作。JavaScript 提供了许多迭代集合的方法，从简单的 for 循环到 map() 和 filter()。迭代器和生成器将迭代的概念直接带入核心语言，并提供了一种机制来自定义 for...of 循环的行为。

## 迭代协议

&emsp;作为 ECMAScript 2015 的一组补充规范，迭代协议并不是新的内置实现或语法，而是协议。这些协议可以被任何遵循某些约定的对象来实现。

&emsp;迭代协议具体分为两个协议：可迭代协议和迭代器协议。

### 可迭代协议

&emsp;可迭代协议允许 JavaScript 对象定义或定制它们的迭代行为，例如，在一个 for..of 结构中，哪些值可以被遍历到。一些内置类型同时是内置可迭代对象，并且有默认的迭代行为，比如 Array 或者 Map，而其他内置类型则不是（比如 Object)）。

&emsp;要成为可迭代对象， 一个对象必须实现 @@iterator 方法。这意味着对象（或者它原型链上的某个对象）必须有一个键为 @@iterator 的属性，可通过常量 Symbol.iterator 访问该属性：`[Symbol.iterator]` -- 一个无参数的函数，其返回值为一个符合迭代器协议的对象。

&emsp;当一个对象需要被迭代的时候（比如被置入一个 for...of 循环时），首先，会不带参数调用它的 @@iterator 方法，然后使用此方法返回的迭代器获得要迭代的值。

&emsp;值得注意的是调用此零个参数函数时，它将作为对可迭代对象的方法进行调用。因此，在函数内部，this 关键字可用于访问可迭代对象的属性，以决定在迭代过程中提供什么。

&emsp;此函数可以是普通函数，也可以是生成器函数，以便在调用时返回迭代器对象。在此生成器函数的内部，可以使用 yield 提供每个条目。

#### Symbol.iterator

&emsp;Symbol.iterator 为每一个对象定义了默认的迭代器。该迭代器可以被 for...of 循环使用。

&emsp;当需要对一个对象进行迭代时（比如开始用于一个 for..of 循环中），它的 @@iterator 方法都会在不传参情况下被调用，返回的迭代器用于获取要迭代的值。

&emsp;一些内置类型拥有默认的迭代器行为，其他类型（如 Object）则没有。下列内置类型拥有默认的 @@iterator 方法：

+ `Array.prototype[@@iterator]()`
+ `TypedArray.prototype[@@iterator]()`
+ `String.prototype[@@iterator]()`
+ `Map.prototype[@@iterator]()`
+ `Set.prototype[@@iterator]()`

&emsp;自定义迭代器，我们可以像下面这样创建自定义的迭代器：

```javascript
var myIterable = {}
myIterable[Symbol.iterator] = function* () {
    yield 1;
    yield 2;
    yield 3;
};
[...myIterable] // [1, 2, 3]
```

&emsp;不符合标准的迭代器，如果一个迭代器 @@iterator 没有返回一个迭代器对象，那么它就是一个不符合标准的迭代器，这样的迭代器将会在运行期抛出异常，甚至非常诡异的 Bug，如下示例代码：

```javascript
var nonWellFormedIterable = {}
nonWellFormedIterable[Symbol.iterator] = () => 1
[...nonWellFormedIterable] // TypeError: [] is not a function
```

### 迭代器协议

&emsp;迭代器协议定义了产生一系列值（无论是有限个还是无限个）的标准方式。当值为有限个时，所有的值都被迭代完毕后，则会返回一个默认返回值。

&emsp;只有实现了一个拥有以下语义（semantic）的 next() 方法，一个对象才能成为迭代器。

&emsp;next: 一个无参数的或者可以接受一个参数的函数，返回一个应当拥有以下两个属性的对象。

+ done（boolean）：如果迭代器可以产生序列中的下一个值，则为 false。（这等价于没有指定 done 这个属性。）如果迭代器已将序列迭代完毕，则为 true。这种情况下，value 是可选的，如果它依然存在，即为迭代结束之后的默认返回值。
+ value 迭代器返回的任何 JavaScript 值。done 为 true 时可省略。

&emsp;next() 方法必须返回一个对象，该对象应当有两个属性：done 和 value，如果返回了一个非对象值（比如 false 或 undefined），则会抛出一个 TypeError 异常（"iterator.next() returned a non-object value"）。

> &emsp;note：不可能判断一个特定的对象是否实现了迭代器协议，然而，创造一个同时满足迭代器协议和可迭代协议的对象是很容易的（如下面的示例中所示）。这样做允许一个迭代器能被各种需要可迭代对象的语法所使用。因此，很少会只实现迭代器协议，而不实现可迭代协议。
  ```javascript
  var myIterator = {
    next: function() {
        // ...
    },
    [Symbol.iterator]: function() { return this }
  }
  ```

### 使用迭代协议的例子

&emsp;String 是一个内置的可迭代对象：

```javascript
let someString = "hi";
typeof someString[Symbol.iterator];          // "function"
```

&emsp;String 的默认迭代器会依次返回该字符串的各码点（code point）：

```javascript
let iterator = someString[Symbol.iterator]();
iterator + "";                               // "[object String Iterator]"

iterator.next();                             // { value: "h", done: false }
iterator.next();                             // { value: "i", done: false }
iterator.next();                             // { value: undefined, done: true }
```

&emsp;一些内置的语法结构 —— 比如展开语法 —— 其内部实现也使用了同样的迭代协议：

```javascript
[...someString]                              // ["h", "i"]
```

&emsp;我们可以通过提供自己的 @@iterator 方法，重新定义迭代行为：

```javascript
// 必须构造 String 对象以避免字符串字面量 auto-boxing
var someString = new String("hi");

someString[Symbol.iterator] = function() {
  return { 
    // 只返回一次元素，字符串 "bye" 的迭代器对象
    next: function() {
      if (this._first) {
        this._first = false;
        return { value: "bye", done: false };
      } else {
        return { done: true };
      }
    },
    _first: true
  };
};
```

&emsp;注意重新定义的 @@iterator 方法是如何影响内置语法结构的行为的：

```javascript
[...someString];                              // ["bye"]
someString + "";                              // "hi"
```

### 可迭代对象示例

&emsp;内置可迭代对象，目前所有的内置可迭代对象如下：String、Array、TypedArray、Map 和 Set，它们的原型对象都实现了 @@iterator 方法。

#### 自定义可迭代对象

&emsp;我们可以实现一个自己的可迭代对象，就像这样：

```javascript
var myIterable = {};
myIterable[Symbol.iterator] = function* () {
    yield 1;
    yield 2;
    yield 3;
};
[...myIterable]; // [1, 2, 3]
```

#### 接受可迭代对象的内置 API

&emsp;很多 API 接受可迭代对象作为参数，例如：

+ `new Map([iterable])`
+ `new WeakMap([iterable])`
+ `new Set([iterable])`
+ `new WeakSet([iterable])`

```javascript
new Map([[1, 'a'], [2, 'b'], [3, 'c']]).get(2); // "b"

let myObj = {};

new WeakMap([
    [{}, 'a'],
    [myObj, 'b'],
    [{}, 'c']
]).get(myObj);             // "b"

new Set([1, 2, 3]).has(3); // true
new Set('123').has('2');   // true

new WeakSet(function* () {
    yield {}
    yield myObj
    yield {}
}()).has(myObj);           // true
```

&emsp;另外，还有…

+ `Promise.all(iterable)`
+ `Promise.race(iterable)`
+ `Array.from(iterable)`

#### 需要可迭代对象的语法

&emsp;一些语句和表达式需要可迭代对象，比如 for...of 循环、展开语法、`yield*`，和解构赋值。

```javascript
for (let value of ["a", "b", "c"]){
    console.log(value);
}

// "a"
// "b"
// "c"

[..."abc"]; // ["a", "b", "c"]

function* gen() {
  yield* ["a", "b", "c"];
}

gen().next(); // { value: "a", done: false }

[a, b, c] = new Set(["a", "b", "c"]);
a // "a"
```

&emsp;下面我们扩展一下：展开语法、解构赋值的内容。

##### 展开语法

&emsp;展开语法 (Spread syntax), 可以在函数调用/数组构造时，将数组表达式或者 string 在语法层面展开；还可以在构造字面量对象时，将对象表达式按 key-value 的方式展开。(字面量一般指 `[1, 2, 3]` 或者 `{name: "mdn"}` 这种简洁的构造方式)

```javascript
function sum(x, y, z) {
  return x + y + z;
}

const numbers = [1, 2, 3];

console.log(sum(...numbers));
// expected output: 6

console.log(sum.apply(null, numbers));
// expected output: 6
```

&emsp;函数调用：`myFunction(...iterableObj);`。

&emsp;字面量数组构造或字符串：`[...iterableObj, '4', ...'hello', 6];`。

&emsp;构造字面量对象时，进行克隆或者属性拷贝（ECMAScript 2018 规范新增特性）：`let objClone = { ...obj };`。

&emsp;在函数调用时使用展开语法，等价于 apply 的方式，如果想将数组元素迭代为函数参数，一般使用 Function.prototype.apply 的方式进行调用。

```javascript
function myFunction(x, y, z) { }
var args = [0, 1, 2];
myFunction.apply(null, args);
```

&emsp;有了展开语法，可以这样写：

```javascript
function myFunction(x, y, z) { }
var args = [0, 1, 2];
myFunction(...args);
```

&emsp;所有参数都可以通过展开语法来传值，也不限制多次使用展开语法。

```javascript
function myFunction(v, w, x, y, z) { }
var args = [0, 1];
myFunction(-1, ...args, 2, ...[3]);
```

&emsp;在 new 表达式中应用，使用 new 关键字来调用构造函数时，不能直接使用数组 + apply 的方式（apply 执行的是调用 `[[Call]]`，而不是构造 `[[Construct]]`）。当然，有了展开语法，将数组展开为构造函数的参数就很简单了：

```javascript
var dateFields = [1970, 0, 1]; // 1970 年 1 月 1 日
var d = new Date(...dateFields);
```

&emsp;如果不使用展开语法，想将数组元素传给构造函数，实现方式可能是这样的：

```javascript
function applyAndNew(constructor, args) {
   function partial () {
      return constructor.apply(this, args);
   };
   if (typeof constructor.prototype === "object") {
      partial.prototype = Object.create(constructor.prototype);
   }
   return partial;
}

function myConstructor () {
   console.log("arguments.length: " + arguments.length);
   console.log(arguments);
   this.prop1="val1";
   this.prop2="val2";
};

var myArguments = ["hi", "how", "are", "you", "mr", null];
var myConstructorWithArguments = applyAndNew(myConstructor, myArguments);

console.log(new myConstructorWithArguments);
// (myConstructor 构造函数中):           arguments.length: 6
// (myConstructor 构造函数中):           ["hi", "how", "are", "you", "mr", null]
// ("new myConstructorWithArguments"中): {prop1: "val1", prop2: "val2"}
```

&emsp;构造字面量数组时使用展开语法。

&emsp;没有展开语法的时候，只能组合使用 push, splice, concat 等方法，来将已有数组元素变成新数组的一部分。有了展开语法，通过字面量方式，构造新数组会变得更简单、更优雅：

```javascript
var parts = ['shoulders', 'knees'];
var lyrics = ['head', ...parts, 'and', 'toes']; 
// ["head", "shoulders", "knees", "and", "toes"]
```

&emsp;和参数列表的展开类似， ... 在构造字面量数组时，可以在任意位置多次使用。

&emsp;数组拷贝 (copy):

```javascript
var arr = [1, 2, 3];
var arr2 = [...arr]; // like arr.slice()
arr2.push(4);

// arr2 此时变成 [1, 2, 3, 4]
// arr 不受影响
```

> &emsp;note：实际上，展开语法和 Object.assign() 行为一致，执行的都是浅拷贝 (只遍历一层)。如果想对多维数组进行深拷贝，下面的示例就有些问题了。

```javascript
var a = [[1], [2], [3]];
var b = [...a];
b.shift().shift(); // 1
// Now array a is affected as well: [[2], [3]]
```

&emsp;连接多个数组，Array.concat 函数常用于将一个数组连接到另一个数组的后面。如果不使用展开语法，代码可能是下面这样的：

```javascript
var arr1 = [0, 1, 2];
var arr2 = [3, 4, 5];
// 将 arr2 中所有元素附加到 arr1 后面并返回
var arr3 = arr1.concat(arr2);
```

&emsp;使用展开语法：

```javascript
var arr1 = [0, 1, 2];
var arr2 = [3, 4, 5];
var arr3 = [...arr1, ...arr2];
```

&emsp;Array.unshift 方法常用于在数组的开头插入新元素/数组，不使用展开语法，示例如下：

```javascript
var arr1 = [0, 1, 2];
var arr2 = [3, 4, 5];
// 将 arr2 中的元素插入到 arr1 的开头
Array.prototype.unshift.apply(arr1, arr2) // arr1 现在是 [3, 4, 5, 0, 1, 2]
```

&emsp;如果使用展开语法，代码如下，请注意，这里使用展开语法创建了一个新的 arr1 数组，Array.unshift 方法则是修改了原本存在的 arr1 数组。

&emsp;构造字面量对象时使用展开语法，Rest/Spread Properties for ECMAScript 提议 (stage 4) 对 字面量对象 增加了展开特性。其行为是，将已有对象的所有可枚举 (enumerable) 属性拷贝到新构造的对象中。

&emsp;浅拷贝（Shallow-cloning，不包含 prototype）和对象合并，可以使用更简短的展开语法。而不必再使用 Object.assign() 方式。

```javascript
var obj1 = { foo: 'bar', x: 42 };
var obj2 = { foo: 'baz', y: 13 };

var clonedObj = { ...obj1 };
// 克隆后的对象: { foo: "bar", x: 42 }

var mergedObj = { ...obj1, ...obj2 };
// 合并后的对象: { foo: "baz", x: 42, y: 13 }
```

> &emsp;note：Object.assign() 函数会触发 setters，而展开语法则不会。

> &emsp;note：不能替换或者模拟 Object.assign() 函数。

```javascript
var obj1 = { foo: 'bar', x: 42 };
var obj2 = { foo: 'baz', y: 13 };
const merge = ( ...objects ) => ( { ...objects } );

var mergedObj = merge ( obj1, obj2);
// Object { 0: { foo: 'bar', x: 42 }, 1: { foo: 'baz', y: 13 } }

var mergedObj = merge ( {}, obj1, obj2);
// Object { 0: {}, 1: { foo: 'bar', x: 42 }, 2: { foo: 'baz', y: 13 } }
```

&emsp;在这段代码中，展开操作符 (spread operator) 并没有按预期的方式执行: 而是先将多个解构变为剩余参数 (rest parameter)，然后再将剩余参数展开为字面量对象。

&emsp;只能用于可迭代对象，在数组或函数参数中使用展开语法时，该语法只能用于 可迭代对象：

```javascript
var obj = {'key1': 'value1'};
var array = [...obj]; // TypeError: obj is not iterable
```

&emsp;展开多个值，在函数调用时使用展开语法，请注意不能超过 JavaScript 引擎限制的最大参数个数。

&emsp;剩余语法（剩余参数），剩余语法（Rest syntax）看起来和展开语法完全相同，不同点在于，剩余参数用于解构数组和对象。从某种意义上说，剩余语法与展开语法是相反的：展开语法将数组展开为其中的各个元素，而剩余语法则是将多个元素收集起来并 "凝聚" 为单个元素。

#### 格式不佳的可迭代对象

&emsp;如果一个可迭代对象的 @@iterator 方法不能返回迭代器对象，那么可以认为它是一个格式不佳的（Non-well-formed）可迭代对象 。

&emsp;使用这样的可迭代对象很可能会导致如下的运行时（runtime）异常，或者不可预料的表现：

```javascript
var nonWellFormedIterable = {}
nonWellFormedIterable[Symbol.iterator] = () => 1
[...nonWellFormedIterable] // TypeError: [] is not a function
```

### 迭代器示例

#### 简单迭代器

```javascript
function makeIterator(array) {
    let nextIndex = 0;
    return {
       next: function () {
           return nextIndex < array.length ? {
               value: array[nextIndex++],
               done: false
           } : {
               done: true
           };
       }
    };
}

let it = makeIterator(['哟', '呀']);

console.log(it.next().value); // '哟'
console.log(it.next().value); // '呀'
console.log(it.next().done);  // true
```

#### 无穷迭代器

```javascript
function idMaker() {
    let index = 0;
    return {
       next: function() {
           return {
               value: index++,
               done: false
           };
       }
    };
}

let it = idMaker();

console.log(it.next().value); // 0
console.log(it.next().value); // 1
console.log(it.next().value); // 2
// ...
```

#### 使用生成器

```javascript
function* makeSimpleGenerator(array) {
    let nextIndex = 0;

    while(nextIndex < array.length) {
        yield array[nextIndex++];
    }
}

let gen = makeSimpleGenerator(['哟', '呀']);

console.log(gen.next().value); // '哟'
console.log(gen.next().value); // '呀'
console.log(gen.next().done);  // true

function* idMaker() {
    let index = 0;
    while (true) {
        yield index++;
    }
}

let gen = idMaker();

console.log(gen.next().value); // 0
console.log(gen.next().value); // 1
console.log(gen.next().value); // 2
// ...
```

#### ES2015 类 class 中的迭代器

```javascript
class SimpleClass {
  constructor(data) {
    this.data = data
  }

  [Symbol.iterator]() {
    // Use a new index for each iterator. 
    // This makes multiple iterations over the iterable safe for non-trivial cases, such as use of break or nested looping over the same iterable.
    let index = 0;

    return {
      next: () => {
        if (index < this.data.length) {
          return {value: this.data[index++], done: false}
        } else {
          return {done: true}
        }
      }
    }
  }
}

const simple = new SimpleClass([1,2,3,4,5])

for (const val of simple) {
  console.log(val)   // 1 2 3 4 5
}
```

### 生成器对象到底是一个迭代器，还是一个可迭代对象？

&emsp;生成器对象既是迭代器，也是可迭代对象：

```javascript
let aGeneratorObject = function* (){
    yield 1;
    yield 2;
    yield 3;
}();

typeof aGeneratorObject.next;
// 返回 "function", 因为有一个 next 方法，所以这是一个迭代器

typeof aGeneratorObject[Symbol.iterator];
// 返回 "function", 因为有一个 @@iterator 方法，所以这是一个可迭代对象

aGeneratorObject[Symbol.iterator]() === aGeneratorObject;
// 返回 true，因为 @@iterator 方法返回自身（即迭代器），所以这是一个格式良好的可迭代对象

[...aGeneratorObject];
// 返回 [1, 2, 3]

console.log(Symbol.iterator in aGeneratorObject)
// 返回 true，因为 @@iterator 方法是 aGeneratorObject 的一个属性
```

## for...of

&emsp;for...of 语句在可迭代对象（包括 Array，Map，Set，String，TypedArray，arguments 对象等等）上创建一个迭代循环，调用自定义迭代钩子，并为每个不同属性的值执行语句。

```javascript
const array1 = ['a', 'b', 'c'];

for (const element of array1) {
  console.log(element);
}

// expected output: "a"
// expected output: "b"
// expected output: "c"
```

&emsp;语法，of 后面是 iterable。variable：在每次迭代中，将不同属性的值分配给变量。iterable：被迭代枚举其属性的对象。

```javascript
for (variable of iterable) {
    //statements
}
```

&emsp;迭代 Array，如果你不想修改语句块中的变量 , 也可以使用 const 代替 let。

&emsp;迭代 String：

```javascript
let iterable = "boo";

for (let value of iterable) {
  console.log(value);
}
// "b"
// "o"
// "o"
```

&emsp;迭代 TypedArray：

```javascript
let iterable = new Uint8Array([0x00, 0xff]);

for (let value of iterable) {
  console.log(value);
}
// 0
// 255
```

&emsp;迭代 Map：

```javascript
let iterable = new Map([["a", 1], ["b", 2], ["c", 3]]);

for (let entry of iterable) {
  console.log(entry);
}
// ["a", 1]
// ["b", 2]
// ["c", 3]

for (let [key, value] of iterable) {
  console.log(value);
}
// 1
// 2
// 3
```

&emsp;迭代 Set，迭代 arguments 对象：

```javascript
(function() {
  for (let argument of arguments) {
    console.log(argument);
  }
})(1, 2, 3);

// 1
// 2
// 3
```

&emsp;迭代 DOM 集合，迭代 DOM 元素集合，比如一个 NodeList 对象：下面的例子演示给每一个 article 标签内的 p 标签添加一个 "read" 类。

```javascript
// 注意：这只能在实现了 NodeList.prototype[Symbol.iterator] 的平台上运行
let articleParagraphs = document.querySelectorAll("article > p");

for (let paragraph of articleParagraphs) {
  paragraph.classList.add("read");
}
```

&emsp;关闭迭代器，对于 for...of 的循环，可以由 break, throw 或 return 终止。在这些情况下，迭代器关闭。

```javascript
function* foo(){
  yield 1;
  yield 2;
  yield 3;
};

for (let o of foo()) {
  console.log(o);
  break; // closes iterator, triggers return
}
```

&emsp;迭代生成器，你还可以迭代一个生成器：

```javascript
function* fibonacci() { // 一个生成器函数
    let [prev, curr] = [0, 1];
    for (;;) { // while (true) {
        [prev, curr] = [curr, prev + curr];
        yield curr;
    }
}

for (let n of fibonacci()) {
     console.log(n);
    // 当 n 大于 1000 时跳出循环
    if (n >= 1000)
        break;
}
```

&emsp;不要重用生成器，生成器不应该重用，即使 for...of 循环的提前终止，例如通过 break 关键字。在退出循环后，生成器关闭，并尝试再次迭代，不会产生任何进一步的结果。

```javascript
var gen = (function *(){
    yield 1;
    yield 2;
    yield 3;
})();
for (let o of gen) {
    console.log(o);
    break;// 关闭生成器
}

// 生成器不应该重用，以下没有意义！
for (let o of gen) {
    console.log(o);
}
```

&emsp;迭代其他可迭代对象，你还可以迭代显式实现可迭代协议的对象：

```javascript
var iterable = {
  [Symbol.iterator]() {
    return {
      i: 0,
      next() {
        if (this.i < 3) {
          return { value: this.i++, done: false };
        }
        return { value: undefined, done: true };
      }
    };
  }
};

for (var value of iterable) {
  console.log(value);
}
// 0
// 1
// 2
```

&emsp;for...of 与 for...in 的区别，无论是 for...in 还是 for...of 语句都是迭代一些东西。它们之间的主要区别在于它们的迭代方式。

&emsp;for...in 语句以任意顺序迭代对象的可枚举属性。

&emsp;for...of 语句遍历可迭代对象定义要迭代的数据。

&emsp;以下示例显示了与Array一起使用时，for...of循环和for...in循环之间的区别。

&emsp;以下示例显示了与 Array 一起使用时，for...of 循环和 for...in 循环之间的区别。

```javascript
Object.prototype.objCustom = function() {};
Array.prototype.arrCustom = function() {};

let iterable = [3, 5, 7];
iterable.foo = 'hello';

for (let i in iterable) {
  console.log(i); // logs 0, 1, 2, "foo", "arrCustom", "objCustom"
}

for (let i in iterable) {
  if (iterable.hasOwnProperty(i)) {
    console.log(i); // logs 0, 1, 2, "foo"
  }
}

for (let i of iterable) {
  console.log(i); // logs 3, 5, 7
}
```

```javascript
Object.prototype.objCustom = function() {};
Array.prototype.arrCustom = function() {};

let iterable = [3, 5, 7];
iterable.foo = 'hello';
```

&emsp;每个对象将继承 objCustom 属性，并且作为 Array 的每个对象将继承 arrCustom 属性，因为将这些属性添加到 Object.prototype 和 Array.prototype。由于继承和原型链，对象 iterable 继承属性 objCustom 和 arrCustom。

```javascript
for (let i in iterable) {
  console.log(i); // logs 0, 1, 2, "foo", "arrCustom", "objCustom"
}
```

&emsp;此循环仅以原始插入顺序记录 iterable 对象的可枚举属性。它不记录数组元素 3, 5, 7 或 hello，因为这些不是枚举属性。但是它记录了数组索引以及 arrCustom 和 objCustom。

```javascript
for (let i in iterable) {
  if (iterable.hasOwnProperty(i)) {
    console.log(i); // logs 0, 1, 2, "foo"
  }
}
```

&emsp;这个循环类似于第一个，但是它使用 hasOwnProperty() 来检查，如果找到的枚举属性是对象自己的（不是继承的）。如果是，该属性被记录。记录的属性是 0, 1, 2 和 foo，因为它们是自身的属性（不是继承的）。属性 arrCustom 和 objCustom 不会被记录，因为它们是继承的。

```javascript
for (let i of iterable) {
  console.log(i); // logs 3, 5, 7
}
```

&emsp;该循环迭代并记录 iterable 作为可迭代对象定义的迭代值，这些是数组元素 3, 5, 7，而不是任何对象的属性。

## Generator

&emsp;生成器对象是由一个 generator function 返回的，并且它符合可迭代协议和迭代器协议。

```javascript
function* gen() {
  yield 1;
  yield 2;
  yield 3;
}

let g = gen();
// "Generator { }"
```

### Generator.prototype.next()

&emsp;返回一个由 yield 表达式生成的值。

&emsp;next() 方法返回一个包含属性 done 和 value 的对象。该方法也可以通过接受一个参数用以向生成器传值。

&emsp;`gen.next(value)`，value 向生成器传递的值，返回的对象包含两个属性：

+ done（boolean 类型）- 如果迭代器超过迭代序列的末尾，则值为 true。在这种情况下，value 可选地指定迭代器的返回值。如果迭代器能够生成序列中的下一个值，则值为 false。这相当于没有完全指定 done 属性。
+ value - 迭代器返回的任意的 JavaScript 值。当 done 的值为 true 时可以忽略该值。  
  
&emsp;使用 next() 方法，下面的例子展示了一个简单的生成器，以及调用 next 后方法的返回值：

```javascript
function* gen() {
  yield 1;
  yield 2;
  yield 3;
}

var g = gen(); // "Generator { }"
g.next();      // "Object { value: 1, done: false }"
g.next();      // "Object { value: 2, done: false }"
g.next();      // "Object { value: 3, done: false }"
g.next();      // "Object { value: undefined, done: true }"
```

&emsp;向生成器传值，在此示例中，使用值调用 next。请注意，第一次调用没有记录任何内容，因为生成器最初没有产生任何结果。

```javascript
function* gen() {
  while(true) {
    var value = yield null;
    console.log(value);
  }
}

var g = gen();
g.next(1);
// "{ value: null, done: false }"
g.next(2);
// 2
// "{ value: null, done: false }"
```

### Generator.prototype.return()

&emsp;返回给定的值并结束生成器。

&emsp;`gen.return(value)`，value 需要返回的值，返回该函数参数中给定的值。

&emsp;使用 return() 以下例子展示了一个简单的生成器和 return 方法的使用。

```javascript
function* gen() {
  yield 1;
  yield 2;
  yield 3;
}

var g = gen();

g.next();        // { value: 1, done: false }
g.return("foo"); // { value: "foo", done: true }
g.next();        // { value: undefined, done: true }
```

&emsp;如果对已经处于 "完成" 状态的生成器调用 return(value)，则生成器将保持在 "完成" 状态。如果没有提供参数，则返回对象的 value 属性与示例最后的 .next() 方法相同。如果提供了参数，则参数将被设置为返回对象的 value 属性的值。

```javascript
function* gen() {
  yield 1;
  yield 2;
  yield 3;
}

var g = gen();
g.next(); // { value: 1, done: false }
g.next(); // { value: 2, done: false }
g.next(); // { value: 3, done: false }
g.next(); // { value: undefined, done: true }
g.return(); // { value: undefined, done: true }
g.return(1); // { value: 1, done: true }
```

### Generator.prototype.throw()

&emsp;throw() 方法用来向生成器抛出异常，并恢复生成器的执行，返回带有 done 及 value 两个属性的对象。

&emsp;`gen.throw(exception)`，exception 用于抛出的异常。使用 Error 的实例对调试非常有帮助。

&emsp;返回值，带有两个属性的对象：

+ done（boolean 类型）- 如果迭代器已经返回了迭代序列的末尾，则值为 true。在这种情况下，可以指定迭代器 value 的返回值。如果迭代能够继续生产在序列中的下一个值，则值为 false。这相当与不指定 done 属性的值。
+ value - 迭代器返回的任何 JavaScript 值。当 done 是 true 的时候可以省略。

&emsp;使用 throw()，下面的例子展示了一个简单的生成器并使用 throw 方法向该生成器抛出一个异常，该异常通常可以通过 try...catch 块进行捕获。

```javascript
function* gen() {
  while(true) {
    try {
       yield 42;
    } catch(e) {
      console.log("Error caught!");
    }
  }
}

var g = gen();
g.next(); // { value: 42, done: false }
g.throw(new Error("Something went wrong")); // "Error caught!"
```

## `function*`

&emsp;`function*` 这种声明方式（function 关键字后跟一个星号）会定义一个生成器函数（generator function），它返回一个 Generator 对象。

```javascript
function* generator(i) {
  yield i;
  yield i + 10;
}

const gen = generator(10);

console.log(gen.next().value); // expected output: 10
console.log(gen.next().value); // expected output: 20
```

&emsp;你也可以使用构造函数 GeneratorFunction 或 `function* expression` 定义生成器函数 。

```javascript
function* name([param[, param[, ... param]]]) { statements }
```

&emsp;name：函数名，param：要传递给函数的一个参数的名称，一个函数最多可以有 255 个参数。statements：普通 JS 语句。

### `function* expression`

&emsp;`function*` 关键字可以在表达式内部定义一个生成器函数。 

```javascript
const foo = function*() {
  yield 'a';
  yield 'b';
  yield 'c';
};

let str = '';
for (const val of foo()) {
  str = str + val;
}

console.log(str); // expected output: "abc"
```

```javascript
function* [name]([param1[, param2[, ..., paramN]]]) {
   statements
}
```

&emsp;name 函数名，在声明匿名函数时可以省略，函数名称只是函数体中的一个本地变量。paramN 传入函数的一个参数名，一个函数最多有 255 个参数。statements 函数体，普通 JS 语句。

&emsp;`function*` 表达式和 `function*` 声明比较相似，并具有几乎相同的语法。`function*` 表达式和 `function*` 声明之间主要区别就是函数名，即在创建匿名函数时，`function*` 表达式可以省略函数名。

### GeneratorFunction

&emsp;GeneratorFunction 构造器生成新的生成器函数（`function*`）对象。在 JavaScript 中，生成器函数实际上都是 GeneratorFunction 的实例对象。注意，GeneratorFunction 并不是一个全局对象。它可以通过下面的代码获取。

```javascript
Object.getPrototypeOf(function*(){}).constructor
```

&emsp;语法：

```javascript
new GeneratorFunction ([arg1[, arg2[, ...argN]],] functionBody)
```

&emsp;arg1, arg2, ... argN 函数使用的名称作为形式参数名称。每个必须是一个字符串，对应于一个有效的 JavaScript 标识符或这样的字符串的列表，用逗号分隔；如 "x", "theValue" 或 "a,b"。

&emsp;functionBody 一个包含多条表示 JavaScript 函数体语句的字符串。

&emsp;当创建函数时，将使用 `GeneratorFunction` 构造函数创建的生成器函数对象进行解析。这比使用 `function*` 表达式 声明生成器函数效率更低，并且在代码中调用它，因为这些函数与其余的代码一起被解析。传递给函数的所有参数按照它们被传递的顺序被视为要创建的函数中参数的标识符的名称。

> &emsp;note：使用 GeneratorFunction 构造函数创建的生成器函数不会为其创建上下文创建闭包；它们始终在全局范围内创建。当运行它们时，它们只能访问自己的本地变量和全局变量，而不是从 GeneratorFunction 构造函数调用的范围的变量。这与使用 eval 与生成函数表达式的代码不同。

&emsp;将 GeneratorFunction 构造函数调用为函数（不使用 new 运算符）与将其作为构造函数调用的效果相同。

&emsp;从 GeneratorFunction 构造函数创建一个生成器函数：

```javascript
var GeneratorFunction = Object.getPrototypeOf(function*(){}).constructor
var g = new GeneratorFunction("a", "yield a * 2");
var iterator = g(10);
console.log(iterator.next().value); // 20
```

### Generator 概述

&emsp;生成器函数在执行时能暂停，后面又能从暂停处继续执行。

&emsp;调用一个生成器函数并不会马上执行它里面的语句，而是返回一个这个生成器的 迭代器（iterator）对象。当这个迭代器的 next() 方法被首次（后续）调用时，其内的语句会执行到第一个（后续）出现 yield 的位置为止，yield 后紧跟迭代器要返回的值。或者如果用的是 `yield*`（多了个星号），则表示将执行权移交给另一个生成器函数（当前生成器暂停执行）。

&emsp;next() 方法返回一个对象，这个对象包含两个属性：value 和 done，value 属性表示本次 yield 表达式的返回值，done 属性为布尔类型，表示生成器后续是否还有 yield 语句，即生成器函数是否已经执行完毕并返回。

&emsp;调用 next() 方法时，如果传入了参数，那么这个参数会传给上一条执行的 yield 语句左边的变量，例如下面例子中的 x：

```javascript
function *gen(){
    yield 10;
    x=yield 'foo';
    yield x;
}

var gen_obj=gen();
console.log(gen_obj.next()); // 执行 yield 10，返回 10
console.log(gen_obj.next()); // 执行 yield 'foo'，返回 'foo'
console.log(gen_obj.next(100)); // 将 100 赋给上一条 yield 'foo' 的左值，即执行 x=100，返回 100
console.log(gen_obj.next()); // 执行完毕，value 为 undefined，done 为 true
```

&emsp;当在生成器函数中显式 return 时，会导致生成器立即变为完成状态，即调用 next() 方法返回的对象的 done 为 true。如果 return 后面跟了一个值，那么这个值会作为当前调用 next() 方法返回的 value 值。

```javascript
function* idMaker(){
  var index = 0;
  while(index<3)
    yield index++;
}

var gen = idMaker();
console.log(gen.next().value); // 0
console.log(gen.next().value); // 1
console.log(gen.next().value); // 2
console.log(gen.next().value); // undefined
```

&emsp;生成器也可以接受参数：

```javascript
function* idMaker(){
    var index = arguments[0] || 0;
    while(true)
        yield index++;
}

var gen = idMaker(5);
console.log(gen.next().value); // 5
console.log(gen.next().value); // 6
```

&emsp;`yield*` 的示例：

```javascript
function* anotherGenerator(i) {
  yield i + 1;
  yield i + 2;
  yield i + 3;
}

function* generator(i){
  yield i;
  yield* anotherGenerator(i); // 移交执行权
  yield i + 10;
}

var gen = generator(10);

console.log(gen.next().value); // 10
console.log(gen.next().value); // 11
console.log(gen.next().value); // 12
console.log(gen.next().value); // 13
console.log(gen.next().value); // 20
```

&emsp;传递参数：(这里一定要理清 let first = yield 1; 这种样式语句的含义，切记不要把其中的 = 理解为一个赋值操作，NO，这里完全没有赋值的含义，这里并不是说 yield 1 执行返回一个值赋值给 first，let first 和 yield 1 之间其实完全没有任何关系，生成器执行 next 到 yield 1 时，yield 1 只是负责为执行到此处的生成器完整的返回一个值，就是那个 value 和 done 属性的对象，而这里的 let first 只是用来记录 next 函数传的值的，用于下一个 yield 关键字后面的语句，甚至你可以把它理解为 next 函数的一个形参。)

```javascript
function *createIterator() {
    let first = yield 1;
    let second = yield first + 2; // 4 + 2
                                  // first =4 是 next(4) 将参数赋给上一条的
    yield second + 3;             // 5 + 3
}

let iterator = createIterator();

console.log(iterator.next());    // "{ value: 1, done: false }"
console.log(iterator.next(4));   // "{ value: 6, done: false }"
console.log(iterator.next(5));   // "{ value: 8, done: false }"
console.log(iterator.next());    // "{ value: undefined, done: true }"
```

&emsp;显式返回：

```javascript
function* yieldAndReturn() {
  yield "Y";
  return "R"; // 显式返回处，可以观察到 done 也立即变为了 true
  yield "unreachable"; // 不会被执行了
}

var gen = yieldAndReturn()
console.log(gen.next()); // { value: "Y", done: false }
console.log(gen.next()); // { value: "R", done: true }
console.log(gen.next()); // { value: undefined, done: true }
```

&emsp;生成器函数不能当构造器使用：

```javascript
function* f() {}
var obj = new f; // throws "TypeError: f is not a constructor"
```

&emsp;使用迭代器遍历二维数组并转换成一维数组：

```javascript
function* iterArr(arr) {            // 迭代器返回一个迭代器对象
  if (Array.isArray(arr)) {         // 内节点
      for(let i=0; i < arr.length; i++) {
          yield* iterArr(arr[i]);   // (*) 递归
      }
  } else {                          // 离开
      yield arr;
  }
}
// 使用 for-of 遍历：
var arr = ['a', ['b', 'c'], ['d', 'e']];
for(var x of iterArr(arr)) {
        console.log(x);               // a  b  c  d  e
 }

// 或者直接将迭代器展开：
var arr = [ 'a', ['b',[ 'c', ['d', 'e']]]];
var gen = iterArr(arr);
arr = [...gen];                        // ["a", "b", "c", "d", "e"]
```

## yield

&emsp;yield 关键字用来暂停和恢复一个生成器函数。

```javascript
[rv] = yield [expression];
```

&emsp;expression 定义通过迭代器协议从生成器函数返回的值。如果省略，则返回 undefined。rv 返回传递给生成器的 next() 方法的可选值，以恢复其执行。

&emsp;yield 关键字使生成器函数执行暂停，yield 关键字后面的表达式的值返回给生成器的调用者。它可以被认为是一个基于生成器的版本的 return 关键字。

&emsp;yield 关键字实际返回一个 IteratorResult 对象，它有两个属性，value 和 done。value 属性是对 yield 表达式求值的结果，而 done 是 false，表示生成器函数尚未完全完成。

&emsp;一旦遇到 yield 表达式，生成器的代码将被暂停运行，直到生成器的 next() 方法被调用。每次调用生成器的 next() 方法时，生成器都会恢复执行，直到达到以下某个值：

+ yield 导致生成器再次暂停并返回生成器的新值。下一次调用 next() 时，在 yield 之后紧接着的语句继续执行。
+ throw 用于从生成器中抛出异常。这让生成器完全停止执行，并在调用者中继续执行，正如通常情况下抛出异常一样。
+ 到达生成器函数的结尾，在这种情况下，生成器的执行结束，并且 IteratorResult 给调用者返回 undefined 并且 done 为 true。
+ 到达 return 语句，在这种情况下，生成器的执行结束，并将 IteratorResult 返回给调用者，其值是由 return 语句指定的，并且 done 为 true。

&emsp;如果将参数传递给生成器的 next() 方法，则该值将成为生成器当前 yield 操作返回的值。

&emsp;在生成器的代码路径中的 yield 运算符，以及通过将其传递给 Generator.prototype.next() 指定新的起始值的能力之间，生成器提供了强大的控制力。

&emsp;以下代码是一个生成器函数的声明：

```javascript
function* countAppleSales () {
  var saleList = [3, 7, 5];
  for (var i = 0; i < saleList.length; i++) {
    yield saleList[i];
  }
}
```

&emsp;一旦生成器函数已定义，可以通过构造一个迭代器来使用它。

```javascript
var appleStore = countAppleSales(); // Generator { }
console.log(appleStore.next()); // { value: 3, done: false }
console.log(appleStore.next()); // { value: 7, done: false }
console.log(appleStore.next()); // { value: 5, done: false }
console.log(appleStore.next()); // { value: undefined, done: true }
```

## `yield*`

&emsp;`yield*` 表达式用于委托给另一个 generator 或可迭代对象。

```javascript
yield* [[expression]];
```

&emsp;expression 返回一个可迭代对象的表达式。

&emsp;`yield*` 表达式迭代操作数，并产生它返回的每个值。`yield*` 表达式本身的值是当迭代器关闭时返回的值（即 done 为 true 时）。

&emsp;委托给其他生成器，以下代码中，g1() yield 出去的每个值都会在 g2() 的 next() 方法中返回，就像那些 yield 语句是写在 g2() 里一样。

```javascript
function* g1() {
  yield 2;
  yield 3;
  yield 4;
}

function* g2() {
  yield 1;
  yield* g1();
  yield 5;
}

var iterator = g2();

console.log(iterator.next()); // { value: 1, done: false }
console.log(iterator.next()); // { value: 2, done: false }
console.log(iterator.next()); // { value: 3, done: false }
console.log(iterator.next()); // { value: 4, done: false }
console.log(iterator.next()); // { value: 5, done: false }
console.log(iterator.next()); // { value: undefined, done: true }
```

&emsp;看到这里我们大概明白了一些嵌套数组平铺展开的操作了。

&emsp;委托给其他可迭代对象，除了生成器对象这一种可迭代对象，`yield*` 还可以 yield 其它任意的可迭代对象，比如说数组、字符串、arguments 对象等等。

```javascript
function* g3() {
  yield* [1, 2];
  yield* "34";
  yield* arguments;
}

var iterator = g3(5, 6);

console.log(iterator.next()); // { value: 1, done: false }
console.log(iterator.next()); // { value: 2, done: false }
console.log(iterator.next()); // { value: "3", done: false }
console.log(iterator.next()); // { value: "4", done: false }
console.log(iterator.next()); // { value: 5, done: false }
console.log(iterator.next()); // { value: 6, done: false }
console.log(iterator.next()); // { value: undefined, done: true }
```

&emsp;`yield*` 表达式的值，`yield*` 是一个表达式，不是语句，所以它会有自己的值。

```javascript
function* g4() {
  yield* [1, 2, 3];
  return "foo";
}

var result;

function* g5() {
  result = yield* g4();
}

var iterator = g5();

console.log(iterator.next()); // { value: 1, done: false }
console.log(iterator.next()); // { value: 2, done: false }
console.log(iterator.next()); // { value: 3, done: false }
console.log(iterator.next()); // { value: undefined, done: true },
                              // 此时 g4() 返回了 { value: "foo", done: true }

console.log(result);          // "foo"
```

&emsp;迭代器和生成器 的铺垫完成了，下面我们从整体角度来看这两个概念。

&emsp;在 JavaScript 中，迭代器是一个对象，它定义一个序列，并在终止时可能返回一个返回值。更具体地说，迭代器是通过使用 next() 方法实现 Iterator protocol 的任何一个对象，该方法返回具有两个属性的对象： value，这是序列中的 next 值；和 done，如果已经迭代到序列中的最后一个值，则它为 true。如果 value 和 done 一起存在，则它是迭代器的返回值。

&emsp;一旦创建，迭代器对象可以通过重复调用 next() 显式地迭代。迭代一个迭代器被称为消耗了这个迭代器，因为它通常只能执行一次。在产生终止值之后，对 next() 的额外调用应该继续返回 {done：true}。

&emsp;Javascript 中最常见的迭代器是 Array 迭代器，它只是按顺序返回关联数组中的每个值。虽然很容易想象所有迭代器都可以表示为数组，但事实并非如此。数组必须完整分配，但迭代器仅在必要时使用，因此可以表示无限大小的序列，例如 0 和无穷大之间的整数范围。

&emsp;这是一个可以做到这一点的例子。它允许创建一个简单的范围迭代器，它定义了从开始（包括）到结束（独占）间隔步长的整数序列。它的最终返回值是它创建的序列的大小，由变量 iterationCount 跟踪。

```javascript
function makeRangeIterator(start = 0, end = Infinity, step = 1) {
    let nextIndex = start;
    let iterationCount = 0;

    const rangeIterator = {
       next: function() {
           let result;
           if (nextIndex < end) {
               result = { value: nextIndex, done: false }
               nextIndex += step;
               iterationCount++;
               return result;
           }
           return { value: iterationCount, done: true }
       }
    };
    return rangeIterator;
}
```

&emsp;使用这个迭代器看起来像这样：

```javascript
let it = makeRangeIterator(1, 10, 2);

let result = it.next();
while (!result.done) {
 console.log(result.value); // 1 3 5 7 9
 result = it.next();
}

console.log("Iterated over sequence of size: ", result.value); // 5
```

&emsp;生成器函数：虽然自定义的迭代器是一个有用的工具，但由于需要显式地维护其内部状态，因此需要谨慎地创建。生成器函数提供了一个强大的选择：它允许你定义一个包含自有迭代算法的函数，同时它可以自动维护自己的状态。 

&emsp;生成器函数使用 `function*` 语法编写。最初调用时，生成器函数不执行任何代码，而是返回一种称为 Generator 的迭代器。通过调用生成器的下一个方法消耗值时，Generator 函数将执行，直到遇到 yield 关键字。

&emsp;可以根据需要多次调用该函数，并且每次都返回一个新的 Generator，但每个 Generator 只能迭代一次。

&emsp;我们现在可以调整上面的例子了。 此代码的行为是相同的，但实现更容易编写和读取。

```javascript
function* makeRangeIterator(start = 0, end = Infinity, step = 1) {
    for (let i = start; i < end; i += step) {
        yield i;
    }
}

var a = makeRangeIterator(1,10,2)
a.next() // {value: 1, done: false}
a.next() // {value: 3, done: false}
a.next() // {value: 5, done: false}
a.next() // {value: 7, done: false}
a.next() // {value: 9, done: false}
a.next() // {value: undefined, done: true}
```

&emsp;可迭代对象：若一个对象拥有迭代行为，比如在 for...of 中会循环哪些值，那么那个对象便是一个可迭代对象。一些内置类型，如 Array 或 Map 拥有默认的迭代行为，而其他类型（比如 Object）则没有。

&emsp;为了实现可迭代，一个对象必须实现 @@iterator 方法，这意味着这个对象（或其原型链中的任意一个对象）必须具有一个带 Symbol.iterator 键（key）的属性。

&emsp;可以多次迭代一个迭代器，或者只迭代一次。程序员应该知道是哪种情况。只能迭代一次的 Iterables（例如 Generators）通常从它们的 @@iterator 方法中返回它本身，其中那些可以多次迭代的方法必须在每次调用 @@iterator 时返回一个新的迭代器。

&emsp;自定义的可迭代对象，我们可以像这样实现自己的可迭代对象：

```javascript
var myIterable = {
  *[Symbol.iterator]() {
    yield 1;
    yield 2;
    yield 3;
  }
}

for (let value of myIterable) {
    console.log(value);
}

// 1
// 2
// 3

// 或者

[...myIterable]; // [1, 2, 3]
```

&emsp;内置可迭代对象，String、Array、TypedArray、Map 和 Set 都是内置可迭代对象，因为它们的原型对象都拥有一个 Symbol.iterator 方法。

&emsp;用于可迭代对象的语法，一些语句和表达式专用于可迭代对象，例如 for-of 循环、展开语法、`yield*` 和 解构赋值。

```javascript
for (let value of ['a', 'b', 'c']) {
    console.log(value);
}
// "a"
// "b"
// "c"

[...'abc']; // ["a", "b", "c"]

function* gen() {
  yield* ['a', 'b', 'c'];
}

gen().next(); // { value: "a", done: false }

[a, b, c] = new Set(['a', 'b', 'c']);
a; // "a"
```

&emsp;高级生成器，生成器会按需计算它们的产生值，这使得它们能够有效的表示一个计算成本很高的序列，甚至是如上所示的一个无限序列。

&emsp;next() 方法也接受一个参数用于修改生成器内部状态。传递给 next() 的参数值会被 yield 接收。要注意的是，传给第一个 next() 的值会被忽略。

&emsp;下面的是斐波那契数列生成器，它使用了 next(x) 来重新启动序列：

```javascript
function* fibonacci() {
  var fn1 = 0;
  var fn2 = 1;
  while (true) {
    var current = fn1;
    fn1 = fn2;
    fn2 = current + fn1;
    var reset = yield current;
    if (reset) {
        fn1 = 0;
        fn2 = 1;
    }
  }
}

var sequence = fibonacci();
console.log(sequence.next().value);     // 0
console.log(sequence.next().value);     // 1
console.log(sequence.next().value);     // 1
console.log(sequence.next().value);     // 2
console.log(sequence.next().value);     // 3
console.log(sequence.next().value);     // 5
console.log(sequence.next().value);     // 8
console.log(sequence.next(true).value); // 0
console.log(sequence.next().value);     // 1
console.log(sequence.next().value);     // 1
console.log(sequence.next().value);     // 2
```

&emsp;你可以通过调用其 throw() 方法强制生成器抛出异常，并传递应该抛出的异常值。这个异常将从当前挂起的生成器的上下文中抛出，就好像当前挂起的 yield 是一个 throw value 语句。如果在抛出的异常处理期间没有遇到 yield，则异常将通过调用 throw() 向上传播，对 next() 的后续调用将导致 done 属性为 true。生成器具有 return(value) 方法，返回给定的值并完成生成器本身。

## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
+ [你不知道的JavaScript——异步编程（下）生成器](https://blog.51cto.com/u_15080030/3505569)
+ [图解JavaScript生成器和迭代器](https://zhuanlan.zhihu.com/p/183124536)
