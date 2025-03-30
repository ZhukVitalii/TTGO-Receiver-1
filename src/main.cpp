// receiver for device 1

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>
#include <iostream>
#include <sstream>
#include "MPU9250.h"
#define SCK     5    // GPIO5  -- SCK
#define MISO    19   // GPIO19 -- MISO
#define MOSI    27   // GPIO27 -- MOSI
#define SS      18   // GPIO18 -- CS
#define RST     14   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    433E6

// 18
#define STEP_PIN 12
// 19
#define DIR_PIN 13
// 21
#define ENABLE_PIN 15

const String deviceKey = "killRusnya_2";
float minusValueForAzimuth = 0;
float plusValueForAzimuth = 0;

const int module1OutputPin = 4;
const int module2OutputPin = 25;
const String enableModule1Command = "enableModule1";
const String disableModule1Command = "disableModule1";
const String isEnabledModule1Command = "enabledModule1";
const String isDisabledModule1Command = "disabledModule1";
const String getModulesStatesCommand = "getDeviceStates";

const String enableModule2Command = "enableModule2";
const String disableModule2Command = "disableModule2";
const String isEnabledModule2Command = "enabledModule2";
const String isDisabledModule2Command = "disabledModule2";
const String module1StateKey = "module1State";
const String module2StateKey = "module2State";
const String azimutDevice1Command = "azimutDevice1";
const String magDataChangedCommand = "magDataChanged";
const String correctAzimutPlusCommand = "correctAzimutPlus";
const String correctAzimutMinusCommand = "correctAzimutMinus";
const String turnRightCommand = "turnRight";
const String turnLeftCommand = "turnLeft";
String module1State = "disabledModule1";
String module2State = "disabledModule2";
String moduleName ="Vitalikiki M1";
String lastInputCommand = "";
String lastOutputCommand = "";
float azimut;
int counter = 0;
int lastAzimutCheckin = 0;

double initMagDataX = 0;
double initMagDataY = 0;
double initMagDataZ = 0;

// for LED 
#define OLED_I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

/**
 * @brief Creates an instance of the DFRobot_QMC5883 class to interface with the QMC5883L magnetometer sensor.
 * 
 * This object is used to communicate with the QMC5883L sensor over the I2C bus using the specified address.
 * 
 * @param Wire The I2C bus interface.
 * @param QMC5883L_ADDRESS The I2C address of the QMC5883L sensor.
 */
#define CS_PIN 2   // Chip Select (NCS)
#define SCK 14     // HSPI Clock (SCL)
#define MISO 12    // HSPI MISO (Not labeled on your board, but needed)
#define MOSI 13 


MPU9250 IMU(Wire,0x68);
int status;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");

  status = IMU.begin();
  
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }

  IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  // setting the gyroscope full scale range to +/-500 deg/s
  IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
  // setting DLPF bandwidth to 20 Hz
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  // setting SRD to 19 for a 50 Hz update rate
  IMU.setSrd(19);

  SPI.begin();
  LoRa.setPins(SS,RST,DI0);


  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  pinMode(module1OutputPin, OUTPUT); 
  pinMode(module2OutputPin, OUTPUT);


  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  digitalWrite(ENABLE_PIN, LOW); // Увімкнути драйвер
  digitalWrite(DIR_PIN, HIGH);

      // Initialize the display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds
}
void printInfo(){
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);
  display.println("Last received cmd : " + lastInputCommand);
  display.setCursor(0, 20);
  display.println("Last sent cmd : " + lastOutputCommand);
  display.setCursor(0, 40);  
  display.println(moduleName);
  display.setCursor(0, 50);  
  display.println(azimut);
  display.display();
}
void sendCommand(String command) {
  Serial.print("Sending command: ");
  Serial.println(command);
  for (int x=0; x<3; x=x+1) {
  // Send the command to the LoRa module
    LoRa.beginPacket();
    LoRa.print(command+deviceKey);
    LoRa.endPacket();
    delay(50);
	}
  lastOutputCommand = command;
}
void sendAzimutCommand(float azimut) {
  Serial.print("Sending azimut: ");
  Serial.println(azimut);
  for (int x=0; x<3; x=x+1) {
  // Send the command to the LoRa module
    LoRa.beginPacket();
    LoRa.print(azimutDevice1Command+deviceKey+";"+azimut);
    LoRa.endPacket();
    delay(50);
	}
  lastOutputCommand = azimutDevice1Command;
}

void sendMagDataChanges() {
  Serial.print("Sending magDataChanged commend: ");
  Serial.println(magDataChangedCommand);
  for (int x=0; x<3; x=x+1) {
  // Send the command to the LoRa module
    LoRa.beginPacket();
    LoRa.print(magDataChangedCommand);
    LoRa.endPacket();
    delay(50);
	}
  lastOutputCommand = magDataChangedCommand;
}

void turnLeft(int degrees) {
    const int stepsPerRevolution = 200; // 200 кроків = 360°
    const int microsteps = 8;  // Використовуємо 1/8 кроку
    float stepsPerDegree = (stepsPerRevolution * microsteps) / 360.0;
    int steps = round(degrees * stepsPerDegree);

    digitalWrite(DIR_PIN, LOW); // Обертання вліво

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(500);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(500);
    }
}

void turnRight(int degrees) {
    const int stepsPerRevolution = 200; // 200 кроків = 360°
    const int microsteps = 8;  // Використовуємо 1/8 кроку
    float stepsPerDegree = (stepsPerRevolution * microsteps) / 360.0;
    int steps = round(degrees * stepsPerDegree);

    digitalWrite(DIR_PIN, HIGH); // Обертання вліво

    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(500);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(500);
    }
}

 void checkAzimut(){
  IMU.readSensor();

  // display the data
  Serial.print(IMU.getAccelX_mss(),6);
  Serial.print("\t");
  Serial.print(IMU.getAccelY_mss(),6);
  Serial.print("\t");
  Serial.print(IMU.getAccelZ_mss(),6);
  Serial.print("\t");
  Serial.print(IMU.getGyroX_rads(),6);
  Serial.print("\t");
  Serial.print(IMU.getGyroY_rads(),6);
  Serial.print("\t");
  Serial.print(IMU.getGyroZ_rads(),6);
  Serial.print("\t");
  Serial.print(IMU.getMagX_uT(),6);
  Serial.print("\t");
  Serial.print(IMU.getMagY_uT(),6);
  Serial.print("\t");
  Serial.print(IMU.getMagZ_uT(),6);
  Serial.print("\t");
  Serial.println(IMU.getTemperature_C(),6);
// 1. Отримання значень
        float accelX = IMU.getAccelX_mss();
        float accelY = IMU.getAccelY_mss();
        float accelZ = IMU.getAccelZ_mss();
        
        float magX = IMU.getMagX_uT();
        float magY = IMU.getMagY_uT();
        float magZ = IMU.getMagZ_uT();

        // 2. Обчислення Roll і Pitch (у радіанах)
        float roll = atan2(accelY, accelZ);
        float pitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ));

        // 3. Коригування магнітометра (Tilt Compensation)
        float magX_corrected = magX * cos(pitch) + magZ * sin(pitch);
        float magY_corrected = magX * sin(roll) * sin(pitch) + magY * cos(roll) - magZ * sin(roll) * cos(pitch);

        // 4. Обчислення азимута (у градусах)
        float azimuth = atan2(magY_corrected, magX_corrected) * 180.0 / PI;
        if (azimuth < 0) {
            azimuth += 360.0;  // Нормалізація 0-360°
        }

        // Вивід даних
        Serial.print("Roll: "); Serial.print(roll * 180.0 / PI); Serial.print("°\t");
        Serial.print("Pitch: "); Serial.print(pitch * 180.0 / PI); Serial.print("°\t");
        Serial.print("Azimuth: "); Serial.print(azimuth); Serial.println("°");

        azimut = azimuth - minusValueForAzimuth + plusValueForAzimuth;
        Serial.print("Corrected azimut: "); Serial.print(azimut);

 }

void loop() {  
  printInfo();
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    String receivedMessage = "";
    // read packet
    while (LoRa.available()) {
       receivedMessage += (char)LoRa.read();
    }
    Serial.print(receivedMessage);
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    if (receivedMessage.indexOf(enableModule1Command+deviceKey) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module1OutputPin, HIGH); 
      Serial.println("ON MODULE 1");
      module1State = isEnabledModule1Command;
      sendCommand(isEnabledModule1Command);
    }

    if (receivedMessage.indexOf(disableModule1Command+deviceKey) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module1OutputPin, LOW); 
      Serial.println("OFF MODULE 1");
      module1State = isDisabledModule1Command;
      sendCommand(isDisabledModule1Command);
    }

    if (receivedMessage.indexOf(enableModule2Command+deviceKey) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module2OutputPin, HIGH); 
      Serial.println("ON MODULE 2");
      module2State = isEnabledModule2Command;
      sendCommand(isEnabledModule2Command);
    }

    if (receivedMessage.indexOf(disableModule2Command+deviceKey) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module2OutputPin, LOW); 
      Serial.println("OFF MODULE 2");
      module2State = isDisabledModule2Command;
      sendCommand(isDisabledModule2Command);
    }

    if (receivedMessage.indexOf(correctAzimutPlusCommand) != -1) {
      lastInputCommand = receivedMessage;
      int separatorIndex = receivedMessage.indexOf(':');
      if (separatorIndex != -1) {
        String valueStr = receivedMessage.substring(separatorIndex + 1);
        float plusValue = valueStr.toFloat();
        plusValueForAzimuth = plusValue;
      }
    }
    if (receivedMessage.indexOf(correctAzimutMinusCommand) != -1) {
      lastInputCommand = receivedMessage;
      int separatorIndex = receivedMessage.indexOf(':');
      if (separatorIndex != -1) {
        String valueStr = receivedMessage.substring(separatorIndex + 1);
        float minusValue = valueStr.toFloat();
        minusValueForAzimuth = minusValue;
      }
    }
    
    if (receivedMessage.indexOf(turnRightCommand) != -1) {
      lastInputCommand = receivedMessage;
      int separatorIndex = receivedMessage.indexOf(':');
      if (separatorIndex != -1) {
        String valueStr = receivedMessage.substring(separatorIndex + 1);
        int value = valueStr.toInt();
        turnRight(value);
      }
    }
    
    if (receivedMessage.indexOf(turnLeftCommand) != -1) {
      lastInputCommand = receivedMessage;
      int separatorIndex = receivedMessage.indexOf(':');
      if (separatorIndex != -1) {
        String valueStr = receivedMessage.substring(separatorIndex + 1);
        int value = valueStr.toInt();
        turnLeft(value);
      }
    }
    

    if (receivedMessage.indexOf(getModulesStatesCommand+deviceKey) != -1) { 
      lastInputCommand = receivedMessage;
      Serial.println("Get modules states: ");
      Serial.println("module 1 state: " + module1State);
      Serial.println("module 2 state: " + module2State);
      delay(1000);
      sendCommand(module1State);
      delay(100);
      sendCommand(module2State);
      checkAzimut();
      sendAzimutCommand(azimut);
    }
  }
  // counter++;
  // if(counter - lastAzimutCheckin > 200){
    // checkMagData();
    // checkAzimut();
    // sendAzimutCommand(azimut);
    // lastAzimutCheckin = counter;
  // }

}