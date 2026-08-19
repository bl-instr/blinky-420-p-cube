#define BLINKY_DIAG        0
#define CUBE_DIAG          0
#define COMM_LED_PIN       2
#define RST_BUTTON_PIN     3
#include <BlinkyPicoW.h>

struct CubeSetting
{
  uint16_t publishInterval;
  uint16_t nsamples;
};
CubeSetting setting;

struct CubeReading
{
  uint16_t signal1;
  uint16_t signal2;
  uint16_t bandWidth;
};
CubeReading reading;

unsigned long lastPublishTime;
float signal1;
float signal2;
int digCount;

void setupBlinky()
{
  if (BLINKY_DIAG > 0) Serial.begin(9600);

  BlinkyPicoW.setMqttKeepAlive(15);
  BlinkyPicoW.setMqttSocketTimeout(4);
  BlinkyPicoW.setMqttPort(1883);
  BlinkyPicoW.setMqttLedFlashMs(100);
  BlinkyPicoW.setHdwrWatchdogMs(8000);

  BlinkyPicoW.begin(BLINKY_DIAG, COMM_LED_PIN, RST_BUTTON_PIN, true, sizeof(setting), sizeof(reading));
}

void setupCube()
{
  if (CUBE_DIAG > 0) Serial.begin(9600);
  setting.publishInterval = 2000;
  lastPublishTime = millis();
  setting.nsamples = 2000;

  analogReadResolution(12);
  reading.signal1 = 0;
  reading.signal2 = 0;

  delay(100);
  signal1 = (float) analogRead(A0);
  signal2 = (float) analogRead(A1);
  digCount = 1;
}

void loopCube()
{
  unsigned long now = millis();
  signal1 = signal1 +(((float) analogRead(A0)) - signal1) / ((float) setting.nsamples);
  signal2 = signal2 +(((float) analogRead(A1)) - signal2) / ((float) setting.nsamples);
  ++digCount;

  if ((now - lastPublishTime) > setting.publishInterval)
  {
    float fbandwidth = 500.0 * ( ((float) digCount) / ((float) setting.publishInterval) ) / ((float) setting.nsamples);
    reading.bandWidth = (uint16_t) fbandwidth;   
    digCount = 0;
    
    reading.signal1 = (uint16_t) signal1;
    reading.signal2 = (uint16_t) signal2;
    if (CUBE_DIAG > 0)
    {
      Serial.print("Signals: ");
      Serial.print(reading.signal1);
      Serial.print(", ");
      Serial.println(reading.signal2);
    }
    
    lastPublishTime = now;
    boolean successful = BlinkyPicoW.publishCubeData((uint8_t*) &setting, (uint8_t*) &reading, false);
  }  
  boolean newSettings = BlinkyPicoW.retrieveCubeSetting((uint8_t*) &setting);
  if (newSettings)
  {
    if (setting.publishInterval < 1000) setting.publishInterval = 1000;
    if (setting.nsamples < 1) setting.nsamples = 1;
    signal1 = (float) analogRead(A0);
    signal2 = (float) analogRead(A1);
  }


}
