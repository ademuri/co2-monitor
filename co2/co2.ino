#include <Arduino.h>
#include <MHZ19.h>

MHZ19 sensor;

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

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  debug_printf("co2: %d\n", sensor.getCO2(), sensor.getTemperature());
  delay(100);
}
