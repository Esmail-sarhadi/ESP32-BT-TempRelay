#include "BluetoothSerial.h"
#include <DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Check if Bluetooth configs are enabled.
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//Bluetooth Serial object.
BluetoothSerial SerialBT;
char *pin = "6622";

//GPIO where LED is connected to.
int pinRelay = 5;

//Handle received and sent messages.
String incomingMessage = "";
char incomingChar;
String outcomingMessage = "";
char outcomingChar;
String temphumidString = "";

//Timer: auxiliar variables
unsigned long previousMillis = 0;    //Stores last time temperature was published.
const long interval = 5000;          //Interval at which to publish sensor readings.
//********************************************************************************************
void setup() {
  pinMode(pinRelay, OUTPUT);
  Serial.begin(115200);
  dht.begin();

  //Bluetooth device name.
  SerialBT.begin("MyESP32");
  SerialBT.setPin(pin);
  Serial.println(">> The device started, now you can pair it with bluetooth!");
}
//********************************************************************************************
void loop() {
  unsigned long currentMillis = millis();
  //Send temperature readings.
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    temphumidString = "Temperature: " + String(dht.readTemperature()) + "C  " + "Humidity: " +  String(dht.readHumidity()) + "%";
    Serial.println(temphumidString);
    SerialBT.println(temphumidString);
  }

  //Read received messages (Relay control command).
  if (SerialBT.available()) {
    incomingChar = SerialBT.read();
    if (incomingChar != '\n') {
      incomingMessage += String(incomingChar);
    }
    else {
      incomingMessage = "";
    }
    Serial.write(incomingChar);
  }

  //Check received message and control output accordingly.
  if (incomingMessage == "Relay_on") {
    digitalWrite(pinRelay, HIGH);
  }
  else if (incomingMessage == "Relay_off") {
    digitalWrite(pinRelay, LOW);
  }

  //Send messages.
  if (Serial.available()) {
    outcomingChar = Serial.read();
    if (outcomingChar != '\n' && outcomingChar != '\r') {
      outcomingMessage += String(outcomingChar);
    }
    else {
      SerialBT.println(outcomingMessage);
      outcomingMessage = "";
    }
  }
  delay(20);
}
//********************************************************************************************
