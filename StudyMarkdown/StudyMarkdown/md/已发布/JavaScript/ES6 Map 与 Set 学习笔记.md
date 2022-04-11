# ES6 Map 与 Set 

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












## 参考链接
**参考链接:🔗**
+ [ES6 Map 与 Set](https://www.runoob.com/w3cnote/es6-map-set.html)
