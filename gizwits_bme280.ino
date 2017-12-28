#include <cy_wunderg.h>
#include "cy_wunderg_cfg.h"

#include <Ticker.h>
#include "cy_wifi.h"
#include "cy_ota.h"
#include "cy_weather.h"
#include "cy_mqtt.h"


#include "btn_led_tool.h"


#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const char* gv_hostname = "gizbme280";
const char* mqtt_pubtopic = "ATSH28/AUSSEN/TEMP/1/value";

Adafruit_BME280 bme; // I2C
float gv_humidity;
float gv_temp;
float gv_press;

Ticker senstick;
boolean gv_senstick;

cy_wunderg WUnderg( wunderg_host, wunderg_sid, wunderg_pwd);

void do_senstick() {
  gv_senstick = true;
}

//********************************************************************************************************
void setup() {
  // put your setup code here, to run once:

#ifdef serdebug
  Serial.begin(115200);
#endif

  DebugPrintln("\n" + String(__DATE__) + ", " + String(__TIME__) + " " + String(__FILE__));

  set_rgb(255, 255, 255);
  delay(500);


  wifi_init(gv_hostname);
  delay(500);

  init_ota(gv_hostname);

  init_mqtt(gv_hostname);
  //add_subtopic("ATSH28/KE/ALT/TEMP/....", callback_mqtt1);
  check_mqtt();

  set_rgb(0, 0, 0);
  delay(500);

  Wire.begin(14, 5);
  bool status = bme.begin(0x76, &Wire);
  if (!status) {
    DebugPrintln("Could not find a valid BME280 sensor, check wiring!");
  }
  delay(500);

  do_sensor();
  gv_senstick = false;
  senstick.attach(60, do_senstick);

  //setup button
  pinMode(btnpin, INPUT);
  attachInterrupt(btnpin, IntBtn, CHANGE);
  delay(500);
}


//********************************************************************************************************
void loop() {
  // put your main code here, to run repeatedly:

  check_ota();

  check_mqtt();

  if (gv_senstick == true) {

    do_sensor();

    gv_senstick = false;
  }

  LDRValue = analogRead(LDRPin);

  check_btn();

  set_leds();

  delay(100);

}

//********************************************************************************************************

void do_sensor() {
  set_rgb(0, 255, 0);
  get_bme280();

  send_val(1, gv_temp);
  WUnderg.send_temp_c(gv_temp);
  delay(100);
  char buffer[10];
  dtostrf(gv_temp, 0, 1, buffer);
  client.publish(mqtt_pubtopic, buffer, true);

  send_val(22, gv_humidity);
  WUnderg.send_hum(gv_humidity);

  send_val(6, gv_press, false);

  set_rgb(0, 0, 0);

}

void get_bme280() {
  float lv_humidity;
  float lv_temp;
  float lv_press;

  lv_temp = (int)(bme.readTemperature() * 10);
  gv_temp = lv_temp / 10;
  DebugPrint("Temperature: ");
  DebugPrint(gv_temp);
  DebugPrint(" *C ");

  gv_press = bme.readPressure() / 100;
  DebugPrint("Pressure = ");
  DebugPrint(gv_press / 100.0F);
  DebugPrintln(" hPa");

  //Serial.print("Approx. Altitude = ");
  //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  //Serial.println(" m");

  lv_humidity = (int)(bme.readHumidity() * 10);
  gv_humidity = lv_humidity / 10;
  DebugPrint("Humidity: ");
  DebugPrint(gv_humidity);
  DebugPrint(" % ");
  DebugPrintln();
}

