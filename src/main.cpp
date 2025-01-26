// receiver for device 1

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>
#include <iostream>
#include <sstream>
#include <DFRobot_QMC5883.h>
#define SCK     5    // GPIO5  -- SCK
#define MISO    19   // GPIO19 -- MISO
#define MOSI    27   // GPIO27 -- MOSI
#define SS      18   // GPIO18 -- CS
#define RST     14   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    433E6
const String deviceKey = "killRusnya_2";

const float minusValueForAzimuth = 0;
const float plusValueForAzimuth = 0;

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
DFRobot_QMC5883 compass(&Wire, 0x1E);

// DFRobot_BMM150_I2C bmm150(&Wire, I2C_ADDRESS_4);

// void setInitMagData(){
//   sBmm150MagData_t magData = bmm150.getGeomagneticData();
//   initMagDataX = magData.x;
//   initMagDataY = magData.y;
//   initMagDataZ = magData.z;  
// }

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");

 // Initialize I2C with specific SCL and SDA pins
  Wire.begin(21, 22);

  // I2C Scanner
  Serial.println("Scanning for I2C devices...");
  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found\n");
  else Serial.println("done\n");

  SPI.begin();
  LoRa.setPins(SS,RST,DI0);


  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  pinMode(module1OutputPin, OUTPUT); 
  pinMode(module2OutputPin, OUTPUT); 
      // Initialize the display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  // while(bmm150.begin()){
  //   Serial.println("bmm150 init failed, Please try again!");
  //   delay(1000);
  // } Serial.println("bmm150 init success!");
  // bmm150.setOperationMode(BMM150_POWERMODE_NORMAL);
  // bmm150.setPresetMode(BMM150_PRESETMODE_HIGHACCURACY);
  // bmm150.setRate(BMM150_DATA_RATE_10HZ);

  /**!
   * Enable the measurement at x-axis, y-axis and z-axis, default to be enabled, no config required, the geomagnetic data at x, y and z will be incorrect when disabled.
   * Refer to setMeasurementXYZ() function in the .h file if you want to configure more parameters.
   */
  // bmm150.setMeasurementXYZ();
  // delay(1000);
  // setInitMagData();

  // Initialize the compass
  if (!compass.begin()) {
    Serial.println("Could not find a valid QMC5883 sensor, check wiring!");
    while (1);
  }
  sVector_t mag = compass.readRaw();
  initMagDataX = mag.XAxis;
  initMagDataY = mag.YAxis;
  initMagDataZ = mag.ZAxis; // Corrected to use ZAxis
  // compass.setRange(QMC5883_RANGE_8GA);
  // compass.setMeasurementMode(QMC5883_CONTINOUS);
  // compass.setDataRate(QMC5883_DATARATE_200HZ);
  // compass.setSamples(QMC5883_SAMPLES_2);
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
 void checkAzimut(){
  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
  compass.setDeclinationAngle(declinationAngle);
  sVector_t mag = compass.readRaw();
  double x = mag.XAxis;
  double y = mag.YAxis;
  double z = mag.ZAxis; // Corrected to use ZAxis
  azimut = mag.HeadingDegress; // Corrected to use getHeadingDegrees method
  Serial.print("mag x = "); Serial.print(x); Serial.println(" uT");
  Serial.print("mag y = "); Serial.print(y); Serial.println(" uT");
  Serial.print("mag z = "); Serial.print(z); Serial.println(" uT");

  // azimuth = azimuth - minusValueForAzimuth + plusValueForAzimuth;
  
  // Serial.print("the angle between the pointing direction and north (counterclockwise) is:");
  // Serial.println(azimut);
  // Serial.println("--------------------------------");

  // azimut = azimut - minusValueForAzimuth + plusValueForAzimuth;
  // Serial.print("corrected azimut is:");
  // Serial.println(azimut);
  // Serial.println("--------------------------------");
  delay(100);
 }

void checkMagData(){
  // float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
  // compass.setDeclinationAngle(declinationAngle);
  sVector_t mag = compass.readRaw();
  double x = mag.XAxis;
  double y = mag.YAxis;
  double z = mag.ZAxis; // Corrected to use ZAxis
  if (abs(x - initMagDataX) > 3 || abs(y - initMagDataY) > 3 || abs(z - initMagDataZ) > 3) {
    Serial.println("Magnetic data has changed significantly.");
    sendMagDataChanges();
    initMagDataX = x;
    initMagDataY = y;
    initMagDataZ = z;
  }
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
  counter++;
  if(counter - lastAzimutCheckin > 100){
    checkMagData();
    checkAzimut();
    sendAzimutCommand(azimut);
    lastAzimutCheckin = counter;
  }
  
}