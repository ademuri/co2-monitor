#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <MHZ19.h>
#include <SPI.h>
#include <Wire.h>

MHZ19 sensor;
Adafruit_SSD1306 display(128, 64, &Wire, /* oled reset */ -1);

void debug_printf(const char *fmt, ...) {
  static char buf[256];  // Statically allocate 256 bytes for printf'ing.
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 128, fmt, args);
  va_end(args);
  Serial.print(buf);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);

  Serial1.begin(9600, SERIAL_8N1, 22, 23);
  sensor.begin(Serial1);
  sensor.autoCalibration(false);

  Wire.begin(5, 4);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) {
      Serial.println(F("SSD1306 allocation failed"));
      delay(100);
    }
  }
  display.clearDisplay();
  display.setTextSize(5);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.cp437(true);  // Use full 256 char 'Code Page 437' fontlay(100);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  debug_printf("co2: %d\n", sensor.getCO2(), sensor.getTemperature());

  display.clearDisplay();
  display.setCursor(0, 12);  // Start at top-left corner

  static char buf[16];
  snprintf(buf, 16, "%4d", sensor.getCO2());
  for (int i = 0; i < 4; i++) {
    display.write(buf[i]);
  }
  display.display();
  delay(100);
}
