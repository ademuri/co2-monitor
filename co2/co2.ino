#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <MHZ19.h>
#include <SPI.h>
#include <Wire.h>

static const uint32_t kWarmupTime = 3 * 60 * 1000;
static const int16_t kScreenWidth = 128;

MHZ19 sensor;
Adafruit_SSD1306 display(kScreenWidth, /* height */ 64, &Wire, /* oled reset */ -1);

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

  digitalWrite(LED_BUILTIN, HIGH);

  // The sensor takes 3 minutes to warm up - show a timer bar while this happens
  while (millis() < kWarmupTime) {
    display.clearDisplay();

    int16_t width = millis() * kScreenWidth / kWarmupTime;
    display_co2(sensor.getCO2());
    display.fillRect(0, 0, width, 4, SSD1306_WHITE);
    display.display();
    delay(100);
  }

  display.setTextColor(SSD1306_WHITE);  // Draw white text
}

void loop() {
  display.clearDisplay();
  display_co2(sensor.getCO2());
  display.display();
  delay(100);
}
