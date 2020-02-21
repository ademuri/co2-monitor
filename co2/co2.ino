#include <Arduino.h>

// These deps are needed by dependent libraries. Due to some PlatformIO bug, the libraries can only find them if they're #included here first.
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <WiFiClientSecure.h>

#include <Adafruit_SSD1306.h>
#include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>
#include <MHZ19.h>
#include <Wire.h>
#include <WiFi.h>

#include "secrets.h"

static const uint32_t kWarmupTime = 3 * 60 * 1000;
static const int16_t kScreenWidth = 128;
// Sensor refreshes every 5s
static const uint32_t kLogDelay = 5 * 1000;

MHZ19 sensor;
Adafruit_SSD1306 display(kScreenWidth, /* height */ 64, &Wire, /* oled reset */ -1);
AdafruitIO_WiFi io(kIoUsername, kIoKey, kSsid, kSsidPassword);
AdafruitIO_Feed *feed;

uint32_t log_at = 0;

void serial_printf(const char *fmt, ...) {
  static char buf[256];  // Statically allocate 256 bytes for printf'ing.
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 128, fmt, args);
  va_end(args);
  Serial.print(buf);
}

void display_co2(int ppm) {
  display.setCursor(0, 16);  // Start at top-left corner

  static char buf[5];
  snprintf(buf, 5, "%4d", ppm);
  for (int i = 0; i < 4; i++) {
    display.write(buf[i]);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // Note: this LED is inverted
  digitalWrite(LED_BUILTIN, LOW);
  delay(1);

  Serial1.begin(9600, SERIAL_8N1, 22, 23);
  sensor.begin(Serial1);
  // Don't auto calibrate, since that assumes that the sensor will be exposed
  // to 400 PPM (outside) air for at least 20m for each 24h on.
  sensor.autoCalibration(false);
  // Return 0 instead of garbage values when the sensor is initializing.
  sensor.setFilter();

  Wire.begin(5, 4);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) {
      Serial.println(F("SSD1306 allocation failed"));
      delay(100);
    }
  }
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(SSD1306_WHITE);  // Draw white text (it's the only option)
  display.cp437(true);  // Use full 256 char 'Code Page 437' fontlay(100);

  // connect to io.adafruit.com
  io.connect();
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    static int pos = 0;
    display.clearDisplay();
    display.setCursor(pos % 2 ? 10 : 0, 0);
    display.write('i');
    display.write('o');
    display.display();
    delay(100);
    pos++;
  }
  feed = io.feed("co2");

  digitalWrite(LED_BUILTIN, HIGH);

  // The sensor takes 3 minutes to warm up - show a timer bar while this happens
  int co2 = 0;
  while (millis() < kWarmupTime || co2 == 0) {
    co2 = sensor.getCO2();

    display.clearDisplay();

    int16_t width = millis() * kScreenWidth / kWarmupTime;
    display_co2(co2);
    display.fillRect(0, 0, width, 4, SSD1306_WHITE);
    display.display();
    delay(100);
  }

  display.setTextColor(SSD1306_WHITE);  // Draw white text
}

void loop() {
  io.run();
  int co2 = sensor.getCO2();

  display.clearDisplay();
  display_co2(co2);

  if (millis() > log_at) {
    feed->save(co2);
    log_at = millis() + kLogDelay;
    display.fillCircle(8, 120, 4, SSD1306_WHITE);
  }

  display.display();

  delay(500);
}
