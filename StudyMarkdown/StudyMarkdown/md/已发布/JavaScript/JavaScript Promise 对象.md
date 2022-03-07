# JavaScript 异步编程（Promise 学习）

&emsp;异步（Asynchronous, async）是与同步（Synchronous, sync）相对的概念。

## 异步概念

&emsp;在传统单线程编程中，程序的运行是同步的（同步不意味着所有步骤同时运行，而是指步骤在一个控制流序列中按顺序执行）。而异步的概念则是不保证同步的概念，也就是说，一个异步过程的执行将不再与原有的序列有顺序关系。简单来理解就是：同步按代码顺序执行，异步不按照代码顺序执行，异步的执行效率更高。(一个是接力赛，一个是短跑赛)

&emsp;以上是关于异步的概念的解释，通俗地解释一下异步：异步就是从主线程发射一个子线程来完成任务。

## 需要异步编程的场景 

&emsp;我们常常用子线程来完成一些可能消耗时间足够长以至于被用户察觉的事情，比如读取一个大文件或者发出一个网络请求。因为子线程独立于主线程，所以即使出现阻塞也不会影响主线程的运行。但是子线程有一个局限：一旦发射了以后就会与主线程失去同步，我们无法确定它的结束，如果结束之后需要处理一些事情，比如处理来自服务器的信息，我们是无法将它合并到主线程中去的。为了解决这个问题，JavaScript 中的异步操作函数往往通过回调函数来实现异步任务的结果处理。

## 回调函数

&emsp;下面示例程序中的 `setTimeout` 就是一个消耗时间较长（3 秒）的过程，它的第一个参数是个回调函数，第二个参数是毫秒数，这个函数执行之后会产生一个子线程，子线程会等待 3 秒，然后执行回调函数 `print`。

```c++
function print() {
    document.getElementById("demo").innerHTML = "SUCCESS!";
}
setTimeout(print, 3000);
console.log("TEST");

// 另一种写法：
setTimeout(function () {
    document.getElementById("demo").innerHTML = "SUCCESS!";
}, 3000);
console.log("TEST");
```

&emsp;`setTimeout` 会在子线程中等待 3 秒，在 `setTimeout` 函数执行之后主线程并没有停止。

## JavaScript Promise

&emsp;Promise 是一个 ECMAScript 6 提供的类，目的是更加优雅地书写复杂的异步任务。由于 Promise 是 ES6 新增加的，所以一些旧的浏览器并不支持，苹果的 Safari 10 和 Windows 的 Edge 14 版本以上浏览器才开始支持 ES6 特性。

### 构造 Promise 对象

&emsp;新建一个 Promise 对象：

```c++
new Promise(function (resolve, reject) {
    // 要做的事情...
});
```

&emsp;通过新建一个 Promise 对象好像并没有看出它怎样 "更加优雅地书写复杂的异步任务"。我们之前遇到的异步任务都是一次异步，如果需要多次调用异步函数呢？例如，如果分三次输出字符串，第一次间隔 1 秒，第二次间隔 4 秒，第三次间隔 3 秒：

```c++
// 首先 1 秒后输出 First
setTimeout(function () {
    console.log("First");
    
    // 然后 4 秒后输出 Second
    setTimeout(function () {
        console.log("Second");
        
        // 最后 3 秒后输出 Third 
        setTimeout(function () {
            console.log("Third");
        }, 3000);
    }, 4000); 
}, 1000);
```

&emsp;这段程序实现了这个功能，但是它是用 "函数瀑布" 来实现的。可想而知，在一个复杂的程序当中，用 "函数瀑布" 实现的程序无论是维护还是异常处理都是一件特别繁琐的事情，而且会让缩进格式变得非常冗赘。

&emsp;现在用 Promise 来实现同样的功能：

```c++
new Promise(function (resolve, reject) {
    
    // 首先 1 秒后输出 First
    setTimeout(function () {
        console.log("First");
        
        // 这里调用了 resolve
        resolve();
    }, 1000);
    
}).then(function () {

    // 然后 4 秒后输出 Second，这里返回一个 Promise 对象
    return new Promise(function (resolve, reject) {
    
        setTimeout(function () {
            console.log("Second");
            
            // 这里调用了 resolve
            resolve();
        }, 4000);
    });    
}).then(function () {
    
    // 最后 3 秒后输出 Third 
    setTimeout(function () {
        console.log("Third");
    }, 3000);
});
```

&emsp;上面示例代码较长，可以下不用理解它，下面会对它进行分析，重点我们关注在：Promise 将嵌套格式的代码变成了顺序格式的代码。

### Promise 的使用

&emsp;Promise 构造函数只有一个参数，是一个函数，这个函数在构造之后会直接被异步运行，所以称之为 **起始函数**。**起始函数** 包含两个参数 `resolve` 和 `reject`。

&emsp;当 Promise 被构造时，起始函数会被异步执行：

```c++
new Promise(function (resolve, reject) {
    console.log("Run");
});
```

&emsp;这段程序会直接输出 Run。`resolve` 和 `reject` 都是函数，其中调用 `resolve` 代表一切正常，`reject` 是出现异常时所调用的：

```c++
new Promise(function (resolve, reject) {
    var a = 0;
    var b = 1;
    
    if (b == 0) reject("Divide zero");
    else resolve(a / b);
}).then(function (value) {
    console.log("a / b = " + value);
}).catch(function (err) {
    console.log(err);
}).finally(function () {
    console.log("End");
});

// 输出
a / b = 0
End
```

&emsp;Promise 类有 `.then()`、`.catch()` 和 `.finally()` 三个方法，这三个方法的参数都是一个函数，`.then()` 可以将参数中的函数添加到当前 Promise 的正常执行序列，`.catch()` 则是设定 Promise 的异常处理序列，`.finally()` 是在 Promise 执行的最后一定会执行的序列。`.then()` 传入的函数会按顺序依次执行，有任何异常都会直接跳到 `catch` 序列：

```c++
new Promise(function (resolve, reject) {
    // 这里输出 1111
    console.log(1111);
    
    // 这里把 2222 作为参数传入下面的 then 参数中的函数做参数，
    // 这里如果不调用 resolve 则函数就不会向下执行了
    resolve(2222);
    console.log("resolve 执行了，这里依然会执行，不要把 resolve 当作 return 看待")
}).then(function (value) {
    // 这里输出上面传来的 2222
    console.log(value);
    
    // 这里把 3333 传入下面的 tnen，这里和上面不同，上面使用的 resolve 执行，这里则是使用 return 返回 
    return 3333;
}).then(function (value) {
    // 这里输出 3333
    console.log(value);
    
    // 这里抛出一个 error
    throw "An error";
}).catch(function (err) {
    // 这里 catch 捕获到上面抛出的 error
    console.log(err);
});

// 输出
1111
resolve 执行了，这里依然会执行，不要把 resolve 当作 return 看待
2222
3333
An error
```

&emsp;`resolve()` 中可以放置一个参数用于向下一个 `then` 传递一个值，`then` 中的函数也可以返回一个值传递给 `then`。但是，如果 `then` 中返回的是一个 Promise 对象，那么下一个 `then` 将相当于对这个返回的 Promise 进行操作，这一点从刚才的计时器的例子中可以看出来。`reject()` 参数中一般会传递一个异常给之后的 `catch` 函数用于处理异常。

&emsp;但是请注意以下两点：

+ `resolve` 和 `reject` 的作用域只有起始函数，不包括 `then` 以及其他序列；
+ `resolve` 和 `reject` 并不能够使起始函数停止运行，别忘了 `return`。

### Promise 函数

&emsp;上述使用 Promise 的 "计时器" 程序看上去比函数瀑布还要长，所以我们可以将它的核心部分写成一个 Promise 函数：

```c++
function print(delay, message) {
    return new Promise(function (resolve, reject) {
        setTimeout(function () {
            console.log(message);
            resolve();
        }, delay);
    });
}
```

&emsp;然后就可以放心大胆的实现程序功能了：

```c++
print(1000, "First").then(function () {
    return print(4000, "Second");
}).then(function () {
    print(3000, "Third");
});
```

&emsp;这种返回值为一个 Promise 对象的函数称作 Promise 函数，它常常用于开发基于异步操作的库。


### 回答常见的问题（FAQ）

&emsp;Q: then、catch 和 finally 序列能否顺序颠倒？
&emsp;A: 可以，效果完全一样。但不建议这样做，最好按 then-catch-finally 的顺序编写程序。

&emsp;Q: 除了 then 块以外，其它两种块能否多次使用？
&emsp;A: 可以，finally 与 then 一样会按顺序执行，但是 catch 块只会执行第一个，除非 catch 块里有异常。所以最好只安排一个 catch 和 finally 块。

&emsp;Q: then 块如何中断？
&emsp;A: then 块默认会向下顺序执行，return 是不能中断的，可以通过 throw 来跳转至 catch 实现中断。

&emsp;Q: 什么时候适合用 Promise 而不是传统回调函数？
&emsp;A: 当需要多次顺序执行异步操作的时候，例如，如果想通过异步方法先后检测用户名和密码，需要先异步检测用户名，然后再异步检测密码的情况下就很适合 Promise。

&emsp;Q: Promise 是一种将异步转换为同步的方法吗？
&emsp;A: 完全不是。Promise 只不过是一种更良好的编程风格。

&emsp;Q: 什么时候我们需要再写一个 then 而不是在当前的 then 接着编程？
&emsp;A: 当你又需要调用一个异步任务的时候。

### 异步函数

&emsp;异步函数（async function）是 ECMAScript 2017 (ECMA-262) 标准的规范，几乎被所有浏览器所支持，除了 Internet Explorer。

&emsp;在 Promise 中我们编写过一个 Promise 函数：

```c++
function print(delay, message) {

    return new Promise(function (resolve, reject) {
        setTimeout(function () {
            console.log(message);
            resolve();
        }, delay);
    });
}
```

&emsp;然后用不同的时间间隔输出了三行文本：

```c++
print(1000, "First").then(function () {
    return print(4000, "Second");
}).then(function () {
    print(3000, "Third");
});
```

&emsp;使用 **异步函数（`async function`）** 可以将这段代码变得更好看：

```c++
async function asyncFunc() {
    await print(1000, "First");
    await print(4000, "Second");
    await print(3000, "Third");
}
asyncFunc();
```

&emsp;这岂不是将异步操作变得像同步操作一样容易了吗！这次的回答是肯定的，异步函数 `async function` 中可以使用 `await` 指令，`await` 指令后必须跟着一个 Promise，异步函数会在这个 Promise 运行中暂停，直到其运行结束再继续运行。异步函数实际上原理与 Promise 原生 API 的机制是一模一样的，只不过更便于程序员阅读。

&emsp;处理异常的机制将用 `try-catch` 块实现：

```c++
async function asyncFunc() {
    try {
        await new Promise(function (resolve, reject) {
            throw "Some error"; // 或者 reject("Some error")
        });
    } catch (err) {
        console.log(err);
        // 会输出 Some error
    }
}
asyncFunc();
```

&emsp;如果 Promise 有一个正常的返回值，`await` 语句也会返回它：

```c++
async function asyncFunc() {
    let value = await new Promise(
        function (resolve, reject) {
            resolve("Return value");
        }
    );
    console.log(value);
}
asyncFunc();
// 输出
Return value
```

&emsp;Promise 对象代表一个异步操作，有三种状态：Pending（进行中）、Resolved（已完成，又称 Fulfilled）和 Rejected（已失败）。通过回调里的 resolve(data) 将这个 promise 标记为 resolverd，然后进行下一步 then((data)=>{ // do something })，`resolve` 里的参数就是要传入 `then` 的数据。

&emsp;ECMAscript 6 原生提供了 Promise 对象。Promise 对象代表了未来将要发生的事件，用来传递异步操作的消息。

### Promise 对象有以下两个特点

1. 对象的状态不受外界影响。Promise 对象代表一个异步操作，有三种状态：

+ pending: 初始状态，不是成功或失败状态。
+ fulfilled: 意味着操作成功完成。
+ rejected: 意味着操作失败。

&emsp;只有异步操作的结果，可以决定当前是哪一种状态，任何其他操作都无法改变这个状态。这也是 Promise 这个名字的由来，它的英语意思就是「承诺」，表示其他手段无法改变。

2. 一旦状态改变，就不会再变，任何时候都可以得到这个结果。Promise 对象的状态改变，只有两种可能：从 Pending 变为 Resolved 和从 Pending 变为 Rejected。只要这两种情况发生，状态就凝固了，不会再变了，会一直保持这个结果。就算改变已经发生了，你再对 Promise 对象添加回调函数，也会立即得到这个结果。这与事件（Event）完全不同，事件的特点是，如果你错过了它，再去监听，是得不到结果的。

### Promise 优缺点

&emsp;有了 Promise 对象，就可以将异步操作以同步操作的流程表达出来，避免了层层嵌套的回调函数。此外，Promise 对象提供统一的接口，使得控制异步操作更加容易。

&emsp;Promise 也有一些缺点。首先，无法取消 Promise，一旦新建它就会立即执行，无法中途取消。其次，如果不设置回调函数，Promise 内部抛出的错误，不会反应到外部。第三，当处于 Pending 状态时，无法得知目前进展到哪一个阶段（刚刚开始还是即将完成）。

### Promise 创建

&emsp;要想创建一个 promise 对象、可以使用 `new` 来调用 Promise 的构造器来进行实例化。

&emsp;下面是创建 promise 的步骤：

```c++
var promise = new Promise(function(resolve, reject) {
    // 异步处理
    // 处理结束后、调用resolve 或 reject
});
```

&emsp;Promise 构造函数包含一个参数和一个带有 `resolve（解析）` 和 `reject（拒绝）` 两个参数的回调。在回调中执行一些操作（例如异步），如果一切都正常，则调用 `resolve`，否则调用 `reject`。

```c++
var myFirstPromise = new Promise(function(resolve, reject){
    // 当异步代码执行成功时，才会调用 resolve(...)，当异步代码失败时就会调用 reject(...)
    // 在本例中，使用 setTimeout(...) 来模拟异步代码，实际编码时可能是 XHR 请求或是 HTML5 的一些 API 方法
    setTimeout(function(){
        resolve("成功!"); // 代码正常执行！
    }, 250);
});
 
myFirstPromise.then(function(successMessage){
    // successMessage 的值是上面调用 resolve(...) 方法传入的值.
    // successMessage 参数不一定非要是字符串类型，这里只是举个例子
    document.write("Yay! " + successMessage);
});
```

&emsp;对于已经实例化过的 `promise` 对象可以调用 `promise.then()` 方法，传递 `resolve` 和 `reject` 方法作为回调。

&emsp;`promise.then()` 是 `promise` 最为常用的方法。

```c++
promise.then(onFulfilled, onRejected)
```

&emsp;`promise` 简化了对 error 的处理，上面的代码也可以这样写：

```c++
promise.then(onFulfilled).catch(onRejected)
```

## Promise AJAX

&emsp;下面是一个用 Promise 对象实现的 AJAX 操作的例子。

```
function ajax(URL) {
    return new Promise(function (resolve, reject) {
        var req = new XMLHttpRequest(); 
        req.open('GET', URL, true);
        req.onload = function () {
        if (req.status === 200) { 
                resolve(req.responseText);
            } else {
                reject(new Error(req.statusText));
            } 
        };
        req.onerror = function () {
            reject(new Error(req.statusText));
        };
        req.send(); 
    });
}
var URL = "/try/ajax/testpromise.php"; 
ajax(URL).then(function onFulfilled(value){
    document.write('内容是：' + value); 
}).catch(function onRejected(error){
    document.write('错误：' + error); 
});
```

&emsp;上面代码中，`resolve` 方法和 `reject` 方法调用时，都带有参数。它们的参数会被传递给回调函数。`reject` 方法的参数通常是 Error 对象的实例，而 `resolve` 方法的参数除了正常的值以外，还可能是另一个 Promise 实例，比如像下面这样。

```c++
var p1 = new Promise(function(resolve, reject){
  // ... some code
});
 
var p2 = new Promise(function(resolve, reject){
  // ... some code
  resolve(p1);
})
```

&emsp;上面代码中，p1 和 p2 都是 Promise 的实例，但是 p2 的 `resolve` 方法将 p1 作为参数，这时 p1 的状态就会传递给 p2。如果调用的时候，p1 的状态是 pending，那么 p2 的回调函数就会等待 p1 的状态改变；如果 p1 的状态已经是 fulfilled 或者 rejected，那么 p2 的回调函数将会立刻执行。

## Promise.prototype.then 方法：链式操作

&emsp;`Promise.prototype.then` 方法返回的是一个新的 Promise 对象，因此可以采用链式写法。

```c++
getJSON("/posts.json").then(function(json) {
  return json.post;
}).then(function(post) {
  // proceed
});
```

&emsp;上面的代码使用 `then` 方法，依次指定了两个回调函数。第一个回调函数完成以后，会将返回结果作为参数，传入第二个回调函数。如果前一个回调函数返回的是 Promise 对象，这时后一个回调函数就会等待该 Promise 对象有了运行结果，才会进一步调用。

```c++
getJSON("/post/1.json").then(function(post) {
  return getJSON(post.commentURL);
}).then(function(comments) {
  // 对 comments 进行处理
});
```

&emsp;这种设计使得嵌套的异步操作，可以被很容易得改写，从回调函数的 "横向发展" 改为 "向下发展"。

## Promise.prototype.catch 方法：捕捉错误

&emsp;`Promise.prototype.catch` 方法是 `Promise.prototype.then(null, rejection)` 的别名，用于指定发生错误时的回调函数。

```c++
getJSON("/posts.json").then(function(posts) {
  // some code
}).catch(function(error) {
  // 处理前一个回调函数运行时发生的错误
  console.log('发生错误！', error);
});
```

&emsp;Promise 对象的错误具有 "冒泡" 性质，会一直向后传递，直到被捕获为止。也就是说，错误总是会被下一个 `catch` 语句捕获。

```c++
getJSON("/post/1.json").then(function(post) {
  return getJSON(post.commentURL);
}).then(function(comments) {
  // some code
}).catch(function(error) {
  // 处理前两个回调函数的错误
});
```

## Promise.all 方法，Promise.race 方法
&emsp;`Promise.all` 方法用于将多个 Promise 实例，包装成一个新的 Promise 实例。

```c++
var p = Promise.all([p1,p2,p3]);
```

&emsp;上面代码中，`Promise.all` 方法接受一个数组作为参数，p1、p2、p3 都是 Promise 对象的实例。（`Promise.all` 方法的参数不一定是数组，但是必须具有 `iterator` 接口，且返回的每个成员都是 Promise 实例。）

&emsp;`p` 的状态由 `p1`、`p2`、`p3` 决定，分成两种情况。

1. 只有 `p1`、`p2`、`p3` 的状态都变成 fulfilled，`p` 的状态才会变成 fulfilled，此时 `p1`、`p2`、`p3` 的返回值组成一个数组，传递给 `p` 的回调函数。
2. 只要 `p1`、`p2`、`p3` 之中有一个被 rejected，`p` 的状态就变成 rejected，此时第一个被 reject 的实例的返回值，会传递给 p 的回调函数。

&emsp;下面是一个具体的例子。

```c++
// 生成一个 Promise 对象的数组
var promises = [2, 3, 5, 7, 11, 13].map(function(id){
  return getJSON("/post/" + id + ".json");
});
 
Promise.all(promises).then(function(posts) {
  // ...  
}).catch(function(reason){
  // ...
});
```

&emsp;`Promise.race` 方法同样是将多个 Promise 实例，包装成一个新的 Promise 实例。

```c++
var p = Promise.race([p1,p2,p3]);
```

&emsp;上面代码中，只要 `p1`、`p2`、`p3` 之中有一个实例率先改变状态，`p` 的状态就跟着改变。那个率先改变的 Promise 实例的返回值，就传递给 `p` 的返回值。

&emsp;如果 `Promise.all` 方法和 `Promise.race` 方法的参数，不是 Promise 实例，就会先调用下面讲到的 `Promise.resolve` 方法，将参数转为 Promise 实例，再进一步处理。

## Promise.resolve 方法，Promise.reject 方法

&emsp;有时需要将现有对象转为 Promise 对象，`Promise.resolve` 方法就起到这个作用。

```c++
var jsPromise = Promise.resolve($.ajax('/whatever.json'));
```

&emsp;上面代码将 jQuery 生成 deferred 对象，转为一个新的 ES6 的 Promise 对象。

&emsp;如果 `Promise.resolve` 方法的参数，不是具有 `then` 方法的对象（又称 `thenable` 对象），则返回一个新的 Promise 对象，且它的状态为 fulfilled。

```c++
var p = Promise.resolve('Hello');
 
p.then(function (s){
  console.log(s)
});
// Hello
```

&emsp;上面代码生成一个新的 Promise 对象的实例 `p`，它的状态为 fulfilled，所以回调函数会立即执行，`Promise.resolve` 方法的参数就是回调函数的参数。

&emsp;如果 `Promise.resolve` 方法的参数是一个 Promise 对象的实例，则会被原封不动地返回。

&emsp;`Promise.reject(reason)` 方法也会返回一个新的 Promise 实例，该实例的状态为 rejected。`Promise.reject` 方法的参数 `reason`，会被传递给实例的回调函数。

```c++
var p = Promise.reject('出错了');
 
p.then(null, function (s){
  console.log(s)
});
// 出错了
```

&emsp;上面代码生成一个 Promise 对象的实例，状态为 rejected，回调函数会立即执行。

&emsp;以上就是 Promise 对象和使用的一些总结。

## 参考链接
**参考链接:🔗**
+ [Promise 对象](https://wohugb.gitbooks.io/ecmascript-6/content/docs/promise.html)
+ [Promise](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Promise)
+ [ECMAScript 6简介](https://wohugb.gitbooks.io/ecmascript-6/content/docs/intro.html)
+ [JavaScript 教程](https://wangdoc.com/javascript/)
+ [JavaScript 异步编程](https://www.runoob.com/js/js-async.html)
+ [JavaScript Promise 对象](https://www.runoob.com/w3cnote/javascript-promise-object.html)
+ [JSON 教程](https://www.runoob.com/json/json-tutorial.html)
+ [JavaScript JSON](https://www.runoob.com/js/js-json.html)
