# ES6 Map 与 Set 

&emsp;Map 是 ES6 中引入的一种新的数据结构。

## Map 对象

&emsp;Map 对象保存键值对。任何值(对象或者原始值) 都可以作为一个键或一个值。

## Maps 和 Objects 的区别

+ 一个 Object 的键只能是字符串或者 Symbols，但一个 Map 的键可以是任意值。
+ Map 中的键值是有序的（FIFO 原则），而添加到对象中的键则不是。
+ Map 的键值对个数可以从 size 属性获取，而 Object 的键值对个数只能手动计算。
+ Object 都有自己的原型，原型链上的键名有可能和你自己在对象上的设置的键名产生冲突。

## Map 中的 key

&emsp;key 是字符串：

```typescript
var myMap = new Map();
var keyString = "a string";

myMap.set(keyString, "和键 'a string' 关联的值");

console.log(myMap.get(keyString));
console.log(myMap.get('a string'));

// 输出
和键 'a string' 关联的值
和键 'a string' 关联的值
```

&emsp;key 是对象：

```typescript
var myMap = new Map();
var keyObj = {};

myMap.set(keyObj, "和键 keyObj 关联的值");

console.log(myMap.get(keyObj));
console.log(myMap.get({}));

// 输出
和键 keyObj 关联的值
undefined // undefined, 因为 keyObj !== {}
```

&emsp;key 是函数：

```typescript
var myMap = new Map();
var keyFunc = function () { }; // 函数

myMap.set(keyFunc, "和键 keyFunc 关联的值");

console.log(myMap.get(keyFunc))
console.log(myMap.get(function () { }));

// 输出
和键 keyFunc 关联的值
undefined // undefined, 因为 keyFunc !== function () {}
```

&emsp;key 是 NaN：

```typescript
var myMap = new Map();
myMap.set(NaN, "not a number");
console.log(myMap.get(NaN));

var otherNaN = Number("foo");
console.log(myMap.get(otherNaN));

// 输出
not a number
not a number
```

&emsp;虽然 NaN 和任何值甚至和自己都不相等(NaN !== NaN 返回 true)，NaN 作为 Map 的键来说是没有区别的。

## Map 的迭代

&emsp;对 Map 进行遍历，以下两个最高级：

### for...of

```typescript
var myMap = new Map();
myMap.set(0, "zero");
myMap.set(1, "one");

for (var [key, value] of myMap) {
    console.log(key + " = " + value);
}
// 输出
0 = zero
1 = one

// 这个 entries 方法返回一个新的 Iterator 对象，它按插入顺序包含了 Map 对象中每个元素的 [key, value] 数组
for (var [key, value] of myMap.entries()) {
    console.log(key + " = " + value);
}
// 输出
0 = zero
1 = one

// 这个 keys 方法返回一个新的 Iterator 对象，它按插入顺序包含了 Map 对象中每个元素的键
for (var key of myMap.keys()) {
    console.log(key);
}
// 输出
0
1

// 这个 values 方法返回一个新的 Iterator 对象，它按插入顺序包含了 Map 对象中每个元素的值
for (var value of myMap.values()) {
    console.log(value);
}
// 输出
zero
one
```

### forEach()

```typescript
var myMap = new Map();
myMap.set(0, "zero");
myMap.set(1, "one");

myMap.forEach(function (value, key) {
    console.log(key + " = " + value);
}, myMap);
// 输出
0 = zero
1 = one
```

## Map 对象的操作

### Map 与 Array 的转换。

```typescript
var map = new Map();
map.set(1, "111");
map.set(2, "222");
map.set(3, "333");

// map -> array
// 使用 Array.from 函数可以将一个 Map 对象转换成一个二维键值对数组
var arr = Array.from(map); // 二维数组
console.log(arr);

// array -> map
// Map 构造函数可以将一个 二维 键值对数组转换为一个 Map 对象
console.log(new Map(arr));

// 使用 ...rest 运算符
console.log([...map]); // 把 map 中的 entries 扩展为数组元素

// 输出
[ [ 1, '111' ], [ 2, '222' ], [ 3, '333' ] ]
Map(3) { 1 => '111', 2 => '222', 3 => '333' }
[ [ 1, '111' ], [ 2, '222' ], [ 3, '333' ] ]
```

### Map 的克隆

```typescript
var myMap1 = new Map([
  ["key1", "value1"],
  ["key2", "value2"],
]);
var myMap2 = new Map(myMap1); // 向构造方法传入 map 对象

console.log(myMap1 === myMap2);

// 输出
false
```

### Map 的合并

&emsp;

```typescript
var first = new Map([[1, 'one'], [2, 'two'], [3, 'three']]);
var second = new Map([[1, 'uno'], [2, 'dos']]);

// 合并两个 Map 对象时，如果有重复的键值，则后面的会覆盖前面的，对应值即 uno、dos、three
console.log(new Map([...first, ...second]));

// 输出
Map(3) { 1 => 'uno', 2 => 'dos', 3 => 'three' }
```

## Set 对象

&emsp;Set 对象允许你存储任何类型的唯一值，无论是原始值或是对象引用。

### Set 中的特殊值

&emsp;Set 对象存储的值总是唯一的，所以需要判断两个值是否恒等。有几个特殊值需要特殊对待：

+ `+0` 与 `-0` 在存储判断唯一性的时候是恒等的，所以不重复
+ `undefined` 与 `undefined` 是恒等的，所以不重复
+ `NaN` 与 `NaN` 是不恒等的，但是在 Set 中只能存一个，不重复

```typescript
let mySet = new Set();

mySet.add(1); // Set(1) {1}
console.log(mySet);

mySet.add(5); // Set(2) {1, 5}
console.log(mySet);

mySet.add(5); // Set(2) {1, 5} 这里体现了值的唯一性
console.log(mySet);

mySet.add("some text"); // Set(3) {1, 5, "some text"} 这里体现了类型的多样性
console.log(mySet);

var o = {a: 1, b: 2};
mySet.add(o);
console.log(mySet);

mySet.add({a: 1, b: 2}); // 这里体现了对象之间引用不同不恒等，即使值相同，Set 也能存储
console.log(mySet);

// 输出
Set(1) { 1 }
Set(2) { 1, 5 }
Set(2) { 1, 5 }
Set(3) { 1, 5, 'some text' }
Set(4) { 1, 5, 'some text', { a: 1, b: 2 } }
Set(5) { 1, 5, 'some text', { a: 1, b: 2 }, { a: 1, b: 2 } }
```

### 类型转换

&emsp;Array：

```typescript
// Array 转 Set
var mySet = new Set(["value1", "value2", "value3"]);
// 用 ... 操作符，将 Set 转 Array
var myArray = [...mySet];
console.log(myArray);

// String 转 Set
var mySet = new Set('hello');
console.log(mySet);

// 输出
[ 'value1', 'value2', 'value3' ]
Set(4) { 'h', 'e', 'l', 'o' }
```

### Set 对象作用

&emsp;数组去重：

```typescript
var mySet = new Set([1, 2, 3, 4, 4, 5]);
console.log([...mySet]);

// 输出
[ 1, 2, 3, 4, 5 ]
```

&emsp;并集：

```typescript
var a = new Set([1, 2, 3]);
var b = new Set([4, 3, 2]);

var union = new Set([...a, ...b]);
console.log(union);

// 输出
Set(4) { 1, 2, 3, 4 }
```

&emsp;交集：

```typescript
var a = new Set([1, 2, 3]);
var b = new Set([4, 3, 2]);
var intersect = new Set([...a].filter(x => b.has(x)));
console.log(intersect);

// 输出
Set(2) { 2, 3 }
```

&emsp;差集：

```typescript
var a = new Set([1, 2, 3]);
var b = new Set([4, 3, 2]);
var intersect = new Set([...[...a].filter(x => !b.has(x)), ...[...b].filter(x => !a.has(x))]);
console.log(intersect);

// 输出
Set(2) { 1, 4 }
```

## 参考链接
**参考链接:🔗**
+ [ES6 Map 与 Set](https://www.runoob.com/w3cnote/es6-map-set.html)
