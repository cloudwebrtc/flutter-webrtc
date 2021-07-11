# Flutter-WebRTC

[![Financial Contributors on Open Collective](https://opencollective.com/flutter-webrtc/all/badge.svg?label=financial+contributors)](https://opencollective.com/flutter-webrtc) [![pub package](https://img.shields.io/pub/v/flutter_webrtc.svg)](https://pub.dartlang.org/packages/flutter_webrtc) [![Gitter](https://badges.gitter.im/flutter-webrtc/Lobby.svg)](https://gitter.im/flutter-webrtc/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge) [![slack](https://img.shields.io/badge/join-us%20on%20slack-gray.svg?longCache=true&logo=slack&colorB=brightgreen)](https://join.slack.com/t/flutterwebrtc/shared_invite/zt-q83o7y1s-FExGLWEvtkPKM8ku_F8cEQ)

Flutter平台的WebRTC插件, 支持移动端/桌面/Web网页

</br>
<p align="center">
<strong>Sponsored with 💖 &nbsp by</strong><br />
<a href="https://getstream.io/chat/flutter/tutorial/?utm_source=https://github.com/flutter-webrtc/flutter-webrtc&utm_medium=github&utm_content=developer&utm_term=flutter" target="_blank">
<img src="https://stream-blog-v2.imgix.net/blog/wp-content/uploads/f7401112f41742c4e173c30d4f318cb8/stream_logo_white.png?w=350" alt="Stream Chat" style="margin: 8px" />
</a>
<br />
Enterprise Grade APIs for Feeds & Chat. <a href="https://getstream.io/chat/flutter/tutorial/?utm_source=https://github.com/flutter-webrtc/flutter-webrtc&utm_medium=github&utm_content=developer&utm_term=flutter" target="_blank">Try the Flutter Chat tutorial</a> 💬
</p>

</br>

## 功能及完成度

| Feature | Android | iOS | [Web](https://flutter.dev/web) | macOS | Windows | Linux | [Fuchsia](https://fuchsia.googlesource.com/) |
| :-------------: | :-------------:| :-----: | :-----: | :-----: | :-----: | :-----: | :-----: |
| Audio/Video | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | [WIP] | |
| Data Channel | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | [WIP] | |
| Screen Capture | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | | | | |
| Unified-Plan | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | | | |
| Simulcast | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | | | |
| MediaRecorder| :warning: | :warning: | :heavy_check_mark: | | | | |

## 使用方法

添加 `flutter_webrtc` 到您的 [pubspec.yaml 依赖](https://flutter.io/using-packages/).

### iOS

添加下面的权限标签到 _Info.plist_ 文件, 位于 `<project root>/ios/Runner/Info.plist`:

```xml
<key>NSCameraUsageDescription</key>
<string>$(PRODUCT_NAME) Camera Usage!</string>
<key>NSMicrophoneUsageDescription</key>
<string>$(PRODUCT_NAME) Microphone Usage!</string>
```

用于声明必要的硬件使用权限.

### Android

确认下列权限标签被添加到了您的 AndroidManifest.xml 文件, 位于 `<project root>/android/app/src/main/AndroidManifest.xml`:

```xml
<uses-feature android:name="android.hardware.camera" />
<uses-feature android:name="android.hardware.camera.autofocus" />
<uses-permission android:name="android.permission.CAMERA" />
<uses-permission android:name="android.permission.RECORD_AUDIO" />
<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
<uses-permission android:name="android.permission.CHANGE_NETWORK_STATE" />
<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS" />
```

假设您需要使用蓝牙设备接听WebRTC通话, 您还需要添加下面的标签:

```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
```

在您对Flutter App 生成模板中添加下列行, 或许它已经存在.

您需要设置为 Java 8, 因为Google 官方 WebRTC jar 现在使用静态的 `EglBase` 接口. 请将下面对行添加到您的 `build.gradle`:

```groovy
android {
    //...
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }
}
```

如果必要，请将 `build.gradle` 中  `defaultConfig` 配置下的 `minSdkVersion` 版本设置为 `21` (Flutter 自动生成的为 `16`).

### 重要提醒

如果你在Android平台需要编译成Release版本的apk, 您需要使用此提交中的操作设置,
[Proguard 规则](https://github.com/flutter-webrtc/flutter-webrtc/commit/d32dab13b5a0bed80dd9d0f98990f107b9b514f4)

## 贡献

社区对持久发展离不开这些支持者.

- [CloudWebRTC](https://github.com/cloudwebrtc) - 项目发起者
- [RainwayApp](https://github.com/rainwayapp) - 财务赞助
- [亢少军](https://github.com/kangshaojun) - 财务赞助
- [ION](https://github.com/pion/ion) - 财务赞助
- [reSipWebRTC](https://github.com/reSipWebRTC) - 财务赞助
- [沃德米科技](https://github.com/woodemi)-[36记手写板](https://www.36notes.com) - 财务赞助

### 例子

如果您需要完整的调用例子，请参考 [flutter-webrtc-demo](https://github.com/cloudwebrtc/flutter-webrtc-demo/).

## 贡献者

### 代码贡献者

感谢下列所有人对项目贡献了代码. [[Contribute](CONTRIBUTING.md)].
<a href="https://github.com/cloudwebrtc/flutter-webrtc/graphs/contributors"><img src="https://opencollective.com/flutter-webrtc/contributors.svg?width=890&button=false" /></a>

### 财务贡献者

成为财务贡献者,帮助flutter-webrtc社区持续发展. [[Contribute](https://opencollective.com/flutter-webrtc/contribute)]

#### 来自个人的赞助

<a href="https://opencollective.com/flutter-webrtc"><img src="https://opencollective.com/flutter-webrtc/individuals.svg?width=890"></a>

#### 来自组织的赞助

使用您的项目进行赞助. 您的logo及网站连接将会出现在此处. [[Contribute](https://opencollective.com/flutter-webrtc/contribute)]

<a href="https://opencollective.com/flutter-webrtc/organization/0/website"><img src="https://opencollective.com/flutter-webrtc/organization/0/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/1/website"><img src="https://opencollective.com/flutter-webrtc/organization/1/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/2/website"><img src="https://opencollective.com/flutter-webrtc/organization/2/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/3/website"><img src="https://opencollective.com/flutter-webrtc/organization/3/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/4/website"><img src="https://opencollective.com/flutter-webrtc/organization/4/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/5/website"><img src="https://opencollective.com/flutter-webrtc/organization/5/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/6/website"><img src="https://opencollective.com/flutter-webrtc/organization/6/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/7/website"><img src="https://opencollective.com/flutter-webrtc/organization/7/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/8/website"><img src="https://opencollective.com/flutter-webrtc/organization/8/avatar.svg"></a>
<a href="https://opencollective.com/flutter-webrtc/organization/9/website"><img src="https://opencollective.com/flutter-webrtc/organization/9/avatar.svg"></a>
