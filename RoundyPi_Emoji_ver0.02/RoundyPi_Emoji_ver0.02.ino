/*******************************************************************************
 * PNG Image Viewer
 * This is a simple PNG image viewer example
 * Image Source: https://github.com/logos
 *
 * Dependent libraries:
 * PNGdec: https://github.com/bitbank2/PNGdec.git
 * 
 * Setup steps:
 * 1. Change your LCD parameters in Arduino_GFX setting
 * 2. Upload PNG file
 *   FFat (ESP32):
 *     upload FFat (FatFS) data with ESP32 Sketch Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *   LittleFS (ESP32 / ESP8266 / Pico):
 *     upload LittleFS data with ESP8266 LittleFS Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *     ESP8266: https://github.com/earlephilhower/arduino-esp8266littlefs-plugin
 *     Pico: https://github.com/earlephilhower/arduino-pico-littlefs-plugin.git
 *   SPIFFS (ESP32):
 *     upload SPIFFS data with ESP32 Sketch Data Upload:
 *     ESP32: https://github.com/lorol/arduino-esp32fs-plugin
 *   SD:
 *     Most Arduino system built-in support SD file system.
 *     Wio Terminal require extra dependant Libraries:
 *     - Seeed_Arduino_FS: https://github.com/Seeed-Studio/Seeed_Arduino_FS.git
 *     - Seeed_Arduino_SFUD: https://github.com/Seeed-Studio/Seeed_Arduino_SFUD.git
 ******************************************************************************/

/*******************************************************************************
 * Start of Arduino_GFX setting
 * 
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * Arduino Nano, Micro and more: CS:  9, DC:  8, RST:  7, BL:  6
 * ESP32 various dev board     : CS:  5, DC: 27, RST: 33, BL: 22
 * ESP32-C3 various dev board  : CS:  7, DC:  2, RST:  1, BL:  3
 * ESP32-S2 various dev board  : CS: 34, DC: 35, RST: 33, BL: 21
 * ESP32-S3 various dev board  : CS: 40, DC: 41, RST: 42, BL: 48
 * ESP8266 various dev board   : CS: 15, DC:  4, RST:  2, BL:  5
 * Raspberry Pi Pico dev board : CS: 17, DC: 27, RST: 26, BL: 28
 * RTL8720 BW16 old patch core : CS: 18, DC: 17, RST:  2, BL: 23
 * RTL8720_BW16 Official core  : CS:  9, DC:  8, RST:  6, BL:  3
 * RTL8722 dev board           : CS: 18, DC: 17, RST: 22, BL: 23
 * RTL8722_mini dev board      : CS: 12, DC: 14, RST: 15, BL: 13
 * Seeeduino XIAO dev board    : CS:  3, DC:  2, RST:  1, BL:  0
 * Teensy 4.1 dev board        : CS: 39, DC: 41, RST: 40, BL: 22
 ******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <map>

#include <Arduino_GFX_Library.h>

#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 12
#define TFT_SCK 10
#define TFT_MOSI 11
#define TFT_MISO -1  // no data coming back
#define TFT_LED 13
#define VSPI    spi1

#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
//Arduino_DataBus *bus = create_default_Arduino_DataBus();
Arduino_DataBus *bus = new Arduino_RPiPicoSPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, spi1);  //create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
//Arduino_GFX *gfx = new Arduino_ILI9341(bus, DF_GFX_RST, 3 /* rotation */, false /* IPS */);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);  

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
#include <Seeed_FS.h>
#include <SD/Seeed_SD.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
#define MISO          16    // Definitions needed for SD library
#define MOSI          19
#define SCK           18
#define SD_CS         17
#define SPI_SPEED     SD_SCK_MHZ(10)
#include <LittleFS.h>
#include <SDFS.h>
SDFSConfig c2;
#elif defined(ESP32)
#include <FFat.h>
#include <LittleFS.h>
#include <SPIFFS.h>
#include <SD.h>
#elif defined(ESP8266)
#include <LittleFS.h>
#include <SD.h>
#else
#include <SD.h>
#endif

#include <PNGdec.h>
PNG png;

std::map<eTaskState, const char *> eTaskStateName { {eReady, "Ready"}, { eRunning, "Running" }, {eBlocked, "Blocked"}, {eSuspended, "Suspended"}, {eDeleted, "Deleted"} };
void ps() 
{
  int tasks = uxTaskGetNumberOfTasks();
  TaskStatus_t *pxTaskStatusArray = new TaskStatus_t[tasks];
  unsigned long runtime;
  tasks = uxTaskGetSystemState( pxTaskStatusArray, tasks, &runtime );
  Serial.printf("# Tasks: %d\n", tasks);
  Serial.println("ID, NAME, STATE, PRIO, CYCLES");
  for (int i=0; i < tasks; i++) 
  {
    Serial.printf("%d: %-16s %-10s %d %lu\n", i, pxTaskStatusArray[i].pcTaskName, eTaskStateName[pxTaskStatusArray[i].eCurrentState], pxTaskStatusArray[i].uxCurrentPriority, pxTaskStatusArray[i].ulRunTimeCounter);
  }
  delete[] pxTaskStatusArray;
}

volatile bool LoopZeroDone = false;
volatile bool LoopOneDone = false;

volatile bool SetupZeroDone = false;
volatile bool SetupOneDone = false;

volatile bool GFXBufferReady = false;
volatile bool GFXClearScreen = false;

volatile uint32_t Draw_y, Draw_Width;

int16_t w, h, xOffset=0, yOffset=0;

// Functions to access a file on the SD card
File pngFile;

#include "emoji_list.h"

//char *fn[] = {"/cir0001.png","/cir0002.png","/cir0003.png","/cir0004.png","/cir0005.png","/cir0006.png","/cir0007.png","/cir0008.png","/cir0009.png","/cir0010.png",
//              "/cir0011.png","/cir0012.png","/cir0013.png","/cir0014.png","/cir0015.png","/cir0016.png","/cir0017.png","/cir0018.png","/cir0019.png","/cir0020.png"};

//char *fn[] = {"/out0001.png","/out0002.png","/out0003.png","/out0004.png","/out0005.png","/out0006.png","/out0007.png","/out0008.png","/out0009.png","/out0010.png",
//              "/out0011.png","/out0012.png","/out0013.png","/out0014.png","/out0015.png","/out0016.png","/out0017.png","/out0018.png","/out0019.png"};

//char *fn[] = {"/out0021.png","/out0022.png","/out0023.png","/out0024.png","/out0025.png","/out0026.png","/out0027.png","/out0028.png","/out0029.png","/out0030.png",
//              "/out0031.png","/out0032.png","/out0033.png","/out0034.png","/out0035.png","/out0036.png","/out0037.png","/out0038.png","/out0039.png","/out0040.png"};

uint16_t usPixels[320];
uint8_t usMask[320];

void *myOpen(const char *filename, int32_t *size)
{
/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
  pngFile = SD.open(filename, "r");
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  //pngFile = LittleFS.open(filename, "r");
  pngFile = SDFS.open(filename, "r");
#elif defined(ESP32)
  // pngFile = FFat.open(filename, "r");
  pngFile = LittleFS.open(filename, "r");
  // pngFile = SPIFFS.open(filename, "r");
  // pngFile = SD.open(filename, "r");
#elif defined(ESP8266)
  pngFile = LittleFS.open(filename, "r");
  // pngFile = SD.open(filename, "r");
#else
  pngFile = SD.open(filename, FILE_READ);
#endif

  if (!pngFile || pngFile.isDirectory())
  {
    Serial.print(F("ERROR: Failed to open "));
    Serial.print(filename);
    Serial.println(F(" file for reading"));
    //gfx->println(F("ERROR: Failed to open " filename " file for reading"));
  }
  else
  {
    *size = pngFile.size();
    //Serial.printf("Opened '%s', size: %d\n", filename, *size);
  }

  return &pngFile;
}

void myClose(void *handle)
{
  if (pngFile)
    pngFile.close();
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
  if (!pngFile)
    return 0;
  return pngFile.read(buffer, length);
}

int32_t mySeek(PNGFILE *handle, int32_t position)
{
  if (!pngFile)
    return 0;
  return pngFile.seek(position);
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw)
{
  while(GFXBufferReady);
  
  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0x00000000);
  png.getAlphaMask(pDraw, usMask, 1);

  Draw_y = pDraw->y;
  Draw_Width = pDraw->iWidth;

  GFXBufferReady = true;
}

void blink(void *param) 
{
  (void) param;
  pinMode(LED_BUILTIN, OUTPUT);
  
  while (true) 
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(750);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
}

void setup()
{
  Serial.begin(115200);

  xTaskCreate(blink, "BLINK", 128, nullptr, 1, nullptr);
  delay(5000);
  
  // while (!Serial);
  Serial.println("PNG Image Viewer");

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 4000000UL))
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  //if (!LittleFS.begin())
  Serial.println("Init SPI Conf.");
  Serial.flush();
  
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  SPI.setRX(MISO);
  SPI.setTX(MOSI);
  SPI.setSCK(SCK);
  SPI.setCS(SD_CS);
  SPISettings pasSPI(50000000, MSBFIRST, SPI_MODE0);
  SPI.begin(false);

  
  c2.setCSPin(SD_CS);
  SDFS.setConfig(c2);
  
  if (!SDFS.begin())
#elif defined(ESP32)
  // if (!FFat.begin())
  if (!LittleFS.begin())
  // if (!SPIFFS.begin())
  // if (!SD.begin(SS))
#elif defined(ESP8266)
  if (!LittleFS.begin())
  // if (!SD.begin(SS))
#else
  if (!SD.begin())
#endif
  {
    Serial.println(F("ERROR: File System Mount Failed!"));Serial.flush();
    gfx->println(F("ERROR: File System Mount Failed!"));
  } else {
    Serial.println(F("File System Mounted!"));Serial.flush();

    Serial.println(F("Waiting for GFX Init on CPU1"));Serial.flush();
    while(!SetupOneDone);
    Serial.println(F("Done GFX Init!"));Serial.flush();

    int i=0;
    while(true)
    {
      unsigned long start = millis();
      int rc;
      rc = png.open(fn[i], myOpen, myClose, myRead, mySeek, PNGDraw);
      if (rc == PNG_SUCCESS)
      {  
        int16_t pw = png.getWidth();
        int16_t ph = png.getHeight();
  
        xOffset = (w - pw) / 2;
        yOffset = (h - ph) / 2;
                
        rc = png.decode(NULL, 0);      
        png.close();

        delay(5000);
        GFXClearScreen = true;
      } else {
        Serial.println("png.open() failed!");
      }
      i++;
      if ( i>=87) i=0;
    }
    Serial.println("STOP");
  }

  delay(5000); // 5 seconds
}

void loop()
{
  delay(1);
}

void setup1()
{
  delay(5000);
  
  Serial.println(F("Init Display!"));Serial.flush();
  // Init Display
  gfx->begin(60000000);
  gfx->fillScreen(BLACK);

  w = gfx->width(), h = gfx->height();
  gfx->fillScreen(BLACK);
  
#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif
  /* GFX Init and setup1 terminated. */
  SetupOneDone = true;
}

void loop1()
{
  if (GFXBufferReady)
  {
    GFXBufferReady=false;
    gfx->draw16bitRGBBitmap(xOffset, yOffset+Draw_y, usPixels, usMask, Draw_Width, 1);
  } else {
    if ( GFXClearScreen)
    {
      GFXClearScreen = false;
      gfx->fillScreen(BLACK);
    }
  }
}
