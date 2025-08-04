# Overview

The AudioCapture application uses Qt's [QAudioSource](https://doc.qt.io/qt-6/qaudiosource.html) for cross-platform audio capturing.
This program has been tested on Windows and MacOS.

The Windows release requires vc_redist.x64.exe [from Microsoft](https://support.microsoft.com/en-gb/help/2977003/the-latest-supported-visual-c-downloads).

# Getting Started

Download the latest version [from the releases page](https://github.com/labstreaminglayer/App-AudioCapture/releases).

## Extra Dependencies (Ubuntu only)

* `sudo apt-get install libqt6multimedia6`
* TODO: Instructions to download and install liblsl
* Other platforms ship with the Qt libraries.

## Usage

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

# Build

The build instructions for this app are mostly the same as the [generic Qt-based LSL App build instructions](https://labstreaminglayer.readthedocs.io/dev/app_build.html).

Additionally, Qt Multimedia development libraries are required:

* On Ubuntu, install it with `sudo apt-get install qt6-multimedia-dev`.
* On other platforms, the Qt Multimedia libraries should be included in the Qt installation.

On Mac, it appears to be necessary to use the Xcode generator: `cmake -B build -S . -DCMAKE_INSTALL_PREFIX="build/install" -G Xcode`

# Further Notes

The previous version of AudioCaptureWin can be found as [release v0.1](https://github.com/labstreaminglayer/App-AudioCapture/releases/tag/v0.1) in this repository.

For Windows XP there is an older LSL audio recording app available on request; it uses the [irrKlang](http://www.ambiera.com/irrklang/) audio library, which in turn uses DirectX audio on Windows. That application does not support accurate time synchronization and is therefore deprecated.

# Developer Notes

There were quite a few changes with Qt6. We follow the general pattern outlined [here](https://doc.qt.io/qt-6/audiooverview.html#low-level-audio-playback-and-recording).

> Conversely, for pull mode with QAudioSource, when audio data is available then the data will be written directly to the QIODevice.
