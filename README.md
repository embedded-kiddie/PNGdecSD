# Loading PNG files from SD card to LCD with TFT_eSPI and PNGdec

## Hardware
- ESP32-2432S028R (aka CYD)

## Software
- `PNGdecSD.ino`  
  Display a full-size PNG images like a photo frame.
- `User_Setup.h`  
  [TFT_eSPI][4] configuration file for CYD and ILI9341 / ST7789.

## Components
| Components                 | Link / Version        |
| -------------------------- | --------------------- |
| Arduino IDE                | [Version 2.3.4][1]    |
| esp32 by Espressif Systems | [v3.3.0][2]           |
| Platform type              | [ESP32 Dev Module][3] |
| TFT_eSPI                   | [1.2.7][4]            |
| SD card library            | [SD library][5]       |
| PNG decode library         | [PNGdec >= 1.1.4][6]  |

## To Do
- [ ] Parallelization utilising dual-core with dedicated SPI bus for SD and TFT.
- [ ] Transition effect using sprites.

## Sample photos
- Sample1.png by [Emma Swoboda](https://unsplash.com/@emmakphoto?utm_content=creditCopyText&utm_medium=referral&utm_source=unsplash) on [Unsplash](https://unsplash.com/photos/snow-capped-mountain-peak-bathed-in-golden-sunlight-B9fkw_aO6fo?utm_content=creditCopyText&utm_medium=referral&utm_source=unsplash)
- Sample2.png by [Yevhenii Deshko](https://unsplash.com/@edeshko?utm_content=creditCopyText&utm_medium=referral&utm_source=unsplash) on [Unsplash](https://unsplash.com/photos/white-church-steeple-against-a-pastel-sky-with-birds-ieY_9lJnLNs?utm_content=creditCopyText&utm_medium=referral&utm_source=unsplash)

[1]: https://github.com/arduino/arduino-ide/releases/tag/2.3.4 "Release 2.3.4 · arduino/arduino-ide"
[2]: https://github.com/espressif/arduino-esp32/releases/tag/3.3.0 "Release Arduino Release v3.3.0 based on ESP-IDF v5.5.0 · espressif/arduino-esp32"
[3]: https://github.com/espressif/arduino-esp32/tree/master/variants/esp32 "arduino-esp32/variants/esp32 at master · espressif/arduino-esp32"
[4]: https://github.com/lovyan03/LovyanGFX/releases/tag/1.2.7 "Release 1.2.7 · lovyan03/LovyanGFX"
[5]: https://github.com/espressif/arduino-esp32/tree/master/libraries/SD "arduino-esp32/libraries/SD at master · espressif/arduino-esp32"
[6]: https://github.com/bitbank2/PNGdec "bitbank2/PNGdec: An optimized PNG decoder suitable for microcontrollers and PCs"