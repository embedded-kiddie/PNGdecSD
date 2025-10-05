/*================================================================================
 * Loading PNG / JPEG files from SD card to LCD with LovyanGFX
 *================================================================================*/
#include <SD.h>

// Auto-detect and manual configuration settings
#define USE_AUTODETECT false

#if USE_AUTODETECT
// Some devices do not support auto-detect since the panel ID cannot be read.
#define LGFX_AUTODETECT
#include <LovyanGFX.h>
#else
// false: Panel driver: ILI9341 (micro-USB x 1 type)
// true : Panel driver: ST7789  (micro-USB x 1 + USB-C x 1 type)
#define DISPLAY_CYD_2USB  true
#include "LGFX_ESP32_2432S028R_CYD.hpp"
#endif // USE_AUTODETECT

static LGFX tft;

/*--------------------------------------------------------------------------------
 * SD chip select pin and SPI clock frequency
 *--------------------------------------------------------------------------------*/
#define SD_CS         SS        // defined in pins_arduino.h
#define SD_SPI_CLOCK  24000000  // 24 MHz for ESP32-2432S028
#define SD_CONFIG     SD_CS, SPI, SD_SPI_CLOCK

/*--------------------------------------------------------------------------------
 * TFT settings
 *--------------------------------------------------------------------------------*/
#define TFT_WIDTH   240
#define TFT_HEIGHT  320
#define TFT_ROTATION  0

/*--------------------------------------------------------------------------------
 * Global variables
 *--------------------------------------------------------------------------------*/

void setup(void) {
  tft.init();                             // Start the tft display
  tft.initDMA();                          // Enable DMA
  tft.setRotation(TFT_ROTATION);          // Set the TFT display rotation
  tft.fillScreen(TFT_WHITE);              // Clear the screen before writing to it
  tft.setTextColor(TFT_BLACK, TFT_WHITE); // Set text color black in white
  tft.setBrightness(255);                 // Duty: 0 to 255

  if (!SD.begin(SD_CONFIG)) {
    tft.print("SD Card initialization failed!");
    while (true);
  }
}

void loop(void) {
  File root = SD.open("/");
  while (true) {
    File myFile = root.openNextFile();
    if (!myFile) {
      root.rewindDirectory(); // No more files, rewind or break
      break;
    }

    if (myFile.isDirectory()) {
      myFile.close();
      continue;
    }

    const char *name = myFile.name();
    if (name[0] == '.') {
      myFile.close();
      continue;
    }

    std::string path = std::string("/") + name;

    if (strstr(name, ".png")) {
      tft.drawPngFile(SD, path.c_str(), 0, 0);
      delay(5000);
    }

    else if (strstr(name, ".jpg") || strstr(name, ".jpeg")) {
      tft.drawJpgFile(SD, path.c_str(), 0, 0);
      delay(5000);
    }

    else if (strstr(name, ".qoi")) {
      tft.drawQoiFile(SD, path.c_str(), 0, 0);
      delay(5000);
    }

    myFile.close();
  }

  root.close();
}