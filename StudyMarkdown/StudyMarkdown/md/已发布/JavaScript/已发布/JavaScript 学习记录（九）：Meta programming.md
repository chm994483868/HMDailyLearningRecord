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

&emsp;下面对上面提到的一些重要概念进行解读。

## Proxy

&emsp;Proxy 对象用于创建一个对象的代理，从而实现基本操作的拦截和自定义（如属性查找、赋值、枚举、函数调用等）。

&emsp;`handler`：包含捕捉器（trap）的占位符对象，可译为处理器对象。

&emsp;一个空的 handler 参数将会创建一个与被代理对象行为几乎完全相同的代理对象。通过在 handler 对象上定义一组处理函数，你可以自定义被代理对象的一些特定行为。例如，通过定义 get() 你就可以自定义被代理对象的属性访问器。

&emsp;选择性代理属性访问器，下面示例中，被代理对象有两个属性：notProxied 和 proxied。我们定义了一个处理函数，它为 proxied 属性返回一个不同的值，而其他属性返回原值。

```javascript
const target = {
  notProxied: "original value",
  proxied: "original value"
};

const handler = {
  get: function(target, prop, receiver) {
    if (prop === "proxied") {
      return "replaced value";
    }
    return Reflect.get(...arguments);
  }
};

const proxy = new Proxy(target, handler);

console.log(proxy.notProxied); // "original value"
console.log(proxy.proxied);    // "replaced value"
```

&emsp;`traps`：提供属性访问的方法。这类似于操作系统中捕获器的概念。

&emsp;`target`：被 Proxy 代理虚拟化的对象，它常被作为代理的存储后端。根据目标验证关于对象不可扩展性或不可配置属性的不变量（保持不变的语义）。

&emsp;语法：

```javascript
const p = new Proxy(target, handler)
```

&emsp;参数：

+ target：要使用 Proxy 包装的目标对象（可以是任何类型的对象，包括原生数组，函数，甚至另一个代理）。
+ handler：一个通常以函数作为属性的对象，各属性中的函数分别定义了在执行各种操作时代理 p 的行为。

&emsp;Proxy.revocable() 创建一个可撤销的 Proxy 对象。

### Proxy.revocable()

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

## handler 对象的方法

&emsp;handler 对象是一个容纳一批特定属性的占位符对象。它包含有 Proxy 的各个捕获器（trap）。

&emsp;所有的捕捉器是可选的。如果没有定义某个捕捉器，那么就会保留源对象的默认行为。

+ `handler.getPrototypeOf()` 是一个代理（Proxy）方法，当读取代理对象的原型时，该方法就会被调用。
  `Object.getPrototypeOf()` 方法返回指定对象的原型（内部 [[Prototype]] 属性的值）。

+ `handler.setPrototypeOf()` 方法主要用来拦截 Object.setPrototypeOf().
  `Object.setPrototypeOf()` 方法设置一个指定的对象的原型（即，内部 [[Prototype]] 属性）到另一个对象或 null。

+ `handler.isExtensible()` 方法用于拦截对象的 Object.isExtensible()。
  `Object.isExtensible()` 方法判断一个对象是否是可扩展的（是否可以在它上面添加新的属性）。

+ `handler.preventExtensions()` 方法用于设置对 Object.preventExtensions() 的拦截。
  `Object.preventExtensions()` 方法让一个对象变的不可扩展，也就是永远不能再添加新的属性。
  
+ `handler.getOwnPropertyDescriptor()` 方法是 Object.getOwnPropertyDescriptor() 的钩子。
  `Object.getOwnPropertyDescriptor()` 方法返回指定对象上一个自有属性对应的属性描述符。（自有属性指的是直接赋予该对象的属性，不需要从原型链上进行查找的属性） 

+ `handler.defineProperty()` 用于拦截对对象的 Object.defineProperty() 操作。
  `Object.defineProperty()` 方法会直接在一个对象上定义一个新属性，或者修改一个对象的现有属性，并返回此对象。

+ `handler.has()` 方法是针对 in 操作符的代理方法。（in 操作符的捕捉器）
  `in` 如果指定的属性在指定的对象或其原型链中，则 in 运算符返回 true。

&emsp;下面拓展学习一下 in 运算符的知识点。

&emsp;如果指定的属性在指定的对象或其原型链中，则 in 运算符返回 true。

```javascript
const car = { make: 'Honda', model: 'Accord', year: 1998 };
console.log('make' in car); // expected output: true
delete car.make;
if ('make' in car === false) {
  car.make = 'Suzuki';
}
console.log(car.make); // expected output: "Suzuki"
```

&emsp;语法：

```javascript
prop in object
```

&emsp;参数：

+ `prop`：一个字符串类型或者 symbol 类型的属性名或者数组索引（非 symbol 类型将会强制转为字符串）。
+ `objectName`：检查它（或其原型链）是否包含具有指定名称的属性的对象。

&emsp;下面的例子演示了一些 in 运算符的用法。

```javascript
// 数组
var trees = new Array("redwood", "bay", "cedar", "oak", "maple");

console.log(0 in trees); // 返回 true
console.log(3 in trees); // 返回 true
console.log(6 in trees); // 返回 false
console.log("bay" in trees); // 返回 false (必须使用索引号，而不是数组元素的值)
console.log("length" in trees); // 返回 true (length 是一个数组属性)
console.log(Symbol.iterator in trees); // 返回 true (数组可迭代，只在 ES2015+ 上有效)

// 内置对象
console.log("PI" in Math); // 返回 true

// 自定义对象
var mycar = { make: "Honda", model: "Accord", year: 1998 };
console.log("make" in mycar); // 返回 true
console.log("model" in mycar); // 返回 true
```

&emsp;in 右操作数必须是一个对象值。例如，你可以指定使用 String 构造函数创建的字符串，但不能指定字符串文字。

```javascript
var color1 = new String("green");
console.log("length" in color1); // 返回 true
var color2 = "coral";
console.log("length" in color2); // 报错 (color2 不是对象)
```

&emsp;对被删除或值为 undefined 的属性使用 in，如果你使用 delete 运算符删除了一个属性，则 in 运算符对所删除属性返回 false。如果你只是将一个属性的值赋值为 undefined，而没有删除它，则 in 运算仍然会返回 true。

```javascript
var mycar = {make: "Honda", model: "Accord", year: 1998};
delete mycar.make;
console.log("make" in mycar);  // 返回 false

var trees = new Array("redwood", "bay", "cedar", "oak", "maple");
delete trees[3];
console.log(3 in trees); // 返回 false
```

```javascript
var mycar = {make: "Honda", model: "Accord", year: 1998};
mycar.make = undefined;
console.log("make" in mycar);  // 返回 true
```

```javascript
var trees = new Array("redwood", "bay", "cedar", "oak", "maple");
trees[3] = undefined;
console.log(3 in trees); // 返回 true
```

&emsp;继承属性，如果一个属性是从原型链上继承来的，in 运算符也会返回 true。

```javascript
console.log("toString" in {}); // 返回 true
```

+ `handler.get()` 方法用于拦截对象的读取属性操作。

+ `handler.set()` 方法是设置属性值操作的捕获器。

+ `handler.deleteProperty()` 方法用于拦截对对象属性的 delete 操作。
  `delete` 操作符用于删除对象的某个属性，如果没有指向这个属性的引用，那它最终会被释放。

+ `handler.ownKeys()` 方法用于拦截 Reflect.ownKeys()。（Object.getOwnPropertyNames 方法和 Object.getOwnPropertySymbols 方法的捕捉器）
  `Object.getOwnPropertyNames()` 方法返回一个由指定对象的所有自身属性的属性名（包括不可枚举属性但不包括 Symbol 值作为名称的属性）组成的数组。 
  `Object.getOwnPropertySymbols()` 方法返回一个给定对象自身的所有 Symbol 属性的数组。

+ `handler.apply()` 方法用于拦截函数的调用。

+ `handler.construct()` 方法用于拦截 new 操作符，为了使 new 操作符在生成的 Proxy 对象上生效，用于初始化代理的目标对象自身必须具有 [[Construct]] 内部方法（即 new target 必须是有效的）。
  `new` 运算符创建一个用户定义的对象类型的实例或具有构造函数的内置对象的实例。

### handler 示例

#### 基础示例

&emsp;在以下简单的例子中，当对象中不存在属性名时，默认返回值为 37。下面的代码以此展示了 get handler 的使用场景。

```javascript
const handler = {
    get: function(obj, prop) {
        return prop in obj ? obj[prop] : 37;
    }
};

const p = new Proxy({}, handler);
p.a = 1;
p.b = undefined;

console.log(p.a, p.b);      // 1, undefined
console.log('c' in p, p.c); // false, 37
```

#### 无操作转发代理

&emsp;在以下例子中，我们使用了一个原生 JavaScript 对象，代理会将所有应用到它的操作转发到这个对象上。

```javascript
let target = {};
let p = new Proxy(target, {});

p.a = 37;   // 操作转发到目标

console.log(target.a);    // 37. 操作已经被正确地转发
```

#### 验证

&emsp;通过代理，你可以轻松地验证向一个对象的传值。下面的代码借此展示了 set handler 的作用。

```javascript
let validator = {
  set: function(obj, prop, value) {
    if (prop === 'age') {
      if (!Number.isInteger(value)) {
        throw new TypeError('The age is not an integer');
      }
      if (value > 200) {
        throw new RangeError('The age seems invalid');
      }
    }

    // The default behavior to store the value
    obj[prop] = value;

    // 表示成功
    return true;
  }
};

let person = new Proxy({}, validator);

person.age = 100;

console.log(person.age);
// 100

person.age = 'young';
// 抛出异常: Uncaught TypeError: The age is not an integer

person.age = 300;
// 抛出异常: Uncaught RangeError: The age seems invalid
```

#### 扩展构造函数

&emsp;方法代理可以轻松地通过一个新构造函数来扩展一个已有的构造函数。这个例子使用了 construct 和 apply。

```javascript
function extend(sup, base) {
  var descriptor = Object.getOwnPropertyDescriptor(
    base.prototype, "constructor"
  );
  base.prototype = Object.create(sup.prototype);
  var handler = {
    construct: function(target, args) {
      var obj = Object.create(base.prototype);
      this.apply(target, obj, args);
      return obj;
    },
    apply: function(target, that, args) {
      sup.apply(that, args);
      base.apply(that, args);
    }
  };
  var proxy = new Proxy(base, handler);
  descriptor.value = proxy;
  Object.defineProperty(base.prototype, "constructor", descriptor);
  return proxy;
}

var Person = function (name) {
  this.name = name
};

var Boy = extend(Person, function (name, age) {
  this.age = age;
});

Boy.prototype.sex = "M";

var Peter = new Boy("Peter", 13);
console.log(Peter.sex);  // "M"
console.log(Peter.name); // "Peter"
console.log(Peter.age);  // 13
```

#### 操作 DOM 节点

&emsp;有时，我们可能需要互换两个不同的元素的属性或类名。下面的代码以此为目标，展示了 set handler 的使用场景。

```javascript
let view = new Proxy({
  selected: null
}, {
  set: function(obj, prop, newval) {
    let oldval = obj[prop];

    if (prop === 'selected') {
      if (oldval) {
        oldval.setAttribute('aria-selected', 'false');
      }
      if (newval) {
        newval.setAttribute('aria-selected', 'true');
      }
    }

    // 默认行为是存储被传入 setter 函数的属性值
    obj[prop] = newval;

    // 表示操作成功
    return true;
  }
});

let i1 = view.selected = document.getElementById('item-1');
console.log(i1.getAttribute('aria-selected')); // 'true'

let i2 = view.selected = document.getElementById('item-2');
console.log(i1.getAttribute('aria-selected')); // 'false'
console.log(i2.getAttribute('aria-selected')); // 'true'
```

#### 值修正及附加属性

&emsp;以下 products 代理会计算传值并根据需要转换为数组。这个代理对象同时支持一个叫做 latestBrowser 的附加属性，这个属性可以同时作为 getter 和 setter。

```javascript
let products = new Proxy({
  browsers: ['Internet Explorer', 'Netscape']
}, {
  get: function(obj, prop) {
    // 附加一个属性
    if (prop === 'latestBrowser') {
      return obj.browsers[obj.browsers.length - 1];
    }

    // 默认行为是返回属性值
    return obj[prop];
  },
  set: function(obj, prop, value) {
    // 附加属性
    if (prop === 'latestBrowser') {
      obj.browsers.push(value);
      return;
    }

    // 如果不是数组，则进行转换
    if (typeof value === 'string') {
      value = [value];
    }

    // 默认行为是保存属性值
    obj[prop] = value;

    // 表示成功
    return true;
  }
});

console.log(products.browsers); // ['Internet Explorer', 'Netscape']
products.browsers = 'Firefox';  // 如果不小心传入了一个字符串
console.log(products.browsers); // ['Firefox'] <- 也没问题，得到的依旧是一个数组

products.latestBrowser = 'Chrome';
console.log(products.browsers);      // ['Firefox', 'Chrome']
console.log(products.latestBrowser); // 'Chrome'
```

#### 通过属性查找数组中的特定对象

&emsp;以下代理为数组扩展了一些实用工具。如你所见，通过 Proxy 我们可以灵活地 "定义" 属性，而不需要使用 Object.defineProperties 方法。以下例子可以用于通过单元格来查找表格中的一行。在这种情况下，target 是 table.rows。

```javascript
let products = new Proxy(
  [
    { name: "Firefox", type: "browser" },
    { name: "SeaMonkey", type: "browser" },
    { name: "Thunderbird", type: "mailer" },
  ],
  {
    get: function (obj, prop) {
      // 默认行为是返回属性值， prop ?通常是一个整数
      if (prop in obj) {
        return obj[prop];
      }

      // 获取 products 的 number; 它是 products.length 的别名
      if (prop === "number") {
        return obj.length;
      }

      let result,
        types = {};

      for (let product of obj) {
        if (product.name === prop) {
          result = product;
        }
        if (types[product.type]) {
          types[product.type].push(product);
        } else {
          types[product.type] = [product];
        }
      }

      // 通过 name 获取 product
      if (result) {
        return result;
      }

      // 通过 type 获取 products
      if (prop in types) {
        return types[prop];
      }

      // 获取 product type
      if (prop === "types") {
        return Object.keys(types);
      }

      return undefined;
    },
  }
);

console.log(products[0]); // { name: 'Firefox', type: 'browser' }
console.log(products["Firefox"]); // { name: 'Firefox', type: 'browser' }
console.log(products["Chrome"]); // undefined
console.log(products.browser); // [{ name: 'Firefox', type: 'browser' }, { name: 'SeaMonkey', type: 'browser' }]
console.log(products.types); // ['browser', 'mailer']
console.log(products.number); // 3
```

## Reflect

&emsp;Reflect 是一个内置的对象，它提供拦截 JavaScript 操作的方法。这些方法与 proxy handlers 的方法相同。Reflect 不是一个函数对象，因此它是不可构造的。

&emsp;与大多数全局对象不同 Reflect 并非一个构造函数，所以不能通过 new 运算符对其进行调用，或者将 Reflect 对象作为一个函数来调用。Reflect 的所有属性和方法都是静态的（就像 Math 对象）。

&emsp;Reflect 对象提供了以下静态方法，这些方法与 proxy handler methods 的命名相同。

&emsp;其中的一些方法与 Object 相同，尽管二者之间存在某些细微上的差别。

### 静态方法

&emsp;`Reflect.apply(target, thisArgument, argumentsList)` 对一个函数进行调用操作，同时可以传入一个数组作为调用参数。和 Function.prototype.apply() 功能类似。

&emsp;`Reflect.construct(target, argumentsList[, newTarget])` 对构造函数进行 new 操作，相当于执行 new target(...args)。

&emsp;`Reflect.defineProperty(target, propertyKey, attributes)` 和 Object.defineProperty() 类似。如果设置成功就会返回 true

&emsp;`Reflect.deleteProperty(target, propertyKey)` 作为函数的 delete 操作符，相当于执行 `delete target[name]`。

&emsp;`Reflect.get(target, propertyKey[, receiver])` 获取对象身上某个属性的值，类似于 `target[name]`。

&emsp;`Reflect.getOwnPropertyDescriptor(target, propertyKey)` 类似于 `Object.getOwnPropertyDescriptor()`。如果对象中存在该属性，则返回对应的属性描述符，否则返回 undefined。

&emsp;`Reflect.getPrototypeOf(target)` 类似于 Object.getPrototypeOf()。

&emsp;`Reflect.has(target, propertyKey)` 判断一个对象是否存在某个属性，和 in 运算符 的功能完全相同。

&emsp;`Reflect.isExtensible(target)` 类似于 Object.isExtensible().

&emsp;`Reflect.ownKeys(target)` 返回一个包含所有自身属性（不包含继承属性）的数组。(类似于 Object.keys(), 但不会受 enumerable 影响).

&emsp;`Reflect.preventExtensions(target)` 类似于 Object.preventExtensions()，返回一个 Boolean。

&emsp;`Reflect.set(target, propertyKey, value[, receiver])` 将值分配给属性的函数。返回一个 Boolean，如果更新成功，则返回 true。

&emsp;`Reflect.setPrototypeOf(target, prototype)` 设置对象原型的函数。返回一个 Boolean，如果更新成功，则返回 true。

### Examples

&emsp;检测一个对象是否存在特定属性：

```javascript
const duck = {
  name: "Maurice",
  color: "white",
  greeting: function () {
    console.log(`Quaaaack! My name is ${this.name}`);
  },
};

console.log(Reflect.has(duck, "color")); // true
console.log(Reflect.has(duck, "haircut")); // false
```

&emsp;返回这个对象自身的属性：

```javascript
console.log(Reflect.ownKeys(duck)); // [ "name", "color", "greeting" ]
```

&emsp;为这个对象添加一个新的属性：

```javascript
console.log(Reflect.set(duck, 'eyes', 'black')); // returns "true" if successful. "duck" now contains the property "eyes: 'black'"
console.log(duck.eyes); // black
```

## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
