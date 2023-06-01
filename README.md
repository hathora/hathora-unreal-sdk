# Hathora Unreal Engine 5 Ping SDK (and Demo)

This directory contains the source code for the Hathora SDK for Unreal Engine 5. Currently the only functionality implemented here is client
ping calculation.

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

