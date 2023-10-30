# Hathora Unreal Engine 5 Cloud SDK (and Demo)

This directory contains the source code for the Hathora SDK plugin for Unreal Engine 5. Currently the only functionality implemented here is client
ping calculation.

## SDK Plugin Usage

The SDK currently only supports retrieving and pinging each region. The easiest method is to call the [Get Regional Pings](#get-regional-pings) function, but if you would prefer to send the ICMP pings manually, you can optionally only call the [GetPingServiceEndpoints](#manually-calling-getpingserviceendpoints) API call.

### Get Regional Pings

This function takes an optional second argument `int32 NumPingsPerRegion` which defaults to `3`. The SDK will execute the following when calling this function:
1. Call the `GetPingServiceEndpoints` API call
2. Send an ICMP echo for each region once in parallel
3. After receiving a result for **each** region, step 2 is repeated `NumPingsPerRegion - 1` times. The SDK waits until a result is returned from each region before sending another set of pings to prevent some of the ICMP echos from being ignored/dropped due to subsequent/overlap requests.
4. After running step 2 a total of `NumPingsPerRegion` times, the minimum is returned for each region. If a particular region timed out or was unreachable otherwise for each ping, it is omitted from the callback delegate.

The ICMP pings have a default timeout of `1.0 seconds`; you can change this in the `Edit > Project Settings... > Plugins submenu > Hathora SDK` menu (or just search for `ping timeout`):

![image](https://github.com/hathora/hathora-unreal-sdk/assets/549323/a40905b3-6058-4290-9c8b-7f9d04b67a01)

If you want to save the change for your entire project/team, make sure to click the **Set as Default** button at the top; this will save the setting in `Config/DefaultGame.ini` which you can add to your version control. If you don't do this, the setting will only apply to your instance of the Unreal project.

#### C++

To call `GetRegionalPings` with C++, you should use the `UHathoraSDK::GetRegionalPings(...)` static function. You can find an example of how to use this in [DemoMenuWidget.cpp](./SDKDemo/Source/SDKDemo/DemoMenuWidget.cpp#L93-L95).

#### Blueprint

To call `GetRegionalPings` with BP, you should use the `Get Regional Pings` BP node found under the `Hathora SDK` category:

![image](https://github.com/hathora/hathora-unreal-sdk/assets/549323/d5fa78ca-fb38-4fc2-9ab1-716f51aa6ebe)

Here is an example of using that node:

![image](https://github.com/hathora/hathora-unreal-sdk/assets/549323/e27ec908-d5a1-400d-89fc-ab0282387be8)

### Manually calling GetPingServiceEndpoints

In some scenarios, it may be desired to have more control on how/when the pings are executed. In this case, you can call the `GetPingServiceEndpoints` API endpoint directly with the SDK and handle the result however you wish. To do this, you need to make an instance of the `UHathoraSDK` and call the `DiscoveryV1->GetPingServiceEndpoints` function.

#### C++

To create an instance of `UHathoraSDK`, call the static `UHathoraSDK::CreateHathoraSDK` function. There's no need to provide an AppID or the Hathora Dev Token, so those are left blank in the call (but can be provided if you need them to access other APIs):

``` c++
UHathoraSDK* SDK = UHathoraSDK::CreateHathoraSDK("", FHathoraSDKSecurity());
```

From there, you can get access to the `GetPingServiceEndpoints` function via the `DiscoveryV1` property:

``` c++
FHathoraOnGetPingServiceEndpoints OnGetEndpointsComplete;
OnGetEndpointsComplete.BindDynamic(this, &UYourClass::YourCallback);
SDK->DiscoveryV1->GetPingServiceEndpoints(OnGetEndpointsComplete);
```

The `UYourClass::YourCallback` function should have the signature `void (const TArray<FHathoraDiscoveredPingEndpoint>& PingEndpoints)`.

You can see how to execute the ICMP echos yourself in [HathoraSDKDiscoveryV1.cpp](./SDKDemo/Plugins/HathoraSDK/Source/HathoraSDK/Private/HathoraSDKDiscoveryV1.cpp#L94-L97).

#### Blueprint

To create an instance of `UHathoraSDK`, call the `Create Hathora SDK` BP node. There's no need to provide an AppID or the Hathora Dev Token, so those are left blank in the call (but can be provided if you need them to access other APIs). From there, you can find the `Get Ping Service Endpoints` function on the `DiscoveryV1` variable:

![image](https://github.com/hathora/hathora-unreal-sdk/assets/549323/2558043c-7814-4264-a949-a4fd2dd37fbb)

## Supported Cloud API Endpoints

- DiscoveryV1
  - GetPingServiceEndpoints (helper function `Get Regional Pings` will call this and execute multiple pings for you, providing the minimum)

## Development Setup

### macOS

#### Install stuff

1. Install Xcode. You will likely need Xcode 14, which only runs on macOS 13 (Ventura). You can find the current release at the [Apple Developer site](https://developer.apple.com/download/all).
1. After installing Xcode, open it once to accept the license agreements and install the macOS SDKs. After this you can close it.
1. Install the [Epic Games Launcher](https://store.epicgames.com/en-US/download). After installation open the Launcher and login with an Epic Games account.
1. Install Unreal Engine 5.2. [This link](com.epicgames.launcher://ue) should open the Epic Games Launcher to the correct screen once you've logged in.
1. _Optional Validation Step_ After installation, attempt to open the Unreal Editor (in your applications folder). If it fails to open, things are not working correctly. Next, create a "blank" C++ game project and stick it wherever on your hard drive. When you click "Create," you should eventually have Xcode and the editor open up. If you receive a message that "your project did not compile," double check that Xcode is installed, you have the right version, and that it is located in your Applications folder (e.g. that you didn't leave it in Downloads or something).

### Windows 10

1. Install the Epic Games Launcher and Unreal Engine 5.2. The installer for Unreal Engine should ensure you have all of the required tools.

### Open project / Run demo
Open the project in your IDE of choice. Xcode (macOS), Visual Studio (Windows), and JetBrains Rider (cross-platform) will work out of the box. You
likely can get decent VSCode setup by following the instructions in [this documentation page](https://docs.unrealengine.com/5.2/en-US/setting-up-visual-studio-code-for-unreal-engine/).

Your IDE likely has a button for "Run." Click that. The project should open in the Unreal Editor. Once open in the editor, click the green "Play" button to start the (very unstyled/ugly) demo. Click the "GET PINGS" button and the pings will be logged to the console. This is the demo.

### Editor Tips

If you're using Rider, here are some tips:
- You can install the RiderLink plugin when prompted, but you don't have to. It has a bunch of features that won't really help with this project.
- Turn off the tree output view for build errors. Unfortunately this view elides most problems and after a build fails you'll only get a split second before
the error messages disappear. In Preferences, go to `Tool Window: switch to tree view after the build if build events were reported` on the `Build, Execution, Deployment | Toolset and Build | Presentation` and uncheck the box. This is the difference between getting "linker exited with exit code 1" as your only error message versus the actual output of `ld`.

## Code Formatting and Style

See [the Unreal Engine 5 coding standard](https://docs.unrealengine.com/5.2/en-US/epic-cplusplus-coding-standard-for-unreal-engine/) for style guidelines.

There is a `clang-format` file included here for the formatting parts of the UE5 code style.
You can install `clang-format` from Homebrew and run it over all files. This is a command that should work with BSD find (the kind installed on Macs):

```bash
find . -name '*.cpp' -o -name '*.h' -exec clang-format -i {} +
```

You might also be able to find a plugin for your editor that uses `clang-format`.

