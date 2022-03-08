# Combine 学习笔记  

## Introducing Combine

> Combine is a unified declarative framework for processing values over time. Learn how it can simplify asynchronous code like networking, key value observing, notifications and callbacks.
> Combine 是一个统一的声明性框架，用于处理随时间变化的值。了解它如何简化异步代码，如网络、键值观察、通知和回调。




&emsp;Asynchronous Interfaces

+ Target/Action
+ Notification center
+ URLSession
+ Key-value observing
+ Ad-hoc callbacks

&emsp;Key Concepts

+ Publishers
+ Subscribers
+ Operators

&emsp;Publisher

+ Defines how values and errors are produced
+ Value type
+ Allows registration of a Subscriber

```c++
    protocol Publisher {
        associatedtype Output
        associatedtype Failure: Error
        func subscribe<S: Subscriber>(_ subscriber: S)
        where S.Input == Output, S.Failure == Failure
    }
```

&emsp;Publisher NotificationCenter

```c++
    extension NotificationCenter {
        struct Publisher: Combine.Publisher {
            typealias Output = Notification
            typealias Failure = Never
            init(center: NotificationCenter, name: Notification.Name, object: Any? = nil)
        }
    }
```

&emsp;Subscriber

+ Receives values and a completion
+ Reference type

```c++
    protocol Subscriber {
        associatedtype Input
        associatedtype Failure: Error
        func receive(subscription: Subscription)
        func receive(_ input: Input) -> Subscribers.Demand
        func receive(completion: Subscribers.Completion<Failure>)
    }
```

&emsp;Subscriber Assign

```c++
    extension Subscribers {
        class Assign<Root, Input>: Subscriber, Cancellable {
            typealias Failure = Never
            init(object: Root, keyPath: ReferenceWritableKeyPath<Root, Input>)
        }
    }
```

&emsp;Operator

+ Adopts `Publisher`
+ Describes a behavior for changing values
+ Subscribes to a `Publisher` (“upstream”)
+ Sends result to a `Subscriber` (“downstream”)
+ Value type

&emsp;Operator Map

```c++
    extension Publishers {
        struct Map<Upstream: Publisher, Output>: Publisher {
            typealias Failure = Upstream.Failure
            
            let upstream: Upstream
            let transform: (Upstream.Output) -> Output
        }
    }
```

```c++
    // Operator Construction
    extension Publisher {
        func map<T>(_ transform: @escaping (Output) -> T) -> Publishers.Map<Self, T> {
            return Publishers.Map(upstream: self, transform: transform)
        }
    }
```

&emsp;Declarative Operator API

+ Functional transformations
+ List operations
+ Error handling
+ Thread or queue movement
+ Scheduling and time

```c++
        let cancellable = NotificationCenter.default.publisher(for: graduated, object: merlin)
            .map { note in
                return note.userInfo?["NewGrade"] as? Int ?? 0
            }
            .filter { $0 >= 5 }
            .prefix(3)
            .assign(to: \.grade, on: merlin)
```

&emsp;Combining Publishers

+ Zip
+ CombineLatest

&emsp;Zip

+ Converts several inputs into a single tuple
+ A “when/and” operation
+ Requires input from all to proceed

```c++
Zip3(organizing, decomposing, arranging)
.map { $0 && $1 && $2 }
.assign(to: \.isEnabled, on: continueButton)

// organizing、decomposing、arranging 全为真时，continueButton 按钮置为可点击
```

&emsp;Combine Latest

+ Converts several inputs into a single value
+ A “when/or” operation
+ Requires input from any to proceed
+ Stores last value

```c++
CombineLatest3(read, practiced, approved) {
    $0 && $1 && $2
}
.assign(to: \.isEnabled, on: playButton)
```


















```c++
/// Declares that a type can transmit a sequence of values over time.
///
/// A publisher delivers elements to one or more ``Subscriber`` instances.
/// The subscriber’s ``Subscriber/Input`` and ``Subscriber/Failure`` associated types must match the ``Publisher/Output`` and ``Publisher/Failure`` types declared by the publisher.
/// The publisher implements the ``Publisher/receive(subscriber:)``method to accept a subscriber.
///
/// After this, the publisher can call the following methods on the subscriber:
/// - ``Subscriber/receive(subscription:)``: Acknowledges the subscribe request and returns a ``Subscription`` instance. The subscriber uses the subscription to demand elements from the publisher and can use it to cancel publishing.
/// - ``Subscriber/receive(_:)``: Delivers one element from the publisher to the subscriber.
/// - ``Subscriber/receive(completion:)``: Informs the subscriber that publishing has ended, either normally or with an error.
///
/// Every ``Publisher`` must adhere to this contract for downstream subscribers to function correctly.
///
/// Extensions on ``Publisher`` define a wide variety of _operators_ that you compose to create sophisticated event-processing chains.
/// Each operator returns a type that implements the ``Publisher`` protocol
/// Most of these types exist as extensions on the ``Publishers`` enumeration.
/// For example, the ``Publisher/map(_:)-99evh`` operator returns an instance of ``Publishers/Map``.
///
/// # Creating Your Own Publishers
///
/// Rather than implementing the ``Publisher`` protocol yourself, you can create your own publisher by using one of several types provided by the Combine framework:
///
/// - Use a concrete subclass of ``Subject``, such as ``PassthroughSubject``, to publish values on-demand by calling its ``Subject/send(_:)`` method.
/// - Use a ``CurrentValueSubject`` to publish whenever you update the subject’s underlying value.
/// - Add the `@Published` annotation to a property of one of your own types. In doing so, the property gains a publisher that emits an event whenever the property’s value changes. See the ``Published`` type for an example of this approach.
@available(macOS 10.15, iOS 13.0, tvOS 13.0, watchOS 6.0, *)
public protocol Publisher {

    /// The kind of values published by this publisher.
    associatedtype Output

    /// The kind of errors this publisher might publish.
    ///
    /// Use `Never` if this `Publisher` does not publish errors.
    associatedtype Failure : Error

    /// Attaches the specified subscriber to this publisher.
    ///
    /// Implementations of ``Publisher`` must implement this method.
    ///
    /// The provided implementation of ``Publisher/subscribe(_:)-4u8kn``calls this method.
    ///
    /// - Parameter subscriber: The subscriber to attach to this ``Publisher``, after which it can receive values.
    func receive<S>(subscriber: S) where S : Subscriber, Self.Failure == S.Failure, Self.Output == S.Input
}
```

```c++
/// A protocol that declares a type that can receive input from a publisher.
///
/// A ``Subscriber`` instance receives a stream of elements from a ``Publisher``, along with life cycle events describing changes to their relationship. A given subscriber’s ``Subscriber/Input`` and ``Subscriber/Failure`` associated types must match the ``Publisher/Output`` and ``Publisher/Failure`` of its corresponding publisher.
///
/// You connect a subscriber to a publisher by calling the publisher’s ``Publisher/subscribe(_:)-4u8kn`` method.  After making this call, the publisher invokes the subscriber’s ``Subscriber/receive(subscription:)`` method. This gives the subscriber a ``Subscription`` instance, which it uses to demand elements from the publisher, and to optionally cancel the subscription. After the subscriber makes an initial demand, the publisher calls ``Subscriber/receive(_:)``, possibly asynchronously, to deliver newly-published elements. If the publisher stops publishing, it calls ``Subscriber/receive(completion:)``, using a parameter of type ``Subscribers/Completion`` to indicate whether publishing completes normally or with an error.
///
/// Combine provides the following subscribers as operators on the ``Publisher`` type:
///
/// - ``Publisher/sink(receiveCompletion:receiveValue:)`` executes arbitrary closures when it receives a completion signal and each time it receives a new element.
/// - ``Publisher/assign(to:on:)`` writes each newly-received value to a property identified by a key path on a given instance.
@available(macOS 10.15, iOS 13.0, tvOS 13.0, watchOS 6.0, *)
public protocol Subscriber : CustomCombineIdentifierConvertible {

    /// The kind of values this subscriber receives.
    associatedtype Input

    /// The kind of errors this subscriber might receive.
    ///
    /// Use `Never` if this `Subscriber` cannot receive errors.
    associatedtype Failure : Error

    /// Tells the subscriber that it has successfully subscribed to the publisher and may request items.
    ///
    /// Use the received ``Subscription`` to request items from the publisher.
    /// - Parameter subscription: A subscription that represents the connection between publisher and subscriber.
    func receive(subscription: Subscription)

    /// Tells the subscriber that the publisher has produced an element.
    ///
    /// - Parameter input: The published element.
    /// - Returns: A `Subscribers.Demand` instance indicating how many more elements the subscriber expects to receive.
    func receive(_ input: Self.Input) -> Subscribers.Demand

    /// Tells the subscriber that the publisher has completed publishing, either normally or with an error.
    ///
    /// - Parameter completion: A ``Subscribers/Completion`` case indicating whether publishing completed normally or with an error.
    func receive(completion: Subscribers.Completion<Self.Failure>)
}

```

## 参考链接
**参考链接:🔗**
+ [Introducing Combine](https://developer.apple.com/videos/play/wwdc2019/722/)
+ [Combine in Practice](https://developer.apple.com/videos/play/wwdc2019/721)
+ [Data Flow Through SwiftUI](https://developer.apple.com/videos/play/wwdc2019/226)
+ [Receiving and Handling Events with Combine](https://developer.apple.com/documentation/combine/receiving-and-handling-events-with-combine)
+ []()



+ [SwiftUI中使用Combine](https://juejin.cn/column/7016364164541054989)
+ [Apple 官方异步编程框架：Swift Combine 应用](https://nemocdz.github.io/post/apple-官方异步编程框架swift-combine-应用/)

