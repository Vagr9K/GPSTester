# GPS tester for Android ROMs

This program uses Android's HAL functionality to completely bypass Java Framework layer and directly connect to GPS satelites and then outputs received information into the terminal.

## How to use

* Make sure Android NDK is installed and `ndk-build` is in your path.

* Create `build_config.local` file with your environment configuration. Example`

```shell
export ANDROID_SRCDIR=/mnt/android-workspace
export SYSDIR=/mnt/android-workspace/out/target/product/gtexslte/
export AUTOPUSH=true

```

* Run `build.sh`.
* By default copy gps-test menually to your device or use `AUTOPUSH` setting in `build_config.local` to do that automatically after compilation.


NOTE: Inspired by ["Android GPS using libhardware"](https://gist.github.com/kanru/1568934).