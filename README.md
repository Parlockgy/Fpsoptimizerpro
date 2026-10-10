# FPS Optimizer Pro Mod (by taumk)

A `.geode` file is NOT just a renamed zip of this folder. It must contain a *compiled* binary for your
platform (e.g. `taumk.fps-optimizer-pro.android64.so`). Renaming this zip to `.geode` produces the error
"Unable to find platform binary". You have to build it.

## Easiest: build on GitHub (no installs)
1. Create a free GitHub repo and upload everything in this folder (make sure the hidden `.github` folder is included).
2. Open the repo's **Actions** tab -> **Build Geode Mod** -> **Run workflow**.
3. When it finishes, open the run and download the **Build Output** artifact.
4. Unzip it: inside is `taumk.fps-optimizer-pro.geode` with Windows, macOS, Android32 and Android64 binaries.
5. Put that file in your Geode `mods` folder (or import it from the Geode mods menu on Android).

## Build locally
1. Install the Geode CLI + SDK: https://docs.geode-sdk.org/getting-started/
2. `geode sdk install` (or `geode sdk update`)
3. In this folder: `geode build`  (Android needs the Android NDK: `geode build -p android64`)
