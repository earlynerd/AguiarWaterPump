#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <BlynkSimpleEsp32.h>
#define BLYNK_DEVICE_NAME "Pump"

char auth[] = "ZzDsI62ylop97rvbHOaHS-F_J4jku4ZS";

const char* ssid = "Sly.fi";
const char* pass = "chiroptera";
WidgetTerminal terminal(V0);

//GPIO assignment - 13 pins
//G0 External thermistor (hot water temp)
//G1 onboard thermistor
//G4 LED Ring anode
//G5 LED Ring anode
//G6 pump pwm pin
//G7 pump power enable
//G8 pump tach pin
//G9 LED Ring anode
//G10 LED Ring anode
//G18 LED Ring anode
//G19 LED Ring anode
//G20 UART - programming
//G21 UART - programming
const int ledPins[] = {3, 19, 10, 5, 18, 4};
const int therm0 = 1;
const int therm1 = 0;
const int pump_PowerPin = 7;
const int pump_PwmPin = 6;
const int pump_TachPin = 8;

TaskHandle_t blynktask;
TaskHandle_t blynkPlots;

void blynkRun(void *pvParams);
void blynkUpdateChart(void *pvParams);

void ledControl(bool state);
void blinkNumber(unsigned long n);
void spinIncremental();

void setup() {
  Serial.begin(115200);

  for(int i = 0; i < 6; i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);
  }
  pinMode(therm0, INPUT);
  pinMode(therm1, INPUT);
  pinMode(pump_PowerPin, OUTPUT);
  digitalWrite(pump_PowerPin, LOW);
  pinMode(pump_PwmPin, OUTPUT);
  digitalWrite(pump_PwmPin, LOW);
  pinMode(pump_TachPin, INPUT_PULLUP);

  //WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
 // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.println("Connection Failed! Rebooting...");
    //delay(5000);
    //ESP.restart();
  //}
  Blynk.begin(auth, ssid, pass);
  MDNS.begin("waterPump.local");

    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.setTimeout(10000);
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(pump_PowerPin, HIGH);
  digitalWrite(pump_PwmPin, HIGH);

  xTaskCreate(
      blynkRun,
      "Blynk update",
      25000,
      NULL,
      23,
      &blynktask
      );

  xTaskCreate(
      blynkUpdateChart,
      "Chart and Gauge update",
      10000,
      NULL,
      24,
      &blynkPlots
      );

  unsigned long count = 0;
  unsigned long startCounterTIme = millis();
  bool tachState = false;
  while(millis() - startCounterTIme < 5000)
  {
    bool newTachState = digitalRead(pump_TachPin);
    if(newTachState && !tachState)
    {
      count++;
      spinIncremental();
    }
    
    tachState = newTachState;
  }
  digitalWrite(pump_PowerPin, LOW);
  digitalWrite(pump_PwmPin, LOW);
  terminal.print("tach pulses: ");
  terminal.println(count);
  terminal.flush();
  //blinkNumber(count);
}




      

void loop() {
  
  for(int i = 0; i < 6; i++)
  {
    digitalWrite(ledPins[i], HIGH);
    delay(30);
    digitalWrite(ledPins[i], LOW);
  }
  ArduinoOTA.handle();
}

void blynkRun(void *pvParams)
{
  for (;;)
  {
    Blynk.run();
    vTaskDelay(65);
  }
}


void blinkNumber(unsigned long n)
{
  const unsigned long period = 500;
  const unsigned long zeroPulse = 50;
  const unsigned long onePulse = 250;
  for(int i = 0; i < sizeof(n)*8; i++ )
  {
    ledControl(true);
    unsigned long pulseLength;
    if(n & (1<<i)) pulseLength = onePulse;
    else pulseLength = zeroPulse;
    delay(pulseLength);
    ledControl(false);
    delay(period - pulseLength);
  }

}

void ledControl(bool state)
{
  for(int i = 0; i < 6; i++)
  {
    digitalWrite(ledPins[i], state);
  }
}

void spinIncremental()
{
  static unsigned long counter = 0;
  for (int i = 0; i < 6; i++)
  {
    if(i == counter%6) digitalWrite(ledPins[i], HIGH);
    else digitalWrite(ledPins[i], LOW);
  }
  counter++;
}

void blynkUpdateChart(void *pvParams)
{
  for (;;)
  {
    Blynk.virtualWrite(V2, 0);
    vTaskDelay(65);
  }
}