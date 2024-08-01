// receiver for device 1

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define SCK     5    // GPIO5  -- SCK
#define MISO    19   // GPIO19 -- MISO
#define MOSI    27   // GPIO27 -- MOSI
#define SS      18   // GPIO18 -- CS
#define RST     14   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    433E6
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
String module1State = "disabledModule1";
String module2State = "disabledModule2";
String moduleName ="Vitalikiki M1";
String lastInputCommand = "";
String lastOutputCommand = "";


// for LED 
#define OLED_I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");
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
  display.display();
}
void sendCommand(String command) {
  Serial.print("Sending command: ");
  Serial.println(command);
  for (int x=0; x<3; x=x+1) {
  // Send the command to the LoRa module
    LoRa.beginPacket();
    LoRa.print(command);
    LoRa.endPacket();
    delay(50);
	}
  lastOutputCommand = command;
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

    if (receivedMessage.indexOf(enableModule1Command) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module1OutputPin, HIGH); 
      Serial.println("ON MODULE 1");
      module1State = isEnabledModule1Command;
      sendCommand(isEnabledModule1Command);
    }

    if (receivedMessage.indexOf(disableModule1Command) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module1OutputPin, LOW); 
      Serial.println("OFF MODULE 1");
      module1State = isDisabledModule1Command;
      sendCommand(isDisabledModule1Command);
    }

    if (receivedMessage.indexOf(enableModule2Command) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module2OutputPin, HIGH); 
      Serial.println("ON MODULE 2");
      module2State = isEnabledModule2Command;
      sendCommand(isEnabledModule2Command);
    }

    if (receivedMessage.indexOf(disableModule2Command) != -1) {
      lastInputCommand = receivedMessage;
      digitalWrite(module2OutputPin, LOW); 
      Serial.println("OFF MODULE 2");
      module2State = isDisabledModule2Command;
      sendCommand(isDisabledModule2Command);
    }

    if (receivedMessage.indexOf(getModulesStatesCommand) != -1) { 
      lastInputCommand = receivedMessage;
      Serial.println("Get modules states: ");
      Serial.println("module 1 state: " + module1State);
      Serial.println("module 2 state: " + module2State);
      delay(1000);
      sendCommand(module1State);
      delay(100);
      sendCommand(module2State);
    }
  }
}