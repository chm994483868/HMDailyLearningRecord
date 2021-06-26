#  Bugly 崩溃问题列表

1. FMResultSet 操作不当引起的 crash。 
```c++
5 考呀呀    -[FMStatement reset] (FMDatabase.m:1667)
6 考呀呀    -[FMResultSet close] (FMResultSet.m:59)
7 考呀呀    -[FMResultSet dealloc] (FMResultSet.m:46)
```
2. 多线程写数据库导致的 crash，`DBSingle getVideoLookInfoFromVideoId:UserId:` 函数曾多次引起。
```c++
8 考呀呀    -[FMDatabase executeQuery:withArgumentsInArray:orDictionary:orVAList:] (FMDatabase.m:835)
9 考呀呀    -[FMDatabase executeQuery:] (FMDatabase.m:0)
10 考呀呀    -[DBSingle getVideoLookInfoFromVideoId:UserId:] (DBSingle.m:734)
11 考呀呀    -[LearnTimeManger getVideoLearnTimeInfo:resolver:rejecter:] (LearnTimeManger.m:30)
```
3. 多线程写数据库导致的 crash，`DBSingle updateDownloadInfo:params:` 引起。
```c++
8 考呀呀    -[FMDatabase executeUpdate:error:withArgumentsInArray:orDictionary:orVAList:] (FMDatabase.m:1020)
9 考呀呀    -[FMDatabase executeUpdate:withArgumentsInArray:] (FMDatabase.m:1225)
10 考呀呀    -[DBSingle updateDownloadInfo:params:] (DBSingle.m:1170)
11 考呀呀    __41-[KYYM3U8Manger updatePlayTimeToLocalDB:]_block_invoke (KYYM3U8Manger.m:77)
```
4. 数组插入时越界导致 crash，`ZYLM3U8Handler praseUrl:uuid:` 引起。
```c++
7 考呀呀    -[NSMutableArray(swizzling) swInsertObject:atIndex:] (NSMutableArray+swizzling.m:0)
8 考呀呀    -[NSMutableArray(swizzling) swAddObject:] (NSMutableArray+swizzling.m:59)
9 考呀呀    -[ZYLM3U8Handler praseUrl:uuid:] (ZYLM3U8Handler.m:72)
10 考呀呀    -[DecodeTool handleM3U8Url:] (DecodeTool.m:64)
11 考呀呀    -[ZBM3U8Downloader start] (ZBM3U8Downloader.m:35)
12 考呀呀    -[ZBM3U8DownloaderManager startDownloadItem:] (ZBM3U8DownloaderManager.m:0)
13 考呀呀    __39-[ZBDownloadManager startDownloadItem:]_block_invoke (ZBDownloadManager.m:181)
```
5. 可能是 RN 内部引发 `com.facebook.react.JavaScript`。
```c++
10 考呀呀    facebook::jsc::JSCRuntime::call(facebook::jsi::Function const&, facebook::jsi::Value const&, facebook::jsi::Value const*, unsigned long) (JSCRuntime.cpp:0)
11 考呀呀    facebook::jsi::Value facebook::jsi::Function::call<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, facebook::jsi::Value>(facebook::jsi::Runtime&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&&&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&&&, facebook::jsi::Value&&) const (jsi-inl.h:224)
12 考呀呀    _ZNSt3__110__function6__funcIZN8facebook5react11JSIExecutor12callFunctionERKNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEESC_RKN5folly7dynamicEE3$_4NS8_ISH_EEFvvEEclEv (JSIExecutor.cpp:231)
```
6. 数据库引发 crash，`DBSingle updateVideoStatus:params:` 引发。
```c++
8 考呀呀    -[FMDatabase executeUpdate:error:withArgumentsInArray:orDictionary:orVAList:] (FMDatabase.m:1020)
9 考呀呀    -[FMDatabase executeUpdate:withArgumentsInArray:] (FMDatabase.m:1225)
10 考呀呀    -[DBSingle updateVideoStatus:params:] (DBSingle.m:999)
11 考呀呀    -[ZYLM3U8Handler praseUrl:uuid:] (ZYLM3U8Handler.m:79)
12 考呀呀    -[DecodeTool handleM3U8Url:] (DecodeTool.m:64)
13 考呀呀    -[ZBM3U8Downloader start] (ZBM3U8Downloader.m:35)
14 考呀呀    -[ZBM3U8DownloaderManager startDownloadItem:] (ZBM3U8DownloaderManager.m:0)
15 考呀呀    __39-[ZBDownloadManager startDownloadItem:]_block_invoke (ZBDownloadManager.m:181)
```
7. UAAPIManager 内 NSDecimalNumber  引发的 crash。
```c++
3 考呀呀    -[UAAPIManager handleTimings] + 384
4 考呀呀    ___48-[UAAPIManager requestWithParameter:completion:]_block_invoke + 296
5 考呀呀    -[UAAPIManager redirectToNotifyWithRequest:result:wapData:operatorType:completion:] + 1384
```
8. RCTFatalException: Unhandled JS Exception: Error: AnimatedValue: Attempting to set value to undefined。
```c++
8 考呀呀    -[RCTModuleMethod invokeWithBridge:module:arguments:] (RCTModuleMethod.mm:587)
9 考呀呀    invokeInner (RCTNativeModule.mm:0)
10 考呀呀    __ZN8facebook5react15RCTNativeModule6invokeEjON5folly7dynamicEi_block_invoke (Optional.h:404)
```
9. `DBSingle getVideoLookInfoFromVideoId:UserId:` 引发数据库 crash。
```c++
8 考呀呀    -[FMDatabase executeQuery:withArgumentsInArray:orDictionary:orVAList:] (FMDatabase.m:835)
9 考呀呀    -[FMDatabase executeQuery:] (FMDatabase.m:0)
10 考呀呀    -[DBSingle getVideoLookInfoFromVideoId:UserId:] (DBSingle.m:734)
11 考呀呀    -[LearnTimeManger getVideoLearnTimeInfo:resolver:rejecter:] (LearnTimeManger.m:30)
```
10. `DBSingle updateVideoStatus:params:` 引发。
```c++
6 考呀呀    -[FMDatabase executeUpdate:error:withArgumentsInArray:orDictionary:orVAList:] (FMDatabase.m:1020)
7 考呀呀    -[FMDatabase executeUpdate:withArgumentsInArray:] (FMDatabase.m:1225)
8 考呀呀    -[DBSingle updateVideoStatus:params:] (DBSingle.m:999)
9 考呀呀    -[ZYLM3U8Handler praseUrl:uuid:] (ZYLM3U8Handler.m:79)
10 考呀呀    -[DecodeTool handleM3U8Url:] (DecodeTool.m:64)
11 考呀呀    -[ZBM3U8Downloader start] (ZBM3U8Downloader.m:35)
12 考呀呀    -[ZBM3U8DownloaderManager startDownloadItem:] (ZBM3U8DownloaderManager.m:0)
13 考呀呀    __39-[ZBDownloadManager startDownloadItem:]_block_invoke (ZBDownloadManager.m:181)
```
11. 参数无效。
```c++
-[__NSCFNumber _accessibilityViewIsVisible]: unrecognized selector sent to instance 0x9c8ecf7b77db579a
CoreFoundation ___exceptionPreprocess
```
12.  -[CCPlayerController interfaceOrientation:] (CCPlayerController.m:) 直播控制器切换横竖屏导致的问题。
```c++
考呀呀    -[CCPlayerController interfaceOrientation:] (CCPlayerController.m:1344)
```
13. CCSDK 内部引发。
```c++
1 UIKitCore    ___85-[UIKeyboardEmojiFamilyConfigurationView _setCurrentlySelectedSkinToneConfiguration:]_block_invoke_2
2 CCSDK    __40-[DrawBitmapView loadImageWithImageURL:]_block_invoke_2 + 68
```
14. `DBSingle updateVideoLookInfoRecord:` 引发。
```c++
4 考呀呀    -[FMDatabase executeUpdate:error:withArgumentsInArray:orDictionary:orVAList:] (FMDatabase.m:1131)
5 考呀呀    -[FMDatabase executeUpdate:] (FMDatabase.m:1222)
6 考呀呀    -[FMDatabase beginTransaction] (FMDatabase.m:1346)
7 考呀呀    -[DBSingle updateVideoLookInfoRecord:] (DBSingle.m:754)
8 考呀呀    -[LearnTimeManger updateVideoLearnTimeInfo:] (LearnTimeManger.m:38)
```
15. WPKMobiWALogManager 引发。
```c++
2 考呀呀    -[WPKMobiWALogManager uploadWAData] (WPKWALogManager.m:247)
3 考呀呀    -[WPKMobiWALogManager start] (WPKWALogManager.m:310)
4 考呀呀    __43-[WPKCrashInstallation asyncSendAllReports]_block_invoke_3 (WPKCrashInstallation.m:176)
```
16. `-[__NSDictionaryM setObject:forKey:]: key cannot be nil` ZBLiveDownloadMoreVC 引发。
```c++
5 考呀呀    -[NSMutableDictionary(swizzling) swSetObject:forKey:] (NSMutableDictionary+swizzling.m:24)
6 考呀呀    __35-[ZBLiveDownloadMoreVC viewDidLoad]_block_invoke (ZBLiveDownloadMoreVC.m:109)
7 考呀呀    __26-[ZBDownloadManager fail:]_block_invoke (ZBDownloadManager.m:474)
```
