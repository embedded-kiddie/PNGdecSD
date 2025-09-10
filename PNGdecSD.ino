/*================================================================================
 * Loading PNG files from SD card to LCD with TFT_eSPI and PNGdec
 * https://github.com/bitbank2/PNGdec (version >= 1.1.6)
 * https://github.com/bitbank2/PNGdec/wiki
 *================================================================================*/
#include <SD.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>

#define DEBUG   0
#if     DEBUG
#define DBG_EXEC(x) x
#else
#define DBG_EXEC(x)
#endif

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
    DBG_EXEC(Serial.println("open: success"));
    *pFileSize = myFile.size();
    return &myFile;
  } else {
    DBG_EXEC(Serial.println("open: failed"));
    *pFileSize = 0;
    return NULL;
  }
}

/*--------------------------------------------------------------------------------
 * typedef void (PNG_CLOSE_CALLBACK)(void *pHandle);
 *--------------------------------------------------------------------------------*/
static void myClose(void *pHandle) {
  DBG_EXEC(Serial.println("close"));

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
    DBG_EXEC(Serial.printf("read: %d --> %d\n", iLen, len));
    return len;
  } else {
    DBG_EXEC(Serial.println("read: error"));
    return 0;
  }
}

/*--------------------------------------------------------------------------------
 * typedef int32_t (PNG_SEEK_CALLBACK)(PNGFILE *pFile, int32_t iPosition);
 *--------------------------------------------------------------------------------*/
static int32_t mySeek(PNGFILE *pFile, int32_t iPosition) {
  DBG_EXEC(Serial.printf("seek: %d\n", iPosition));

  if (myFile) {
    return myFile.seek(iPosition);
  } else {
    DBG_EXEC(Serial.println("seek: error"));
    return 0;
  }
}

/*--------------------------------------------------------------------------------
 * This next function will be called during decoding of the png file to
 * render each image line to the TFT. If you use a different TFT library
 * you will need to adapt this function to suit.
 *--------------------------------------------------------------------------------*/
static int pngDraw(PNGDRAW *pDraw) {
  DBG_EXEC(Serial.printf("pngDraw y: %d, iWidth: %d\n", pDraw->y, pDraw->iWidth));

  uint16_t lineBuffer[TFT_WIDTH > TFT_HEIGHT ? TFT_WIDTH : TFT_HEIGHT];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

  tft.startWrite();
  tft.pushImage(start_x, start_y + pDraw->y, pDraw->iWidth, 1, lineBuffer);
  tft.endWrite();

  return pDraw->iWidth;
}

void setup(void) {
  DBG_EXEC({
    Serial.begin(115200);
    while (millis() < 1000);
    Serial.println(USER_SETUP_INFO);      // Check if User_Setup.h is included correctly
  });

  tft.init();                             // Start the tft display
  tft.initDMA();                          // Enable DMA
  tft.setRotation(TFT_ROTATION);          // Set the TFT display rotation
  tft.fillScreen(TFT_WHITE);              // Clear the screen before writing to it
  tft.setTextColor(TFT_BLACK, TFT_WHITE); // Set text color black in white

  if (SD.begin(SD_CONFIG)) {
    DBG_EXEC(Serial.println("SD Card initialized."));
  } else {
    DBG_EXEC(Serial.println("SD Card initialization failed!"));
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

    const char *name = myFile.name();
    if (name[0] != '.' && (strstr(name, ".PNG") || strstr(name, ".png"))) {
      DBG_EXEC(Serial.printf("Displaying: %s\n", name));

      int ret = png.open(name, myOpen, myClose, myRead, mySeek, pngDraw);
      DBG_EXEC(Serial.printf("open: %d\n", ret)); // PNG_SUCCESS = 0

      if (ret == PNG_SUCCESS) {
        DBG_EXEC(Serial.printf("width: %d, height: %d\n", png.getWidth(), png.getHeight()));

        ret = png.decode(NULL, 0); // decode and draw

        DBG_EXEC(Serial.printf("decode: %d\n", ret));
      }

      png.close();
      delay(5000);
    }
    myFile.close();
  }
  root.close();
}