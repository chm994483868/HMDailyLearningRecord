# KYY 知识点记录



// jsCodeLocation = [NSURL URLWithString:@"http://192.168.5.128:8081/index.bundle?platform=ios"];

1. 添加苹果内购可购买商品的过滤机制。
2. 修改充币页面头像昵称丢失、默认选中一个充值项。

## RN 与 Native 交互
1. RN 调用 OC
&emsp;遵循 RCTBridgeModule 协议，`RCT_EXPORT_METHOD(methodName:(NSInteger)param1 count:(NSInteger)param2 resolver:(RCTPromiseResolveBlock)resolver rejecter:(RCTPromiseRejectBlock)reject) { ... }` 形式调用。

2. OC 调用 RN
&emsp;继承 RCTEventEmitter，然后 `- (NSArray<NSString *> *)supportedEvents{ ... }` 返回可用的方法明，调用时则是 `- (void)sendEventWithName:(NSString *)eventName body:(id)body` 函数，`eventName` 是要调用的 RN 函数名，`body` 是参数。

3. RN 使用原生 UI，RCTViewManager 和 RCT_EXPORT_VIEW_PROPERTY 的使用
&emsp;这个链接下 RCTViewManager 和 RCT_EXPORT_VIEW_PROPERTY 已经介绍的足够清晰：[React Native之创建iOS视图](https://blog.csdn.net/u014410695/article/details/51133727)。继承 RCTViewManager，然后实现 `- (UIView *)view` 函数，返回自己的自定义 View，然后 RCT_EXPORT_VIEW_PROPERTY(title, NSString *); 这样释放出 RN 可以直接使用的我们的自定义 View 的某些属性。

### 分析 AppDelegate 文件
1. AppDelegate.h 文件中添加了一个 AFNetworkReachabilityStatus 类型的 status 属性用于监听手机网络状态，这样写没必要，只需要在启动时开启 AFN 的网络监听即可，当网络变化时做出提示。
#### 分析 application:didFinishLaunchingWithOptions: 函数
1. initNotify 函数有对本地 plist 文件的操作，可以放在后台线程中执行，加快 App 启动速度。（initNotify 中的内容甚至放在第一个页面显示完毕后再执行都不晚，完全没必要放在 application... 函数中）
2. AVAudioSession 设置支持后台音频播放可以放在后面。
3. 添加了 canLandscape 通知，监听是否支持横屏。
4. bugly 的注册可以根据 DEBUG 的情况来进行注册和非注册。
5. ShanYanConfig SDK 的注册可以提取出来，在涉及到登录之前进行注册。
6. 注册友盟 SDK 使用的代码行数较多，可以抽取出来。
7. jsCodeLocation 变量表示 DEBUG （真机和模拟器）和 RELEASE 模式下不同的运行环境。
8. AFN 的一大段的监测网络的代码可以抽出来。监测网络的中的弹框使用的 UIAlertView，它在 iOS 9.0 以后已经废弃了，可以替换为 UIAlertViewController。
9. application:didRegisterForRemoteNotificationsWithDeviceToken: 中注册远程推送，且收到远程通知的回调后，只处理 IM 的聊天部分，那那些推广活动的推送是怎么处理的？
10. applicationDidBecomeActive: 中 App 启动后直接把角标置空，会不会太快了。
11. 本地数据库的处理。
12. application 中前后台切换时 IM 的处理。

### 引入 react-native-config
1. 在项目根目录下执行 yarn add react-native-config。
2. 在 ios 文件夹下执行 pod install。
3. 打开 macOS 的隐藏文件显示，然后在项目根目录下创建一个 .env 文件，在里面添加配置变量，如 URL、APPID。
4. #import "ReactNativeConfig.h" 后 NSString *testServer = [ReactNativeConfig envFor:@"ISTESTSERVER"]; 即可获取 .env 文件中指定 key 的 value 值。

### 梳理内购模块
1. application:didFinishLaunchingWithOptions: 中的调用 [PurchaseViewController sharePurchase]; 进行 PurchaseViewController 单例类实现和添加通知以及直接从 SK 获取所有可卖的商品。
2. KYYAppSet.m 文件中的 RCT_EXPORT_METHOD(goToPurchaseVC:(NSString *)avatar username:(NSString *)username) 函数，RN 调用 Native 跳转到原生的考币充值控制器 PurchaseViewController。（PurchaseViewController 直接被定为了一个单例类）
3. PurchaseViewController 的 viewWillAppear 函数中，从本地获取 Token 后作为参数去获取用户头像和当前的考币。
4. PurchaseView 作为 PurchaseViewController 的 View 把每一个购买项列出。（循环创建 UIButton）
5. sharePurchase 函数内 dispatch_once 里面 loadProducts: 函数获取可卖的商品，这种写法，没有任何重试机制，且是在 APP 刚启动时获取的，如果此次获取失败的话，只能从任务管理器划掉 APP 后，重新启动 APP 了。
6. SKProductsRequest start 发起请求，从 Apple 请求用户可以进行购买的产品信息。- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response 代理函数返回请求成功的的所有的商品信息。
7. [[BLPaymentManager sharedManager] buyProduct:nowProduct error:nil]; 函数发起支付。内部是以 SKProduct* product 为参数构建一个 SKPayment *payment = [SKPayment paymentWithProduct:product]; 然后 [[SKPaymentQueue defaultQueue] addPayment:payment]; 把 payment 添加到 SKPaymentQueue 队列中去。
8. 然后就是 SKPaymentTransactionObserver 代理函数执行。当事务队列发生更改时发送（添加或状态更改）。 客户端应检查交易状态并酌情完成。
```c++
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray<SKPaymentTransaction *> *)transactions;
```
&emsp;当收到 SKPaymentTransactionStatePurchased 时去本地沙盒获取交易凭据去带着数据去 @"orders/payByAPPStoreOrder" 验证交易是否成功。服务器验证成功后，删除本地 NSUserDefaults .plist 中保存的凭证。（交易凭据保存在 NSUserDefaults 中不是一个稳妥的方案，应当保存在钥匙串中。）
9. PurchaseView 页面可以抽成一个 CollectionView。

### 梳理直播模块
1. 直播入口，在 LiveShow.m 文件中的 RCT_EXPORT_METHOD(LiveCCShow:(NSDictionary *)data resolver:(RCTPromiseResolveBlock)resolver rejecter:(RCTPromiseRejectBlock)reject) 函数，这里会根据 RN 传来的 data 区分是进行直播还是回放。直播的控制器：CCPlayerController 和 回放的控制器：CCPlayBackController。
2. 直播控制器 CCPlayerController：viewDidLoad 函数：根据视频 ID 判断是否需要在本地保存视频观看信息记录。根据本地标识判断是否 TouchGestureTipView 显示。
3. 
