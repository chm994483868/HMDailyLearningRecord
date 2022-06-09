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

## `function*`

&emsp;`function*` 这种声明方式（function 关键字后跟一个星号）会定义一个生成器函数（generator function），它返回一个 Generator 对象。

```javascript
function* generator(i) {
  yield i;
  yield i + 10;
}

const gen = generator(10);

console.log(gen.next().value);
// expected output: 10

console.log(gen.next().value);
// expected output: 20
```

&emsp;你也可以使用构造函数 GeneratorFunction 或 `function* expression` 定义生成器函数 。

```javascript
function* name([param[, param[, ... param]]]) { statements }
```

&emsp;name：函数名，param：要传递给函数的一个参数的名称，一个函数最多可以有 255 个参数。statements：普通 JS 语句。









## Generator

## yield

## `yield*`











## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
+ [你不知道的JavaScript——异步编程（下）生成器](https://blog.51cto.com/u_15080030/3505569)
+ [图解JavaScript生成器和迭代器](https://zhuanlan.zhihu.com/p/183124536)
