# Examples

The example applications in `src/` are small firmware entry points that show how
to compose `libFRX` with `jescore`.

They are intentionally thin:

- initialize `jescore`;
- initialize one or more `libFRX` modules with default configuration;
- expose a CLI-compatible control surface through the module's `jescore` job.

Available examples:

- [Audio demo app](audio_demo.md)
- [RTC demo app](rtc_demo.md)
- [SD card demo app](sdcard_demo.md)
