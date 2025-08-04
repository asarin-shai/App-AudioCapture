# Overview

The AudioCapture application uses Qt's [QAudioSource](https://doc.qt.io/qt-6/qaudiosource.html) for cross-platform audio capturing and streaming over [LSL](https://labstreaminglayer.org).

# Getting Started

Download the latest version [from the releases page](https://github.com/labstreaminglayer/App-AudioCapture/releases).

The Windows release requires vc_redist.x64.exe; if you don't already have it then you can install the download [from Microsoft](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads).

Using this app is very simple:

  * Make sure that you have connected a microphone to your computer.
  * Start the AudioCapture app. You should see a window like the following.
> ![audiocapture.PNG](audiocapture.PNG)
  * Set the audio capture parameters.
    * Make sure the correct audio device is selected.
    * The preferred "Sample Size" should be 16. In our testing, 8- and 32-bits per sample did not produce useful data.
    * The number of output channels can be one or two depending on the connected microphone.
  * Click the "Link" button to link the app to the lab network. If successful, the button should turn into "Unlink".
  * If a firewall complains, allow the app to connect to the network.
  * Please allow microphone access if asked.
  * You should now have a stream on your lab network that has type "Audio" and its name is the name entered in the GUI.
  * Note that you cannot close the app while it is linked.

# Build from source

## Pre-requisites

### liblsl

TODO

### Qt6 >= 6.5

* MacOS: `brew install qt`
* Windows or Linux: Download and run installer
  * You will need a (free) Qt account
  * This is an open source project so you can use the LGPL Qt6 open source version
  * QtMultimedia should be enabled by default so default options are fine

### Build Essentials

* CMake >= 3.25
* Compiler

## Instructions

** Configure: **
  * MacOS: `cmake -B build -S . -DCMAKE_INSTALL_PREFIX=build/install -DCMAKE_BUILD_TYPE=Release -G Xcode`
  * Linux: `cmake -B build -S . -DCMAKE_INSTALL_PREFIX=build/install -DCMAKE_BUILD_TYPE=Release -DQt6_DIR=~/Qt/6.9.1/gcc_64/lib/cmake/Qt6`
  * Windows: `cmake -B build -S . -DCMAKE_INSTALL_PREFIX=build/install -DCMAKE_BUILD_TYPE=Release -DQt6_DIR=C:\\Qt\\6.9.1\\mingw_64\\lib\\cmake\\Qt6`

** Build: **
  * `cmake --build build -DCMAKE_BUILD_TYPE=Release --target install`

Note on MacOS: If the `APPLE_CODE_SIGN_IDENTITY_APP` env variable is set then the package will be code-signed at this stage.

** Package: **

TODO

** Notarization (MacOS only; Optional): **

TODO

** Deploy and Use: **

Without the packaging step in place, the build/install/ folder will container  

# Further Notes

The previous version of AudioCaptureWin can be found as [release v0.1](https://github.com/labstreaminglayer/App-AudioCapture/releases/tag/v0.1) in this repository.

For Windows XP there is an older LSL audio recording app available on request; it uses the [irrKlang](http://www.ambiera.com/irrklang/) audio library, which in turn uses DirectX audio on Windows. That application does not support accurate time synchronization and is therefore deprecated.

# Developer Notes

There were quite a few changes with Qt6. We follow the general pattern outlined [here](https://doc.qt.io/qt-6/audiooverview.html#low-level-audio-playback-and-recording).

> Conversely, for pull mode with QAudioSource, when audio data is available then the data will be written directly to the QIODevice.
