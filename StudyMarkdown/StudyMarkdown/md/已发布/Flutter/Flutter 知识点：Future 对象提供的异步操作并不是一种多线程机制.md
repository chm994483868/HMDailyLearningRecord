# Flutter 知识点：Future 对象提供的异步操作并不是一种多线程机制

&emsp;如果在 Flutter 中遇到了跟异步操作相关的卡顿，那么我们首先要做的便是加深自己对 Flutter 单线程模型 Single-Threaded（或者是 Dart 的单线程模型）的理解。

## Future 并不是一种多线程机制

&emsp;Flutter 中由 Future 对象以及 async 和 await 关键字提供的异步操作，并不是一种多线程机制，它只是一种非阻塞机制，此时的异步操作并没有在另外一条线程中执行，它还是在当前线程执行的。这里可以类比 JavaScript 中的 Promise，同时 Dart 的 Single-Threaded 也可以类比 JavaScript 的 Single-Threaded 来理解学习。那么如何通过代码的形式来验证这些概念呢？下面我们会通过一个简单的网络请求代码来进行验证。

> 异步编程并不一定意味着多线程。（[async programming does not necessarily mean multi-threaded.](https://stackoverflow.com/questions/8982489/how-does-asynchronous-programming-work-in-a-single-threaded-programming-model)）

&emsp;在 iOS 和 Android 开发中我们直接有 Thread 类来表示一个线程，在 Flutter 中则是没有这样的类的，这对我们理解 Flutter 中的线程概念造成了一些困难，不过这里我们可以借助 Isolate 来近似理解，一个 Isolate 对应一个线程。

## 验证 Future 对象提供的异步操作还在当前线程执行

&emsp;下面我们主要依靠打印 Isolate 对象信息来验证在 Flutter 中 Future 对象提供的异步操作还是在当前线程执行。我们准备了两段功能完全相同的 iOS 和 Flutter 代码，功能都是完成一个简单的网络请求，通过 log 输出可以看出 iOS 在网络请求过程中有子线程出现，而 Flutter 则一直只有主线程。

&emsp;下面是 iOS 下的一个最简单的网络请求示例代码：

```swift
    func getUsers() {
        print("tag 1️⃣ current thread：\(Thread.current)")
        
        let url = URL(string: "https://reqres.in/api/user?page=1")!
        let task = URLSession.shared.dataTask(with: url) { data, response, error in
            let dataString = String(data: data!, encoding: .utf8)
            print("🌹打印请求返回的数据：\(dataString ?? "")");
            
            print("tag 2️⃣ current thread：\(Thread.current)")
            
            DispatchQueue.main.async {
                print("tag 3️⃣ current thread：\(Thread.current)")
                // 在主线程中更新 UI
            }
        }
        
        task.resume()
        
        // 对照输出
        print("tag 4️⃣ current thread：\(Thread.current)")
    }
    
    // 控制台打印
    tag 1️⃣ current thread：<_NSMainThread: 0x600001708040>{number = 1, name = main}
    tag 4️⃣ current thread：<_NSMainThread: 0x600001708040>{number = 1, name = main}
    
    🌹打印请求返回的数据：{"page":1,"per_page":6,"..."}
    
    tag 2️⃣ current thread：<NSThread: 0x6000017002c0>{number = 5, name = (null)}
    tag 3️⃣ current thread：<_NSMainThread: 0x600001708040>{number = 1, name = main}
```

&emsp;我们在主线程中调用 `getUsers` 函数，首先看到 tag 1️⃣ 和 tag 4️⃣ 被执行，且当前都是主线程，tag 4️⃣ 并未等 `URLSession.shared.dataTask` 网络请求完成后才去执行，而是立即得到了执行，这里可以看出 `URLSession.shared.dataTask` 也是非阻塞的，这里它和 Future 的非阻塞机制并不相同的，但是呈现的结果则是相同的。然后重点来了，看 tag 2️⃣ 处的打印，它当前并未在主线程，而是在一个子线程中，这里说明 `URLSession.shared.dataTask` 发起网络请求，iOS 为其准备了一条子线程来进行处理后续事宜。那么在 Flutter 中呢？下面看同 iOS 代码结构完全相同的 Flutter 代码：

```dart
  getUsers() {
    debugPrint(
      "tag 1️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
    );

    const String userUrl = "https://reqres.in/api/user?page=1";
    http.get(Uri.parse(userUrl)).then(
      (response) {
        final result = jsonDecode(response.body);
        debugPrint("🌹打印请求返回的数据：$result");

        debugPrint(
          "tag 2️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
        );
      },
    );

    debugPrint(
      "tag 4️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
    );
  }
  
  // 控制台打印
  flutter: tag 1️⃣ current isolate hashCode: 310700307 current isolate name: main
  flutter: tag 4️⃣ current isolate hashCode: 310700307 current isolate name: main
  
  flutter: 🌹打印请求返回的数据：{page: 1, per_page: 6,...
  
  flutter: tag 2️⃣ current isolate hashCode: 310700307 current isolate name: main
```

&emsp;我们在主 Isolate 中调用 `getUsers` 函数，同样的看到 tag 1️⃣ 和 tag 4️⃣ 首先被执行，且都打印了当前是主 Isolate，即使 `http.get(Uri.parse(userUrl))` 请求完成后回调执行打印 tag 2️⃣ 依然在主 Isolate 中。这里 `http.get(Uri.parse(userUrl))` 函数返回的就是一个 Future 对象。可以看到在网络请求的整个过程中都没有出现子线程，但是这样看好像不明显，下面直接通过创建一个 Future 对象看看：

```dart
  getUsers() {
    debugPrint(
      "tag 1️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
    );

    Future(() {
      debugPrint(
        "tag 2️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
      );

      return 123;
    }).then((value) {
      debugPrint(
        "tag 3️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
      );
    });

    debugPrint(
      "tag 4️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
    );
  }
  
  // 控制台打印
  flutter: tag 1️⃣ current isolate hashCode: 979228963 current isolate name: main
  flutter: tag 4️⃣ current isolate hashCode: 979228963 current isolate name: main
  
  flutter: tag 2️⃣ current isolate hashCode: 979228963 current isolate name: main
  flutter: tag 3️⃣ current isolate hashCode: 979228963 current isolate name: main
``` 

&emsp;我们自己直接创建 Future 对象，看到 tag 4️⃣ 早于 tag 2️⃣、tag 3️⃣ 执行，且四个打印完全都在主 Isolate 中。然后可以再做个实验，直接在 tag 2️⃣ 上面添加一行 `sleep(const Duration(seconds: 5));` 然后执行，可以看到 tag 1️⃣ 和 tag 4️⃣ 执行后 UI 卡顿了 5 秒完全不可交互，5 秒后卡顿才结束，才执行 tag 2️⃣ 和 tag 3️⃣。看到这里既可验证了：Future 异步操作都是在主 Isolate 中执行的，并没有开辟新线程。  

## 对比 Isolate 异步操作(在新线程中异步操作)

&emsp;既然都看到这里了，作为对照组，我们试一下在 Flutter 中创建新 Isolate 开辟新线程的能力：

```dart
  void createIsolate() async {
    debugPrint(
      "tag 1️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
    );

    var result = await compute(doWork, "");

    debugPrint(
      "tag 2️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
    );
  }

  
String doWork(String value) {
  debugPrint(
    "tag 3️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
  );

  // 模拟耗时 5 秒
  sleep(const Duration(seconds: 5));

  debugPrint(
    "tag 4️⃣ current isolate hashCode: ${Isolate.current.hashCode} current isolate name: ${Isolate.current.debugName}",
  );

  return "";
}
 
 // 控制台打印
 flutter: tag 1️⃣ current isolate hashCode: 1051875222 current isolate name: main
 flutter: tag 3️⃣ current isolate hashCode: 473982230 current isolate name: Closure: (String) => String from Function 'doWork': static.
 
 // 延迟了 5 秒后才打印 4️⃣ 和 2️⃣，且这个过程中 APP 一直是正常运行无 UI 阻塞的
 
 flutter: tag 4️⃣ current isolate hashCode: 473982230 current isolate name: Closure: (String) => String from Function 'doWork': static.
 flutter: tag 2️⃣ current isolate hashCode: 1051875222 current isolate name: main 
``` 

&emsp;通过控制台打印可以看出 `compute` 构建了新的 Isolate，开辟了新的线程执行异步操作，且这个 `sleep(const Duration(seconds: 5));` 模拟耗时操作 5 秒是完全不阻塞 UI 线程的，App 正常运行 UI 可交互。不像 Future 中添加耗时操作会直接阻塞主线程导致 UI 不可交互。那么既然都看到这里了，你能分清 Future 和 Isolate 的各自使用场景了吗？

## 参考链接
**参考链接:🔗**
+ [Concurrency and isolates](https://docs.flutter.dev/perf/isolates)
+ [Flutter in Focus](https://www.youtube.com/watch?v=vl_AaCgudcY&list=RDCMUCwXdFgeE9KYzlDdR7TG9cMw&index=19)
+ [Why Dart and Flutter is a single-threaded framework (and it is a great thing)](https://swansoftwaresolutions.com/why-dart-and-flutter-is-a-single-threaded-framework-and-it-is-a-great-thing/)
+ [以生活例子说明单线程与多线程](https://cloud.tencent.com/developer/article/1340815?areaId=106001)
+ [【Flutter 异步编程 - 壹】 | 单线程下的异步模型](https://juejin.cn/post/7144878072641585166)
