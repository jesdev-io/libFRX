# `libFRX`: An Open-Source Field-Recording Library

<div class="frx-hero" markdown>

<p class="frx-hero-kicker">Need it? Make it. <a href="https://jesdev.io/">jesdev.io</a>🔆</p>

`libFRX` is a module-based library for ESP32 field-recording applications. It provides
modules for audio I/O, SD-card file access, WAV handling, DSP helpers, signal
synthesis, external flash, external time-keeping and [`jescore`](https://github.com/jesdev-io/jescore/wiki)-compatible CLI control.

<span class="frx-chip">ESP32</span>
<span class="frx-chip">audio I/O</span>
<span class="frx-chip">SD-Card</span>
<span class="frx-chip">WAV files</span>
<span class="frx-chip">DSP</span>
<span class="frx-chip">CLI-ready</span>

</div>

## Before you continue...

`libFRX` is easiest to use with [PlatformIO](https://platformio.org/), because module enable flags and board-specific macros fit naturally into `platformio.ini` environments. Classical ESP-IDF or Arduino integration is possible too: add the library sources and define the same macros in your build system. This documentation focuses on the PlatformIO path and does not provide a separate classical-build guide.

`libFRX` is not just a set of static function calls. It is a real-time runtime based on [`jescore`](https://github.com/jesdev-io/jescore/wiki), the runtime control layer used by the examples and CLI adapters. It lets firmware expose named jobs over a serial command interface, so a recorder can be inspected and controlled from a host computer without adding a custom UI first. It in turn runs with [FreeRTOS](https://www.freertos.org/Documentation/00-Overview). 

`libFRX` modules can be used directly from firmware code, but `jescore` support is what makes them convenient to drive from [`jescorecli`](https://github.com/jesdev-io/jescorecli). That way, testing, automation and UI-backend glue are a piece of cake.

## What this library is for

If you have an ESP32, one or more microphones or a codec, an SD card, and supporting
periphery, `libFRX` provides the reusable tools needed to build a field
recorder or other related embedded audio devices. It handles peripheral and stream interaction, preemptive multitasking and system-native CLI access. It enables you to record and play back audio of up to 4 channels, provides a micro file system, commonly needed DSP routines and can be controlled entirely programmatically if desired. While it focuses on field recording applications, you can build a rich set of embedded audio devices with `libFRX`: **All tools are at your fingertips.**

## Start here

<div class="grid cards" markdown>

-   **API overview**

    Browse modules, dependencies, and public interfaces.

    [:octicons-arrow-right-24: Open API overview](reference/index.md)

-   **Examples**

    Start from minimal firmware entry points for audio and SD-card control.

    [:octicons-arrow-right-24: Open examples](examples/index.md)

-   **Configuration**

    Learn how build flags, defaults, and `_init_default()` fit together.

    [:octicons-arrow-right-24: Open configuration](reference/configuration.md)

-   **Meet the back-end: `jescore`**

    Read up on the `jesdev.io` in-house RTOS flavor of FreeRTOS and see how it works.

    [:octicons-arrow-right-24: Open `jescore` wiki](https://github.com/jesdev-io/jescore/wiki)

</div>

## Need more? Contribute!

Want to add a module, improve docs, fix a driver, or extend a demo? Start with the repository contribution guide: [CONTRIBUTING.md](https://github.com/jesdev-io/libFRX/blob/main/CONTRIBUTING.md).
