# API Reference

`libFRX` offers standalone and co-dependent modules for various tasks associated with audio field recording. Each module needs to be enabled with a build flag at compile time in a manual compile call or in the `build_flags` section of a PlatformIO environment:

```bash
-DFRX_ENABLE_MODULE_<module>
```

Each module's documentation holds exact function/struct/macro argument, member, and return descriptions and additional information on why they exist and how they can be used within a greater scope. Be aware that some modules depend on others, meaning you will not be able to utilize the **WAV** module if you don't enable a system for them to be written to, in this case the **SD Card** module. 

- [Configuration and defaults](configuration.md)
- [Audio](audio.md)
- [DSP](generated/dsp_frx_api.md)
- [External Flash](ext_flash.md)
- [I2C](i2c.md)
  - [DS3231 RTC](i2c/rtc_ds3231.md)
- [SD Card](sdcard.md)
- [Synth](generated/synth_api.md)
- [Utils](generated/utils_api.md)
- [WAV](generated/wav_api.md)
