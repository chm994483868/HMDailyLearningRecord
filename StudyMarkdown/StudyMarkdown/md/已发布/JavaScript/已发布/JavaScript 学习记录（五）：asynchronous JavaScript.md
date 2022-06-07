# JavaScript 学习记录（五）：asynchronous JavaScript

&emsp;异步指两个或两个以上的对象或事件不同时存在或发生（或多个相关事物的发生无需等待其前一事物的完成）。在计算机技术中，"异步" 一 词被用于两大语境。

&emsp;异步通信是一种在双方或多方之间交换消息的方式。其中每个参与方各自在他们方便或可操作的情况下接收并处理消息，而不是在收到消息后立即进行处理。 另外，消息的发送无需等待确认信息，前提是如果出现问题，接收方将请求更正或以其他方式处理该情况。

&emsp;对人类来说，电子邮件就是一种异步通信方式；发送者发送了一封邮件，接着接收者会在方便时读取和回复该邮件，而不是马上这样做。双方可以继续随时发送和接收信息，而无需双方安排何时进行操作。

&emsp;在软件进行异步通信时，一个程序可能会向另一软件（如服务器）请求信息，并在等待回复的同时继续执行其他操作。例如，AJAX（Asynchronous JavaScript and XML）编程技术（现在通常简写为 "Ajax"，不过现在的应用不常用 XML，而是用 JSON）就是这样一种机制，它通过 HTTP 从服务器请求较少的数据，当结果可被返回时才返回结果，而不是立即返回。

&emsp;异步软件设计通过构建代码扩展了异步的概念，按照这种设计编写的代码使得程序能够要求一个任务与先前的一个（或多个）任务一起执行，而无需为了等待它们完成而停止执行。当后来的任务完成时，程序将使用约定好的机制通知先前的任务，以便让它知道任务已经完成，以及如果有结果存在的话，这个结果是可用的。

&emsp;在 JavaScript 代码中，你经常会遇到两种异步编程风格：老派 callbacks，新派 promise。

## 异步 callbacks

&emsp;异步 callbacks 其实就是函数，只不过是作为参数传递给那些在后台执行的其他函数。当那些后台运行的代码结束，就调用 callbacks 函数，通知你工作已经完成，或者其他有趣的事情发生了。使用 callbacks 有一点老套，在一些老派但经常使用的 API 里面，经常会看到这种风格。

&emsp;不是所有的回调函数都是异步的 — 有一些是同步的。一个例子就是使用 Array.prototype.forEach() 来遍历数组。

```javascript
const gods = ['Apollo', 'Artemis', 'Ares', 'Zeus'];

gods.forEach(function (eachName, index){
  console.log(index + '. ' + eachName);
});
```

&emsp;forEach() 需要的参数是一个回调函数，回调函数本身带有两个参数，数组元素和索引值。它无需等待任何事情，立即运行。

&emsp;Promises 是新派的异步代码，现代的 web APIs 经常用到。fetch() API 就是一个很好的例子，它基本上就是一个现代版的更高效的 XMLHttpRequest。

```javascript
fetch('products.json').then(function(response) {
  return response.json();
}).then(function(json) {
  products = json;
  initialize();
}).catch(function(err) {
  console.log('Fetch problem: ' + err.message);
});
```

&emsp;这里 fetch() 只需要一个参数 — 资源的网络 URL — 返回一个 promise。promise 是表示异步操作完成或失败的对象。可以说，它代表了一种中间状态。本质上，这是浏览器说 "我保证尽快给你答复" 的方式，因此得名 "promise"。这两种可能的结果都还没有发生，因此 fetch 操作目前正在等待浏览器试图在将来某个时候完成该操作的结果。然后我们有三个代码块链接到 fetch() 的末尾：

+ 两个 then() 块。两者都包含一个回调函数，如果前一个操作成功，该函数将运行，并且每个回调都接收前一个成功操作的结果作为输入，因此你可以继续对它执行其他操作。每个 .then() 块返回另一个 promise，这意味着可以将多个 .then() 块链接到另一个块上，这样就可以依次执行多个异步操作。
+ 如果其中任何一个 then() 块失败，则在末尾运行 catch() 块 —— 与同步 try...catch 类似，catch() 提供了一个错误对象，可用来报告发生的错误类型。但是请注意，同步 try...catch 不能与 promise 一起工作，尽管它可以与 async/await 一起工作。

&emsp;像 promise 这样的异步操作被放入事件队列中，事件队列在主线程完成处理后运行，这样它们就不会阻止后续 JavaScript 代码的运行。排队操作将尽快完成，然后将结果返回到 JavaScript 环境。

### Promises 对比 callbacks

&emsp;Promises 与旧式 callbacks 有一些相似之处。它们本质上是一个返回的对象，你可以将回调函数附加到该对象上，而不必将回调作为参数传递给另一个函数。然而，Promise是专门为异步操作而设计的，与旧式回调相比具有许多优点：

+ 你可以使用多个 then() 操作将多个异步操作链接在一起，并将其中一个操作的结果作为输入传递给下一个操作。这种链接方式对回调来说要难得多，会使回调以混乱的 "末日金字塔" 告终（也称为回调地狱）。
+ Promise 总是严格按照它们放置在事件队列中的顺序调用。
+ 错误处理要好得多 —— 所有的错误都由块末尾的一个 .catch() 块处理，而不是在 "金字塔" 的每一层单独处理。

&emsp;在最基本的形式中，JavaScript 是一种同步的、阻塞的、单线程的语言，在这种语言中，一次只能执行一个操作。但 web 浏览器定义了函数和 API，允许我们当某些事件发生时不是按照同步方式，而是异步地调用函数(比如，时间的推移，用户通过鼠标的交互，或者获取网络数据)。这意味着你的代码可以同时做几件事情，而不需要停止或阻塞主线程。

## 使用 Promise

&emsp;Promise 对象用于表示一个异步操作的最终完成（或失败）及其结果值。

&emsp;Promise 是一个对象，它代表了一个异步操作的最终完成或者失败。因为大多数人仅仅是使用已创建的 Promise 实例对象，所以这里首先说明怎样使用 Promise，再说明如何创建 Promise。

&emsp;本质上 Promise 是一个函数返回的对象，我们可以在它上面绑定回调函数，这样我们就不需要在一开始把回调函数作为参数传入这个函数了。

&emsp;假设现在有一个名为 createAudioFileAsync() 的函数，它接收一些配置和两个回调函数，然后异步地生成音频文件。一个回调函数在文件成功创建时被调用，另一个则在出现异常时被调用。

&emsp;以下为使用 createAudioFileAsync() 的示例：

```javascript
// 成功的回调函数
function successCallback(result) {
  console.log("音频文件创建成功: " + result);
}

// 失败的回调函数
function failureCallback(error) {
  console.log("音频文件创建失败: " + error);
}

createAudioFileAsync(audioSettings, successCallback, failureCallback)
```

&emsp;更现代的函数会返回一个 Promise 对象，使得你可以将你的回调函数绑定在该 Promise 上。

&emsp;如果函数 createAudioFileAsync() 被重写为返回 Promise 的形式，那么我们可以像下面这样简单地调用它：

```javascript
const promise = createAudioFileAsync(audioSettings);
promise.then(successCallback, failureCallback);
```

&emsp;或者简写为：

```javascript
createAudioFileAsync(audioSettings).then(successCallback, failureCallback);
```

&emsp;我们把这个称为异步函数调用，这种形式有若干优点。

&emsp;不同于 "老式" 的传入回调，在使用 Promise 时，会有以下约定：

+ 在本轮 事件循环 运行完成之前，回调函数是不会被调用的。
+ 即使异步操作已经完成（成功或失败），在这之后通过 then() 添加的回调函数也会被调用。
+ 通过多次调用 then() 可以添加多个回调函数，它们会按照插入顺序进行执行。

&emsp;Promise 很棒的一点就是链式调用（chaining）。

### 链式调用（chaining）

&emsp;连续执行两个或者多个异步操作是一个常见的需求，在上一个操作执行成功之后，开始下一个的操作，并带着上一步操作所返回的结果。我们可以通过创造一个 Promise 链来实现这种需求。

&emsp;then() 函数会返回一个和原来不同的新的 Promise：

```javascript
const promise = doSomething();
const promise2 = promise.then(successCallback, failureCallback);
```

&emsp;或者：

```javascript
const promise2 = doSomething().then(successCallback, failureCallback);
```

&emsp;promise2 不仅表示 doSomething() 函数的完成，也代表了你传入的 successCallback 或者 failureCallback 的完成，这两个函数也可以返回一个 Promise 对象，从而形成另一个异步操作，这样的话，在 promise2 上新增的回调函数会排在这个 Promise 对象的后面。

&emsp;基本上，每一个 Promise 都代表了链中另一个异步过程的完成。

&emsp;在过去，要想做多重的异步操作，会导致经典的回调地狱：

```javascript
doSomething(function(result) {
  doSomethingElse(result, function(newResult) {
    doThirdThing(newResult, function(finalResult) {
      console.log('Got the final result: ' + finalResult);
    }, failureCallback);
  }, failureCallback);
}, failureCallback);
```

&emsp;现在，我们可以把回调绑定到返回的 Promise 上，形成一个 Promise 链：

```javascript
doSomething().then(function(result) {
  return doSomethingElse(result);
})
.then(function(newResult) {
  return doThirdThing(newResult);
})
.then(function(finalResult) {
  console.log('Got the final result: ' + finalResult);
})
.catch(failureCallback);
```

&emsp;then 里的参数是可选的，catch(failureCallback) 是 then(null, failureCallback) 的缩略形式。如下所示，我们也可以用箭头函数来表示：

```javascript
doSomething()
.then(result => doSomethingElse(result))
.then(newResult => doThirdThing(newResult))
.then(finalResult => {
  console.log(`Got the final result: ${finalResult}`);
})
.catch(failureCallback);
```

> &emsp;note：一定要有返回值，否则，callback 将无法获取上一个 Promise 的结果。(如果使用箭头函数，() => x 比 () => { return x; } 更简洁一些，但后一种保留 return 的写法才支持使用多个语句。）。

### Catch 的后续链式操作

&emsp;有可能会在一个回调失败之后继续使用链式操作即使用一个 catch，这对于在链式操作中抛出一个失败之后，再次进行新的操作会很有用。请阅读下面的例子：

```javascript
new Promise((resolve, reject) => {
    console.log('初始化');

    resolve();
})
.then(() => {
    throw new Error('有哪里不对了');

    console.log('执行「这个」');
})
.catch(() => {
    console.log('执行「那个」');
})
.then(() => {
    console.log('执行「这个」，无论前面发生了什么');
});
```

&emsp;输出：

```javascript
初始化
执行「那个」
执行「这个」，无论前面发生了什么
```

> &emsp;note：因为抛出了错误 有 '哪里不对了'，所以前一个 '执行「这个」' 没有被输出。

### 错误传递

&emsp;在之前的回调地狱示例中，你可能记得有 3 次 failureCallback 的调用，而在 Promise 链中只有尾部的一次调用。

```javascript
doSomething()
.then(result => doSomethingElse(result))
.then(newResult => doThirdThing(newResult))
.then(finalResult => console.log(`Got the final result: ${finalResult}`))
.catch(failureCallback);
```

&emsp;通常，一遇到异常抛出，浏览器就会顺着 Promise 链寻找下一个 onRejected 失败回调函数或者由 .catch() 指定的回调函数。这和以下同步代码的工作原理（执行过程）非常相似。

```javascript
try {
  let result = syncDoSomething();
  let newResult = syncDoSomethingElse(result);
  let finalResult = syncDoThirdThing(newResult);
  console.log(`Got the final result: ${finalResult}`);
} catch(error) {
  failureCallback(error);
}
```

&emsp;在 ECMAScript 2017 标准的 async/await 语法糖中，这种异步代码的对称性得到了极致的体现：

```javascript
async function foo() {
  try {
    const result = await doSomething();
    const newResult = await doSomethingElse(result);
    const finalResult = await doThirdThing(newResult);
    console.log(`Got the final result: ${finalResult}`);
  } catch(error) {
    failureCallback(error);
  }
}
```

&emsp;这个例子是在 Promise 的基础上构建的，例如：doSomething() 与之前的函数是相同的。

&emsp;通过捕获所有的错误，甚至抛出异常和程序错误，Promise 解决了回调地狱的基本缺陷。这对于构建异步操作的基础功能而言是很有必要的。

### Promise 拒绝事件

&emsp;当 Promise 被拒绝时，会有下文所述的两个事件之一被派发到全局作用域（通常而言，就是 window；如果是在 web worker 中使用的话，就是 Worker 或者其他 worker-based 接口）。这两个事件如下所示：

+ `rejectionhandled`
  当 Promise 被拒绝、并且在 reject 函数处理该 rejection 之后会派发此事件。

&emsp;当 Promise 被 rejected 且有 rejection 处理器时会在全局触发 rejectionhandled 事件 (通常是发生在 window 下，但是也可能发生在 Worker 中)。应用于调试一般应用回退。当 Promise 被 rejected 且没有 rejection 处理器处理时会触发 unhandledrejection 事件。这两个事件协同工作。

&emsp;如下示例代码可以使用 rejectionhandled 事件在控制台打印出被 rejected 的 Promise，以及被 rejected 的原因：

```javascript
window.addEventListener("rejectionhandled", event => {
  console.log("Promise rejected; reason: " + event.reason);
}, false);
```

+ `unhandledrejection`
  当 Promise 被拒绝，但没有提供 reject 函数来处理该 rejection 时，会派发此事件。
  
&emsp;当 Promise 被 reject 且没有 reject 处理器的时候，会触发 unhandledrejection 事件（通常是发生在 window 下，但是也可能发生在 Worker 中）。这对于调试回退错误处理非常有用。

&emsp;unhandledrejection 继承自 PromiseRejectionEvent，而 PromiseRejectionEvent 又继承自 Event。因此 unhandledrejection 含有 PromiseRejectionEvent 和 Event 的属性和方法。

&emsp;下面通过几个例子来展示 unhandledrejection 事件的使用方式。该事件主要包含两部分有用的信息：

+ `promise`
  特定的 Promise 被 reject 而没有被相应的异常处理方法所处理时
  
+ `reason`
  将会传入异常处理方法中的错误原因（如果存在），查看 [catch()](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Promise/catch) 相关以获取更多细节。

#### 基本的异常上报

&emsp;此示例只是将有关未处理的 Promise rejection 信息打印到控制台。

```javascript
window.addEventListener("unhandledrejection", event => {
  console.warn(`UNHANDLED PROMISE REJECTION: ${event.reason}`);
});
```

&emsp;还可以使用 onunhandledrejection 事件处理程序属性来设置事件侦听器：

```javascript
window.onunhandledrejection = event => {
  console.warn(`UNHANDLED PROMISE REJECTION: ${event.reason}`);
};
```

#### 防止默认处理

&emsp;许多环境 (例如 Node.js ) 默认情况下会向控制台打印未处理的 Promise rejections。你可以通过添加一个处理程序来防止 unhandledrejection 这种情况的发生，该处理程序除了你希望执行的任何其他任务之外，还可以调用 preventDefault() 来取消该事件，从而防止该事件冒泡并由运行时的日志代码处理。这种方法之所以有效，是因为 unhandledrejection 是可以取消的。

```javascript
window.addEventListener('unhandledrejection', function (event) {
  // ...你的代码可以处理未处理的拒绝...

  // 防止默认处理（例如将错误输出到控制台）

  event.preventDefault();
});
```

### preventDefault

&emsp;Event 接口的 preventDefault() 方法，告诉 user agent：如果此事件没有被显式处理，它默认的动作也不应该照常执行。此事件还是继续传播，除非碰到事件侦听器调用 stopPropagation() 或 stopImmediatePropagation()，才停止传播。

#### 阻止默认的点击事件执行

&emsp;选中复选框是点击复选框的默认行为。下面这个例子说明了怎样阻止默认行为的发生：

```javascript
document.querySelector("#id-checkbox").addEventListener("click", function(event) {
         document.getElementById("output-box").innerHTML += "Sorry! <code>preventDefault()</code> won't let you check this!<br>";
         event.preventDefault();
}, false);
```

#### 在编辑域中阻止按键

&emsp;下面的这个例子说明了如何使用 preventDefault() 在文本编辑域中阻止有效的文本输入。

&emsp;HTML:

```javascript
<div class="container">
  <p>Please enter your name using lowercase letters only.</p>

  <form>
    <input type="text" id="my-textbox">
  </form>
</div>
```

&emsp;CSS:

&emsp;当用户按下一个有效按键的时候，我们就给这个 warning box 加上一些样式：

```javascript
.warning {
  border: 2px solid #f39389;
  border-radius: 2px;
  padding: 10px;
  position: absolute;
  background-color: #fbd8d4;
  color: #3b3c40;
}
```

&emsp;JavaScript:

&emsp;这里是相关的 JavaScript 代码。首先，监听 keypress 事件：

```javascript
var myTextbox = document.getElementById('my-textbox');
myTextbox.addEventListener('keypress', checkName, false);
```

&emsp;checkName() 方法可以监听按键并且决定是否允许按键的默认行为发生。

```javascript
function checkName(evt) {
  var charCode = evt.charCode;
  if (charCode != 0) {
    if (charCode < 97 || charCode > 122) {
      evt.preventDefault();
      displayWarning(
        "Please use lowercase letters only."
        + "\n" + "charCode: " + charCode + "\n"
      );
    }
  }
}
```

&emsp;displayWarning() 方法显示了一个问题的通知。这不是一种优雅的方法，但是确实可以达到我们的目的。

```javascript
var warningTimeout;
var warningBox = document.createElement("div");
warningBox.className = "warning";

function displayWarning(msg) {
  warningBox.innerHTML = msg;

  if (document.body.contains(warningBox)) {
    window.clearTimeout(warningTimeout);
  } else {
    // insert warningBox after myTextbox
    myTextbox.parentNode.insertBefore(warningBox, myTextbox.nextSibling);
  }

  warningTimeout = window.setTimeout(function() {
      warningBox.parentNode.removeChild(warningBox);
      warningTimeout = -1;
    }, 2000);
}
```

&emsp;在事件流的任何阶段调用 preventDefault() 都会取消事件，这意味着任何通常被该实现触发并作为结果的默认行为都不会发生。

&emsp;你可以使用 Event.cancelable 来检查该事件是否支持取消。为一个不支持 cancelable 的事件调用 preventDefault() 将没有效果。

### catch

&emsp;catch() 方法返回一个 Promise，并且处理拒绝的情况。它的行为与调用 Promise.prototype.then(undefined, onRejected) 相同。(事实上，calling obj.catch(onRejected) 内部 calls obj.then(undefined, onRejected))

```javascript
p.catch(onRejected);

p.catch(function(reason) {
   // 拒绝
});
```

&emsp;参数 onRejected：当 Promise 被 rejected 时，被调用的一个 Function。该函数拥有一个参数 reason：rejection 的原因。如果 onRejected 抛出一个错误或返回一个本身失败的 Promise，通过 catch() 返回的 Promise 被 rejected；否则，它将显示为成功（resolved）。

&emsp;catch 方法可以用于你的 promise 组合中的错误处理。

&emsp;下面我们继续回到：rejectionhandled 和 unhandledrejection 两种情况，PromiseRejectionEvent 事件都有两个属性，一个是 promise 属性，该属性指向被驳回的 Promise，另一个是 reason 属性，该属性用来说明 Promise 被驳回的原因。

&emsp;因此，我们可以通过以上事件为 Promise 失败时提供补偿处理，也有利于调试 Promise 相关的问题。在每一个上下文中，该处理都是全局的，因此不管源码如何，所有的错误都会在同一个处理函数中被捕捉并处理。

&emsp;一个特别有用的例子：当你使用 Node.js 时，有些依赖模块可能会有未被处理的 rejected promises，这些都会在运行时打印到控制台。你可以在自己的代码中捕捉这些信息，然后添加与 unhandledrejection 相应的处理函数来做分析和处理，或只是为了让你的输出更整洁。举例如下：

```javascript
window.addEventListener("unhandledrejection", event => {
  /* 你可以在这里添加一些代码，以便检查 event.promise 中的 promise 和 event.reason 中的 rejection 原因 */

  event.preventDefault();
}, false);
```

&emsp;调用 event 的 preventDefault() 方法是为了告诉 JavaScript 引擎当 Promise 被拒绝时不要执行默认操作，默认操作一般会包含把错误打印到控制台，Node 就是如此的。理想情况下，在忽略这些事件之前，我们应该检查所有被拒绝的 Promise，来确认这不是代码中的 bug。

### 在旧式回调 API 中创建 Promise

&emsp;可以通过 Promise 的构造器从零开始创建 Promise。这种方式（通过构造器的方式）应当只在封装旧 API 的时候用到。

&emsp;理想状态下，所有的异步函数都已经返回 Promise 了。但有一些 API 仍然使用旧方式来传入的成功（或者失败）的回调。典型的例子就是 setTimeout() 函数：

```javascript
setTimeout(() => saySomething("10 seconds passed"), 10000);
```

&emsp;混用旧式回调和 Promise 可能会造成运行时序问题。如果 saySomething 函数失败了，或者包含了编程错误，那就没有办法捕获它了。这得怪 setTimeout。

&emsp;幸运地是，我们可以用 Promise 来封装它。最好的做法是，将这些有问题的函数封装起来，留在底层，并且永远不要再直接调用它们：

```javascript
const wait = ms => new Promise(resolve => setTimeout(resolve, ms));

wait(10000).then(() => saySomething("10 seconds")).catch(failureCallback);
```

&emsp;通常，Promise 的构造器接收一个执行函数 (executor)，我们可以在这个执行函数里手动地 resolve 和 reject 一个 Promise。既然 setTimeout 并不会真的执行失败，那么我们可以在这种情况下忽略 reject。

### 组合

&emsp;Promise.resolve() 和 Promise.reject() 是手动创建一个已经 resolve 或者 reject 的 Promise 快捷方法。它们有时很有用。

&emsp;Promise.all() 和 Promise.race() 是并行运行异步操作的两个组合式工具。

&emsp;我们可以发起并行操作，然后等多个操作全部结束后进行下一步操作，如下：

```javascript
Promise.all([func1(), func2(), func3()]).then(([result1, result2, result3]) => { /* use result1, result2 and result3 */ });
```

&emsp;可以使用一些聪明的 JavaScript 写法实现时序组合：

```javascript
[func1, func2, func3].reduce((p, f) => p.then(f), Promise.resolve()).then(result3 => { /* use result3 */ });
```

&emsp;通常，我们递归调用一个由异步函数组成的数组时，相当于一个 Promise 链：

```javascript
Promise.resolve().then(func1).then(func2).then(func3);
```

&emsp;我们也可以写成可复用的函数形式，这在函数式编程中极为普遍：

```javascript
const applyAsync = (acc,val) => acc.then(val);
const composeAsync = (...funcs) => x => funcs.reduce(applyAsync, Promise.resolve(x));
```

&emsp;composeAsync() 函数将会接受任意数量的函数作为其参数，并返回一个新的函数，该函数接受一个通过 composition pipeline 传入的初始值。这对我们来说非常有益，因为任一函数可以是异步或同步的，它们能被保证按顺序执行：

```javascript
const transformData = composeAsync(func1, func2, func3);
const result3 = transformData(data);
```

&emsp;在 ECMAScript 2017 标准中，时序组合可以通过使用 async/await 而变得更简单：

```javascript
let result;
for (const f of [func1, func2, func3]) {
  result = await f(result);
}

/* use last result (i.e. result3) */
```

### 时序

&emsp;为了避免意外，即使是一个已经变成 resolve 状态的 Promise，传递给 then() 的函数也总是会被异步调用：

```javascript
Promise.resolve().then(() => console.log(2));
console.log(1); // 1, 2
```

&emsp;传递到 then() 中的函数被置入到一个微任务队列中，而不是立即执行，这意味着它是在 JavaScript 事件队列的所有运行时结束了，且事件队列被清空之后，才开始执行：

```javascript
const wait = ms => new Promise(resolve => setTimeout(resolve, ms));

wait().then(() => console.log(4));
Promise.resolve().then(() => console.log(2)).then(() => console.log(3));
console.log(1); // 1, 2, 3, 4
```

### 嵌套

&emsp;简便的 Promise 链式编程最好保持扁平化，不要嵌套 Promise，因为嵌套经常会是粗心导致的。

&emsp;嵌套 Promise 是一种可以限制 catch 语句的作用域的控制结构写法。明确来说，嵌套的 catch 仅捕捉在其之前同时还必须是其作用域的 failureres，而捕捉不到在其链式以外或者其嵌套域以外的 error。如果使用正确，那么可以实现高精度的错误修复。

```javascript
doSomethingCritical()
.then(result => doSomethingOptional()
  .then(optionalResult => doSomethingExtraNice(optionalResult))
  .catch(e => {console.log(e.message)})) // 即使有异常也会忽略，继续运行;(最后会输出)
.then(() => moreCriticalStuff())
.catch(e => console.log("Critical failure: " + e.message));// 没有输出
```

&emsp;注意，有些代码步骤是嵌套的，而不是一个简单的纯链式，这些语句前与后都被括号 () 包裹着。

&emsp;这个内部的 catch 语句仅能捕获到 doSomethingOptional() 和 doSomethingExtraNice() 的失败，之后就恢复到 moreCriticalStuff() 的运行。重要提醒：如果 doSomethingCritical() 失败，这个错误仅会被最后的（外部）catch 语句捕获到。

### 常见错误

&emsp;在编写 Promise 链时，需要注意以下示例中展示的几个错误：

```javascript
// 错误示例，包含 3 个问题！

doSomething().then(function(result) {
  doSomethingElse(result) // 没有返回 Promise 以及没有必要的嵌套 Promise
  .then(newResult => doThirdThing(newResult));
}).then(() => doFourthThing());
// 最后，是没有使用 catch 终止 Promise 调用链，可能导致没有捕获的异常
```

&emsp;第一个错误是没有正确地将事物相连接。当我们创建新 Promise 但忘记返回它时，会发生这种情况。因此，链条被打破，或者更确切地说，我们有两个独立的链条竞争（同时在执行两个异步而非一个一个的执行）。这意味着 doFourthThing() 不会等待 doSomethingElse() 或 doThirdThing() 完成，并且将与它们并行运行，可能是无意的。单独的链也有单独的错误处理，导致未捕获的错误。

&emsp;第二个错误是不必要地嵌套，实现第一个错误。嵌套还限制了内部错误处理程序的范围，如果是非预期的，可能会导致未捕获的错误。其中一个变体是 Promise 构造函数反模式，它结合了 Promise 构造函数的多余使用和嵌套。

&emsp;第三个错误是忘记用 catch 终止链。这导致在大多数浏览器中不能终止的 Promise 链里的 rejection。

&emsp;一个好的经验法则是总是返回或终止 Promise 链，并且一旦你得到一个新的 Promise，返回它。下面是修改后的平面化的代码：

```javascript
doSomething()
.then(function(result) {
  return doSomethingElse(result);
})
.then(newResult => doThirdThing(newResult))
.then(() => doFourthThing())
.catch(error => console.log(error));
```

> &emsp;note: () => x 是 () => { return x; } 的简写。

&emsp;上述代码的写法就是具有适当错误处理的简单明确的链式写法。使用 async/await 可以解决以上大多数错误，使用 async/await 时，最常见的语法错误就是忘记了 await 关键字。

## Promise 解析

&emsp;一个 Promise 对象代表一个在这个 promise 被创建出来时不一定已知值的代理。它让你能够把异步操作最终的成功返回值或者失败原因和相应的处理程序关联起来。这样使得异步方法可以像同步方法那样返回值：异步方法并不会立即返回最终的值，而是会返回一个 promise，以便在未来某个时候把值交给使用者。

&emsp;一个 Promise 必然处于以下几种状态之一：

+ 待定（pending）：初始状态，既没有被兑现，也没有被拒绝。
+ 已兑现（fulfilled）：意味着操作成功完成。
+ 已拒绝（rejected）：意味着操作失败。

&emsp;待定状态的 Promise 对象要么会通过一个值被兑现，要么会通过一个原因（错误）被拒绝。当这些情况之一发生时，我们用 promise 的 then 方法排列起来的相关处理程序就会被调用。如果 promise 在一个相应的处理程序被绑定时就已经被兑现或被拒绝了，那么这个处理程序也同样会被调用，因此在完成异步操作和绑定处理方法之间不存在竞态条件。

&emsp;因为 Promise.prototype.then 和 Promise.prototype.catch 方法返回的是 promise，所以它们可以被链式调用。

> &emsp;note: 有一些语言中有惰性求值和延迟计算的特性，它们也被称为 "promise"，例如 Scheme。JavaScript 中的 promise 代表的是已经在发生的进程，而且可以通过回调函数实现链式调用。如果你想对一个表达式进行惰性求值，就考虑一下使用无参数的箭头函数，如 f = () => expression 来创建惰性求值的表达式，然后使用 f() 进行求值。

> &emsp;note: 如果一个 promise 已经被兑现或被拒绝，那么我们也可以说它处于 已敲定（settled） 状态。你还会听到一个经常跟 promise 一起使用的术语：已决议（resolved），它表示 promise 已经处于已敲定状态，或者为了匹配另一个 promise 的状态被 "锁定" 了。

&emsp;我们可以用 Promise.prototype.then()、Promise.prototype.catch() 和 Promise.prototype.finally() 这些方法将进一步的操作与一个变为已敲定状态的 promise 关联起来。

&emsp;例如 .then() 方法需要两个参数，第一个参数作为处理已兑现状态的回调函数，而第二个参数则作为处理已拒绝状态的回调函数。每一个 .then() 方法还会返回一个新生成的 promise 对象，这个对象可被用作链式调用。

&emsp;当 .then() 中缺少能够返回 promise 对象的函数时，链式调用就直接继续进行下一环操作。因此，链式调用可以在最后一个 .catch() 之前把所有的处理已拒绝状态的回调函数都省略掉。

&emsp;过早地处理变为已拒绝状态的 promise 会对之后 promise 的链式调用造成影响。不过有时候我们因为需要马上处理一个错误也只能这样做。例如，外面必须抛出某种类型的错误以在链式调用中传递错误状态。另一方面，在没有迫切需要的情况下，可以在最后一个 .catch() 语句时再进行错误处理，这种做法更加简单。.catch() 其实只是没有给处理已兑现状态的回调函数预留参数位置的 .then() 而已。

&emsp;本质上，Promise 是一个对象，代表操作的中间状态 —— 正如它的单词含义 '承诺'，它保证在未来可能返回某种结果。虽然 Promise 并不保证操作在何时完成并返回结果，但是它保证当结果可用时，你的代码能正确处理结果，当结果不可用时，你的代码同样会被执行，来优雅的处理错误。

## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)







