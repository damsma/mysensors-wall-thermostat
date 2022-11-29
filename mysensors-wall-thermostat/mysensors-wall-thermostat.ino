#include <Arduino.h>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// ROOM SELECTION
#include "livingroom.h"
//#include "bedroom.h"
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Enable debug prints to serial monitor
#define MY_DEBUG

// When ENC28J60 is connected we have to move CE/CSN pins for NRF radio
#define MY_RF24_CE_PIN 0
#define MY_RF24_CS_PIN 0

// The port to keep open on node server mode / or port to contact in client mode
#define MY_PORT 8888

#include <U8x8lib.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MySensors.h>
#include <Wire.h>
#include "OneButton.h"
#include "SHTSensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS1 13
#define ONE_WIRE_BUS2 11
#define ONE_WIRE_BUS3 9
#define ONE_WIRE_BUS4 7
#define BUTTON_UP_PIN 17
#define BUTTON_DOWN_PIN 15
#define BUTTON_MENU_PIN 14
#define BUTTON_OK_PIN 16

#define CHILD_ID_HVAC 11

U8X8_SSD1327_MIDAS_128X128_4W_SW_SPI u8x8(/* clock=*/ 2, /* data=*/ 3, /* cs=*/ 4, /* dc=*/ 5, /* reset=*/ 6);

SHTSensor sht(SHTSensor::SHT3X);
OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);
OneWire oneWire3(ONE_WIRE_BUS3);
OneWire oneWire4(ONE_WIRE_BUS4);
DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);
DallasTemperature sensors4(&oneWire4);

// Initialize humidity message
MyMessage msg_humidity(1,V_HUM);

MyMessage msg_temp(2,V_TEMP);

MyMessage msg_temp_floor1(3,V_TEMP);
MyMessage msg_temp_floor2(4,V_TEMP);
MyMessage msg_temp_floor3(5,V_TEMP);
MyMessage msg_temp_floor4(6,V_TEMP);

//MyMessage textMsg(10, V_TEXT);  // message for Sending Text to Controller

// Initialize thermostat message
MyMessage msg_setpoint(CHILD_ID_HVAC, V_HVAC_SETPOINT_HEAT);
MyMessage msg_setspeed(CHILD_ID_HVAC, V_HVAC_SPEED);
MyMessage msg_setflowstate(CHILD_ID_HVAC, V_HVAC_FLOW_STATE);
MyMessage msg_acttemp(CHILD_ID_HVAC, V_TEMP);

// Setup a new OneButton on pin BUTTON_UP_PIN.  
OneButton buttonUP(BUTTON_UP_PIN, true);
// Setup a new OneButton on pin BUTTON_DOWN_PIN.  
OneButton buttonDOWN(BUTTON_DOWN_PIN, true);
// Setup a new OneButton on pin BUTTON_MENU_PIN.  
OneButton buttonMENU(BUTTON_MENU_PIN, true);
// Setup a new OneButton on pin BUTTON_OK_PIN.  
OneButton buttonOK(BUTTON_OK_PIN, true);

float wantedTemperature = 21.5;
float actualTemperature = 99.9;
float actualTemperatureFloor1 = 99.9;
float actualTemperatureFloor2 = 99.9;
float actualTemperatureFloor3 = 99.9;
float actualTemperatureFloor4 = 99.9;
float actualHumidity = 99.9;
//String newMessage;          // next message to be displayed if available
bool initialValueSent = false;

//Some global variables to hold the text states sent to the home assistant controller
String FAN_STATE_TXT = "Auto";  // possible values ("Min", "Normal", "Max", "Auto")
String MODE_STATE_TXT = "AutoChangeOver"; // possible values ("Off", "HeatOn", "CoolOn", or "AutoChangeOver")

long previousMillisTemperature = 0;
long previousMillisOutTemperature = 0;
long previousMillisDisplay = 0;
long previousMillisDisplayTurnedOff = 0;
long previousMillisButtons = 0;

long startMillisButtonUP = 0;
long startMillisButtonDOWN = 0;

long tempReadInterval = 1000;
long tempReadOutInterval = 5000;
long displayRefreshInterval = 200;
long displayTurnOffInterval = 50000;
long buttonReadInterval = 20;

int showScreen = 1;

//DS18b20
int  resolution = 12;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
DeviceAddress tempDeviceAddress;
//DS18b20

// TEMP
char cstr[1];
char buff[4];
// TEMP

void setup(void)
{
  u8x8.begin();
  u8x8.setPowerSave(0);
  
  Serial.begin(115200);

  Wire.begin();
  delay(1000); // let serial console settle

  if (sht.init()) {
      Serial.print("init(): success\n");
  } else {
      Serial.print("init(): failed\n");
  }
  sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); // only supported by SHT3x

  
  //dS18b20
  sensors1.begin();
  sensors1.getAddress(tempDeviceAddress, 0);
  sensors1.setResolution(tempDeviceAddress, resolution);

  sensors1.setWaitForConversion(false);
  sensors1.requestTemperatures();
  
  sensors2.begin();
  sensors2.getAddress(tempDeviceAddress, 0);
  sensors2.setResolution(tempDeviceAddress, resolution);

  sensors2.setWaitForConversion(false);
  sensors2.requestTemperatures();
  
  sensors3.begin();
  sensors3.getAddress(tempDeviceAddress, 0);
  sensors3.setResolution(tempDeviceAddress, resolution);

  sensors3.setWaitForConversion(false);
  sensors3.requestTemperatures();

  sensors4.begin();
  sensors4.getAddress(tempDeviceAddress, 0);
  sensors4.setResolution(tempDeviceAddress, resolution);

  sensors4.setWaitForConversion(false);
  sensors4.requestTemperatures();
  delayInMillis = 1750 / (1 << (12 - resolution));
  lastTempRequest = millis();

  // Setup the buttons

  // link the button UP functions.
  buttonUP.attachClick(clickUP);
  buttonUP.attachLongPressStart(longPressUPStart);
  buttonUP.attachLongPressStop(longPressUPStop);
  buttonUP.attachDuringLongPress(longPressUP);
  //buttonUP.attachDoubleClick(doubleclickUP);
  
  // link the button DOWN functions.
  buttonDOWN.attachClick(clickDOWN);
  buttonDOWN.attachLongPressStart(longPressDOWNStart);
  buttonDOWN.attachLongPressStop(longPressDOWNStop);
  buttonDOWN.attachDuringLongPress(longPressDOWN);
  //buttonDOWN.attachDoubleClick(doubleclick1);

  // link the button MENU functions.
  buttonMENU.attachClick(clickMENU);
  buttonMENU.attachLongPressStart(longPressMENUStart);
  buttonMENU.attachLongPressStop(longPressMENUStop);
  buttonMENU.attachDuringLongPress(longPressMENU);
  //buttonMENU.attachDoubleClick(doubleclick1);

  // link the button OK functions.
  buttonOK.attachClick(clickOK);
  buttonOK.attachLongPressStart(longPressOKStart);
  buttonOK.attachLongPressStop(longPressOKStop);
  buttonOK.attachDuringLongPress(longPressOK);
  //buttonOK.attachDoubleClick(doubleclick1);

  presentation();
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  String sNameSketch = sRoomName+" OLED thermostat | github.com/damsma";
  sendSketchInfo(sNameSketch.c_str(), "0.5");

  // Present all sensors to controller
  String sNameChild = sRoomName+" humidity thermostat";
  present(1, S_HUM, sNameChild.c_str());

  sNameChild = sRoomName+" temperature thermostat";
  present(2, S_TEMP, sNameChild.c_str());

  sNameChild = sRoomName+" temperature floor 1";
  present(3, S_TEMP, sNameChild.c_str());

  sNameChild = sRoomName+" temperature floor 2";
  present(4, S_TEMP, sNameChild.c_str());

  sNameChild = sRoomName+" temperature floor 3";
  present(5, S_TEMP, sNameChild.c_str());

  sNameChild = sRoomName+" temperature floor 4";
  present(6, S_TEMP, sNameChild.c_str());

  //sNameChild = sRoomName+" Displaytext";
  //present(10, S_INFO, sNameChild.c_str());
  
  sNameChild = sRoomName+" thermostat";
  present(CHILD_ID_HVAC, S_HVAC, sNameChild.c_str());
}

void loop(void)
{
  unsigned long currentMillis = millis();

  if (!initialValueSent) {
    Serial.println("Sending initial value");

    send(msg_setpoint.set(wantedTemperature, 1));
    send(msg_setspeed.set(FAN_STATE_TXT.c_str()));
    send(msg_setflowstate.set(MODE_STATE_TXT.c_str()));

    //newMessage = "";
    //send(textMsg.set(newMessage));
    // Serial.println("Requesting initial value from controller");
    // request(10, V_TEXT);
    // wait(2000, C_SET, V_TEXT);

    initialValueSent = true;
  }
  
  if(currentMillis - previousMillisButtons > buttonReadInterval) {
    previousMillisButtons = currentMillis;
    buttonUP.tick();
    buttonDOWN.tick();
    buttonMENU.tick();
    buttonOK.tick();
    //Serial.print("button tick ");
  }

  /** READ TEMPERATURE **/
  if(currentMillis - previousMillisTemperature > tempReadInterval) {
    previousMillisTemperature = currentMillis;
    readTemp();
    //Serial.print("readTemp ");
  }
  
  /** READ OUTSIDE TEMPERATURE FROM DOMOTICZ **/
  if(currentMillis - previousMillisOutTemperature > tempReadOutInterval) {
    previousMillisOutTemperature = currentMillis;
    //Serial.print("request V_TEXT\n");
    request(10, V_TEXT, 0);          // request value from controller
    //Serial.print("request V_TEXT OK\n");
  }
  
  /** REFRESH DISPLAY **/
  if(currentMillis - previousMillisDisplay > displayRefreshInterval) {
    previousMillisDisplay = currentMillis;   
    refreshDisplay();
    //Serial.print("refreshDisplay ");
  }

  /** DISPLAY OFF**/
  if(currentMillis - previousMillisDisplayTurnedOff > displayTurnOffInterval) {
    changeDisplay(0);
//    Serial.print("\nDISPLAY OFF ");
  }

}

void receive(const MyMessage &message) {
  Serial.print("Recieved val: "); Serial.println(message.data); Serial.print("\n"); 
  String recvData = message.data;
  recvData.trim();
  
  if (message.type==V_HVAC_SETPOINT_HEAT) {
    wantedTemperature = atof(message.data);
    
    changeDisplay(1);
  }
  if (message.type==V_HVAC_SPEED) {
    FAN_STATE_TXT = recvData;

    changeDisplay(1);
  }
  if (message.type==V_HVAC_FLOW_STATE) {
    MODE_STATE_TXT = recvData;

    changeDisplay(1);

    if(showScreen == 1) {
      clear_info();
    }
  }
  if (message.type==V_TEXT) {
    //strcpy(newMessage, message.getString());    // copy it in
    //newMessage = message.getString();    // copy it in
  } 
}

/** ----------- **/
/** SENSOR READ **/
/** ----------- **/
void readTemp()
{
  //Serial.print("Reading SHT30 Data...\n");
  //Serial.print("SHT:\n");
  //Serial.print("  RH: ");

  if (sht.readSample()) {
    actualHumidity = sht.getHumidity();
    //Serial.print(actualHumidity, 2);
    //Serial.print("\n");
    //Serial.print("  T:  ");
    actualTemperature = sht.getTemperature();
    //Serial.print(actualTemperature, 2);
    //Serial.print("\n");
    //Serial.print("Reading SHT30 Data...OK\n");

    if (! isnan(actualHumidity)) {  // check if 'is not a number'
      send(msg_humidity.setSensor(1).set(actualHumidity, 1));
    //Serial.print("sent HUMIDITY...OK\n");
    }
  
    if (! isnan(actualTemperature)) {  // check if 'is not a number'
      send(msg_temp.setSensor(2).set(actualTemperature, 1));
      send(msg_acttemp.setSensor(CHILD_ID_HVAC).set(actualTemperature, 1));
    //Serial.print("sent TEMP...OK\n");
    }
  } else {
      Serial.print("Error in readSample()\n");
  }

  //DS18b20
  if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
  {
    //Serial.print("TEMP DS18b20: ");
    actualTemperatureFloor1 = sensors1.getTempCByIndex(0);
    actualTemperatureFloor2 = sensors2.getTempCByIndex(0);
    actualTemperatureFloor3 = sensors3.getTempCByIndex(0);
    actualTemperatureFloor4 = sensors4.getTempCByIndex(0);
    if(actualTemperatureFloor1 < -50)
    {
      actualTemperatureFloor1 = 99;
    }
    //Serial.print(actualTemperatureFloor1);
    //Serial.println(" °C actualTemperatureFloor1");
    if(actualTemperatureFloor2 < -50)
    {
      actualTemperatureFloor2 = 99;
    }
    //Serial.print(actualTemperatureFloor2);
    //Serial.println(" °C actualTemperatureFloor2");
    if(actualTemperatureFloor3 < -50)
    {
      actualTemperatureFloor3 = 99;
    }
    //Serial.print(actualTemperatureFloor3);
    //Serial.println(" °C actualTemperatureFloor3");
    if(actualTemperatureFloor4 < -50)
    {
      actualTemperatureFloor4 = 99;
    }
    //Serial.print(actualTemperatureFloor4);
    //Serial.println(" °C actualTemperatureFloor4");
    
    sensors1.requestTemperatures(); 
    sensors2.requestTemperatures(); 
    sensors3.requestTemperatures(); 
    sensors4.requestTemperatures(); 
    delayInMillis = 1750 / (1 << (12 - resolution));
    lastTempRequest = millis();

    if (! isnan(actualTemperatureFloor1)) {  // check if 'is not a number'
      send(msg_temp_floor1.setSensor(3).set(actualTemperatureFloor1, 1));
    }
    if (! isnan(actualTemperatureFloor2)) {  // check if 'is not a number'
      send(msg_temp_floor2.setSensor(4).set(actualTemperatureFloor2, 1));
    }
    if (! isnan(actualTemperatureFloor3)) {  // check if 'is not a number'
      send(msg_temp_floor3.setSensor(5).set(actualTemperatureFloor3, 1));
    }
    if (! isnan(actualTemperatureFloor4)) {  // check if 'is not a number'
      send(msg_temp_floor4.setSensor(6).set(actualTemperatureFloor4, 1));
    }
  }  

  send(msg_setpoint.set(wantedTemperature, 1));
  send(msg_setspeed.set(FAN_STATE_TXT.c_str()));
  send(msg_setflowstate.set(MODE_STATE_TXT.c_str()));
}

/** ------- **/
/** DISPLAY **/
/** ------- **/
void refreshDisplay() {
  //Serial.print("Refreshing display...\n");

  switch(showScreen) {
    case 0:
      showDefault();
      break;
    case 1:
      showSmall();
      break;
    case 2:
      showBig();
      break;
    default:
      showDefault();
      break;
  }
}

void changeDisplay(int setTo) {
  //Serial.println("\nchangeDisplay to ");
  //Serial.println(setTo);
  if(showScreen != setTo) {
    showScreen = setTo;
    u8x8.clear();
    if(setTo != 0) {
      unsigned long currentMillis = millis();
      previousMillisDisplayTurnedOff = currentMillis-200; //200ms reserve damit nicht wieder auf 0 gesetzt wird in loop()
    }
  }
}

void showDefault() {
}

void showBig() {
  u8x8.noInverse();
  u8x8.setFont(u8x8_font_pressstart2p_f);
  
  dtostrf(wantedTemperature, 2, 3, buff);

  if(wantedTemperature <= 9.95)
  {
    cstr[1] = buff[0];
    cstr[0] = ' ';
  }
  else {
    cstr[0] = buff[0];
    cstr[1] = buff[1];
  }
  u8x8.draw2x2String(5, 8, cstr);

  cstr[1] = '°';
  if(wantedTemperature <= 9.95)
  {
    cstr[0] = buff[2];
  }
  else {
    cstr[0] = buff[3];
  }
  u8x8.drawUTF8(9, 8, ".");
  u8x8.drawUTF8(10, 8, cstr);


  dtostrf(actualTemperature, 2, 1, buff);
  u8x8.drawUTF8(0, 15, buff);
  u8x8.drawUTF8(4, 15, "°");

  dtostrf(actualHumidity, 2, 1, buff);
  u8x8.drawUTF8(11, 15, buff);
  u8x8.drawUTF8(15, 15, "%");
}

void showSmall() {
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);    
  u8x8.inverse();
  
  u8x8.setCursor(0,0);
  u8x8.print(" Akt.  ");

  u8x8.setCursor(9,0);
  u8x8.print("  Ust. ");

  u8x8.noInverse();
  u8x8.setFont(u8x8_font_pressstart2p_f);
  
  dtostrf(wantedTemperature, 2, 3, buff);

  if(wantedTemperature <= 9.95)
  {
    cstr[1] = buff[0];
    cstr[0] = ' ';
  }
  else {
    cstr[0] = buff[0];
    cstr[1] = buff[1];
  }
  u8x8.draw2x2String(9, 2, cstr);

  cstr[1] = '°';
  if(wantedTemperature <= 9.95)
  {
    cstr[0] = buff[2];
  }
  else {
    cstr[0] = buff[3];
  }
    
  u8x8.drawUTF8(13, 2, ".");
  u8x8.drawUTF8(14, 2, cstr);

  if (! isnan(actualTemperature)) {  // check if 'is not a number'
    dtostrf(actualTemperature, 2, 3, buff);
  
    if(actualTemperature <= 9.95)
    {
      cstr[1] = buff[0];
      cstr[0] = ' ';
    }
    else {
      cstr[0] = buff[0];
      cstr[1] = buff[1];
    }
      u8x8.draw2x2String(0, 2, cstr);
  
    cstr[1] = '°';
    if(actualTemperature <= 9.95)
    {
      cstr[0] = buff[2];
    }
    else {
      cstr[0] = buff[3];
    }
    
    u8x8.drawUTF8(4, 2, ".");
    u8x8.drawUTF8(5, 2, cstr);
    
    //Serial.print("Temp *C = "); Serial.println(actualTemperature);
  } else { 
    //Serial.println("Failed to read temperature");
  }
  
  if (! isnan(actualHumidity)) {  // check if 'is not a number'
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);    
    u8x8.inverse();
    
    u8x8.drawUTF8(0, 5, " Wilg. ");

    u8x8.noInverse();
    
    dtostrf(actualHumidity, 2, 1, buff);
    u8x8.drawUTF8(9, 5, buff);
    u8x8.drawUTF8(13, 5, "%  ");
    //Serial.print("Hum. % = "); Serial.println(actualHumidity);
  } else { 
    //Serial.println("Failed to read humidity");
  }
  //Serial.println();
  
  u8x8.setFont(u8x8_font_pressstart2p_f);
  u8x8.setCursor(0,9);
  u8x8.inverse();
  u8x8.print(" Temp. floor ");
  u8x8.noInverse();

  if (! isnan(actualTemperatureFloor1)) {  // check if 'is not a number'
    dtostrf(actualTemperatureFloor1, 2, 3, buff);
  
    if(actualTemperatureFloor1 <= 9.95)
    {
      cstr[1] = buff[0];
      cstr[0] = ' ';
    }
    else {
      cstr[0] = buff[0];
      cstr[1] = buff[1];
    }
    u8x8.draw2x2String(0, 11, cstr);

    cstr[1] = '°';
    if(actualTemperatureFloor1 <= 9.95)
    {
      cstr[0] = buff[2];
    }
    else {
      cstr[0] = buff[3];
    }
    
    u8x8.drawUTF8(4, 11, ".");
    u8x8.drawUTF8(5, 11, cstr);

    //Serial.print("Temp actualTemperatureFloor1 *C = "); Serial.println(actualTemperatureFloor1);
  } else { 
    //Serial.println("Failed to read temperature floor 1");
  }

  if (! isnan(actualTemperatureFloor2)) {  // check if 'is not a number'
    dtostrf(actualTemperatureFloor2, 2, 3, buff);
  
    if(actualTemperatureFloor2 <= 9.95)
    {
      cstr[1] = buff[0];
      cstr[0] = ' ';
    }
    else {
      cstr[0] = buff[0];
      cstr[1] = buff[1];
    }
    u8x8.draw2x2String(9, 11, cstr);

    cstr[1] = '°';
    if(actualTemperatureFloor2 <= 9.95)
    {
      cstr[0] = buff[2];
    }
    else {
      cstr[0] = buff[3];
    }
    
    u8x8.drawUTF8(13, 11, ".");
    u8x8.drawUTF8(14, 11, cstr);

    //Serial.print("Temp actualTemperatureFloor2 *C = "); Serial.println(actualTemperatureFloor2);
  } else { 
    //Serial.println("Failed to read temperature floor 2");
  }

  if (! isnan(actualTemperatureFloor3)) {  // check if 'is not a number'
    dtostrf(actualTemperatureFloor3, 2, 3, buff);
  
    if(actualTemperatureFloor3 <= 9.95)
    {
      cstr[1] = buff[0];
      cstr[0] = ' ';
    }
    else {
      cstr[0] = buff[0];
      cstr[1] = buff[1];
    }
    u8x8.draw2x2String(0, 14, cstr);

    cstr[1] = '°';
    if(actualTemperatureFloor3 <= 9.95)
    {
      cstr[0] = buff[2];
    }
    else {
      cstr[0] = buff[3];
    }
    
    u8x8.drawUTF8(4, 14, ".");
    u8x8.drawUTF8(5, 14, cstr);

    //Serial.print("Temp actualTemperatureFloor3 *C = "); Serial.println(actualTemperatureFloor3);
  } else { 
    //Serial.println("Failed to read temperature floor 3");
  }

  if (! isnan(actualTemperatureFloor4)) {  // check if 'is not a number'
    dtostrf(actualTemperatureFloor4, 2, 3, buff);
  
    if(actualTemperatureFloor4 <= 9.95)
    {
      cstr[1] = buff[0];
      cstr[0] = ' ';
    }
    else {
      cstr[0] = buff[0];
      cstr[1] = buff[1];
    }
    u8x8.draw2x2String(9, 14, cstr);

    cstr[1] = '°';
    if(actualTemperatureFloor4 <= 9.95)
    {
      cstr[0] = buff[2];
    }
    else {
      cstr[0] = buff[3];
    }
    
    u8x8.drawUTF8(13, 14, ".");
    u8x8.drawUTF8(14, 14, cstr);

    //Serial.print("Temp actualTemperatureFloor4 *C = "); Serial.println(actualTemperatureFloor4);
  } else { 
    //Serial.println("Failed to read temperature floor 4");
  }

  u8x8.noInverse();
  u8x8.setCursor(0, 7);
  u8x8.print(MODE_STATE_TXT);

  //u8x8.print(newMessage.c_str());
}

void clear_info()
{  
  u8x8.noInverse();
  u8x8.setCursor(0, 7);
  u8x8.print("               ");
}

void draw_line(uint8_t c, uint8_t is_inverse)
{  
  uint8_t r;
  u8x8.setInverseFont(is_inverse);
  for( r = 0; r < u8x8.getCols(); r++ )
  {
    u8x8.setCursor(r, c);
    u8x8.print(" ");
  }
}

/** ------- **/
/** BUTTONS **/
/** ------- **/
void clickUP() {
  changeDisplay(1);
  Serial.print("Button UP pressed\n "); 
  if(wantedTemperature + 0.1 <= 30) {
    wantedTemperature += 0.1;
    send(msg_setpoint.setSensor(0).set(wantedTemperature, 1));
  }
}

void clickDOWN() {
  changeDisplay(1);
  Serial.print("Button DOWN pressed\n ");
  if(wantedTemperature - 0.1 >= 4) {
    wantedTemperature -= 0.1;
    send(msg_setpoint.setSensor(0).set(wantedTemperature, 1));
  }
}

void clickOK() {
  Serial.print("Button OK pressed\n ");
  if(showScreen == 2) {
    changeDisplay(0);
  }
  else {
    changeDisplay(showScreen + 1);
  }
}

void clickMENU() {
  changeDisplay(1);
  Serial.print("Button MENU pressed\n ");
}

void longPressUPStart() {
  Serial.print("Button UP start\n ");
}

void longPressUPStop() {
  Serial.print("Button UP stop\n ");
}

void longPressUP() {
  Serial.print("Button UP hold\n ");
}

void longPressDOWNStart() {
  Serial.print("Button DOWN start\n ");
}

void longPressDOWNStop() {
  Serial.print("Button DOWN stop\n ");
}

void longPressDOWN() {
  Serial.print("Button DOWN hold\n ");
}


void longPressMENUStart() {
  Serial.print("Button MENU start\n ");
}

void longPressMENUStop() {
  Serial.print("Button MENU stop\n ");
}

void longPressMENU() {
  Serial.print("Button MENU hold\n ");
}


void longPressOKStart() {
  Serial.print("Button OK start\n ");
}

void longPressOKStop() {
  Serial.print("Button OK stop\n ");
}

void longPressOK() {
  Serial.print("Button longPressOK\n ");
}
