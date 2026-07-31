# `libFRX`: An Open-Source Field-Recording Library

<div class="frx-hero" markdown>

`libFRX` is a module-based library for ESP32 field-recording applications. It provides
modules for audio I/O, SD-card file access, WAV handling, DSP helpers, signal
synthesis, external flash, external time-keeping and [`jescore`](https://github.com/jesdev-io/jescore/wiki)-compatible CLI control.

<span class="frx-chip">ESP32</span>
<span class="frx-chip">audio I/O</span>
<span class="frx-chip">SD/WAV</span>
<span class="frx-chip">CLI-ready</span>

</div>

## Before you continue... 🔆

`libFRX` is not just a set of static function calls. It is a real-time runtime based on [`jescore`](https://github.com/jesdev-io/jescore/wiki), the runtime control layer used by the examples and CLI adapters. It lets firmware expose named jobs over a serial command interface, so a recorder can be inspected and controlled from a host computer without adding a custom UI first. It in turn runs with [FreeRTOS](https://www.freertos.org/Documentation/00-Overview). 

`libFRX` modules can be used directly from firmware code, but `jescore` support is what makes them convenient to drive from [`jescorecli`](https://github.com/jesdev-io/jescorecli). That way, testing, automation and UI-backend glue are a piece of cake.

## What this library is for

If you have an ESP32, one or more microphones or a codec, an SD card, and supporting
periphery, `libFRX` provides the reusable tools needed to build a field
recorder or other related embedded audio device. It handles peripheral and stream interaction, preemptive multitasking and system-native CLI access. It enables you to record and play back audio of up to 4 channels, provides a micro file system, commonly needed DSP routines and can be controlled entirely programmatically if desired.  

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

</div>

## Need more? Contribute!

Want to add a module, improve docs, fix a driver, or extend a demo? Start with the repository contribution guide: [CONTRIBUTING.md](https://github.com/jesdev-io/libFRX/blob/main/CONTRIBUTING.md).
