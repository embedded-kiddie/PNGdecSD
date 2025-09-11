/*================================================================================
 * Loading PNG files from SD card to LCD with TFT_eSPI and PNGdec
 * https://github.com/bitbank2/PNGdec (version >= 1.1.4)
 * https://github.com/bitbank2/PNGdec/wiki
 *================================================================================*/
#include <SPI.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>
#include <string.h>

#define USE_SDFAT 1
#if USE_SDFAT
#define DISABLE_FS_H_WARNING
#include <SdFat.h>
#else
#include <SD.h>
#endif

/*--------------------------------------------------------------------------------
 * SD chip select pin and SPI clock frequency
 *--------------------------------------------------------------------------------*/
#define SD_CS         SS        // defined in pins_arduino.h
#define SD_SPI_CLOCK  24000000  // 24 MHz for ESP32-2432S028

#if USE_SDFAT
static SdFat SD;
typedef FsFile File;

// SHARED_SPI or DEDICATED_SPI
#define USE_DEDICATED_SPI   1

#if USE_DEDICATED_SPI
static SPIClass sd_spi = SPIClass(HSPI);
#define SD_CONFIG SdSpiConfig(SD_CS, DEDICATED_SPI, SD_SPI_CLOCK, &sd_spi)
#else
#define SD_CONFIG SdSpiConfig(SD_CS, SHARED_SPI, SD_SPI_CLOCK, &SPI) // VSPI
#endif // USE_DEDICATED_SPI

#else // Espressif standard SD library
#define SD_CONFIG SD_CS, SPI, SD_SPI_CLOCK
#endif // USE_SDFAT

/*--------------------------------------------------------------------------------
 * TFT settings
 *--------------------------------------------------------------------------------*/
#define TFT_WIDTH   240
#define TFT_HEIGHT  320
#define TFT_ROTATION  0

/*--------------------------------------------------------------------------------
 * Global variables
 *--------------------------------------------------------------------------------*/
static TFT_eSPI tft = TFT_eSPI();
static File myFile;
static PNG png;

// Display start position
static uint16_t start_x = 0;
static uint16_t start_y = 0;

/*--------------------------------------------------------------------------------
 * typedef void * (PNG_OPEN_CALLBACK)(char *szFilename, int32_t *pFileSize);
 *--------------------------------------------------------------------------------*/
static void * myOpen(const char *filename, int32_t *pFileSize) {
  if (myFile) {
    *pFileSize = myFile.size();
    return &myFile;
  } else {
    *pFileSize = 0;
    return NULL;
  }
}

/*--------------------------------------------------------------------------------
 * typedef void (PNG_CLOSE_CALLBACK)(void *pHandle);
 *--------------------------------------------------------------------------------*/
static void myClose(void *pHandle) {
  if (myFile) {
    myFile.close();
  }
}

/*--------------------------------------------------------------------------------
 * typedef int32_t (PNG_READ_CALLBACK)(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen);
 *--------------------------------------------------------------------------------*/
static int32_t myRead(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  if (myFile) {
    int32_t len = myFile.read(pBuf, iLen);
    return len;
  } else {
    return 0;
  }
}

/*--------------------------------------------------------------------------------
 * typedef int32_t (PNG_SEEK_CALLBACK)(PNGFILE *pFile, int32_t iPosition);
 *--------------------------------------------------------------------------------*/
static int32_t mySeek(PNGFILE *pFile, int32_t iPosition) {
  if (myFile) {
    return myFile.seek(iPosition);
  } else {
    return 0;
  }
}

/*--------------------------------------------------------------------------------
 * This next function will be called during decoding of the png file to
 * render each image line to the TFT. If you use a different TFT library
 * you will need to adapt this function to suit.
 *--------------------------------------------------------------------------------*/
static int pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[TFT_WIDTH > TFT_HEIGHT ? TFT_WIDTH : TFT_HEIGHT];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

//tft.startWrite();
  tft.pushImage(start_x, start_y + pDraw->y, pDraw->iWidth, 1, lineBuffer);
//tft.endWrite();

  return true;
}

void setup(void) {
  Serial.begin(115200);
  while (millis() < 1000);
  Serial.println(USER_SETUP_INFO);        // Check if User_Setup.h is included correctly

  tft.init();                             // Start the tft display
  tft.initDMA();                          // Enable DMA
  tft.setRotation(TFT_ROTATION);          // Set the TFT display rotation
  tft.fillScreen(TFT_WHITE);              // Clear the screen before writing to it
  tft.setTextColor(TFT_BLACK, TFT_WHITE); // Set text color black in white

#if USE_DEDICATED_SPI
  sd_spi.begin(SCK, MISO, MOSI, SD_CS);
#endif

  if (SD.begin(SD_CONFIG)) {
    Serial.println("SD Card initialized.");
  } else {
    Serial.println("SD Card initialization failed!");
    while (true);
  }
}

void loop(void) {
  File root = SD.open("/");
  while (true) {
    myFile = root.openNextFile();
    if (!myFile) {
      root.rewindDirectory(); // No more files, rewind or break
      break;
    }

    if (myFile.isDirectory()) {
      myFile.close();
      continue;
    }

#if USE_SDFAT
    char buf[BUFSIZ]; // 128
    myFile.getName(buf, sizeof(buf));
    const char *name = buf;
#else
    const char *name = myFile.name();
#endif
    if (name[0] != '.' && (strstr(name, ".PNG") || strstr(name, ".png"))) {

      int ret = png.open(name, myOpen, myClose, myRead, mySeek, pngDraw);

      if (ret == PNG_SUCCESS) {
        uint32_t t = millis();

        tft.startWrite();
          ret = png.decode(NULL, 0); // decode and draw
        tft.endWrite();

        Serial.printf("%s: %d\n", name, millis() - t);
      }

      png.close();
      delay(5000);
    }
    myFile.close();
  }
  root.close();
}