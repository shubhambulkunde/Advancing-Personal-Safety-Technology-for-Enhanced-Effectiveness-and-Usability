#include <SoftwareSerial.h>
#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPSPlus.h>

//unsigned long prevMillis = 0;
const int switchPin = 9;

void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

class Sensor {
public:
    //int sensorID;
    //String sensorType;
    virtual void readData();
};

class HealthMonitor : public Sensor {
private:
    int heartRate=0;
    MAX30105 particleSensor;
public:
    HealthMonitor(){
      if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
      {
        //MAX30105 was not found. Please check wiring/power.
        while (1);
      }

      byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
      byte sampleAverage = 8; //Options: 1, 2, 4, 8, 16, 32
      byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
      int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
      int pulseWidth = 411; //Options: 69, 118, 215, 411
      int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

      particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
    }
    void readData(){
        heartRate=particleSensor.getIR();
    }
    int getHealthData(){ 
      return heartRate;
    };
};

class AccelerometerSensor : public Sensor {
private:
    bool fall=false;
    Adafruit_MPU6050 mpu;

public:
    AccelerometerSensor(){
      mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
      mpu.setGyroRange(MPU6050_RANGE_500_DEG);
      mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
      delay(100);  
    }
    void readData(){
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      fall=(a.acceleration.x<3 && (abs(a.acceleration.z)>=8.5 || abs(a.acceleration.y)>=8.5));
    }
    bool detectFall() {
      return fall;
    }
};

class GPSSensor : public Sensor {
private:
    // A sample NMEA stream.
    const char *gpsStream =
      "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
      "$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n"
      "$GPRMC,045200.000,A,3014.3820,N,09748.9514,W,36.88,65.02,030913,,,A*77\r\n"
      "$GPGGA,045201.000,3014.3864,N,09748.9411,W,1,10,1.2,200.8,M,-22.5,M,,0000*6C\r\n"
      "$GPRMC,045251.000,A,3014.4275,N,09749.0626,W,0.51,217.94,030913,,,A*7D\r\n"
      "$GPGGA,045252.000,3014.4273,N,09749.0628,W,1,09,1.3,206.9,M,-22.5,M,,0000*6F\r\n";

    // The TinyGPSPlus object
    TinyGPSPlus gps;
    double latitude;
    double longitude;
public:
    GPSSensor(){
    }
    double getLatitude(){
      return latitude;
    }
    
    double getLongitude(){
      return longitude;
    }
    void readData() {
      if (gps.location.isValid())
      {
        latitude=gps.location.lat();
        longitude=gps.location.lng();
      }
    }
};

HealthMonitor healthMonitorObject;
AccelerometerSensor accelerometerObject;
GPSSensor gpsObject;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(switchPin, INPUT);
}

void loop() {
  TCA9548A(1);
  healthMonitorObject.readData();
  TCA9548A(2);
  accelerometerObject.readData();
  gpsObject.readData();

  String sendStr="i";
  sendStr+= String(healthMonitorObject.getHealthData())+".";
  sendStr+= String(accelerometerObject.detectFall()?1:0)+".";
  sendStr+= String(digitalRead(switchPin));
  //s1532.0.0
  Serial.println(sendStr);
  delay(200);

  sendStr="g";
  sendStr+=String(gpsObject.getLatitude())+",";
  sendStr+=String(gpsObject.getLongitude());

  Serial.println(sendStr);
  delay(200);

}
