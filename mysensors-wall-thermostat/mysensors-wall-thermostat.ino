// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable serial gateway
#define MY_GATEWAY_W5100

// When ENC28J60 is connected we have to move CE/CSN pins for NRF radio
#define MY_RF24_CE_PIN 0
#define MY_RF24_CS_PIN 0

// Gateway IP address
#define MY_IP_ADDRESS 192,168,0,40

// The MAC address can be anything you want but should be unique on your network.
// Newer boards have a MAC address printed on the underside of the PCB, which you can (optionally) use.
// Note that most of the Ardunio examples use  "DEAD BEEF FEED" for the MAC address.
#define MY_MAC_ADDRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xED

// The port to keep open on node server mode / or port to contact in client mode
#define MY_PORT 8888

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192,168,0,50

// Set blinking period
//#define MY_DEFAULT_LED_BLINK_PERIOD 300

#include <Arduino.h>
#include <U8x8lib.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MySensors.h>
#include <Wire.h>
#include "OneButton.h"
#include "SHTSensor.h"
 
// Enable repeater functionality for this node
//#define MY_REPEATER_FEATURE

#define BUTTON_UP_PIN 17
#define BUTTON_DOWN_PIN 15
#define BUTTON_MENU_PIN 14
#define BUTTON_OK_PIN 16

U8X8_SSD1327_MIDAS_128X128_4W_SW_SPI u8x8(/* clock=*/ 2, /* data=*/ 3, /* cs=*/ 4, /* dc=*/ 5, /* reset=*/ 6);

SHTSensor sht;

// Initialize thermostat message
MyMessage msg_temp(0,V_TEMP);
MyMessage msg_setpoint(0,V_HVAC_SETPOINT_HEAT);
MyMessage msg_setflowstate(0,V_HVAC_FLOW_STATE);
MyMessage msg_setstatus(0,V_STATUS);

// Initialize humidity message
MyMessage msg_humidity(1,V_HUM);

MyMessage textMsg(2, V_TEXT);  // message for Sending Text to Controller

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
float actualHumidity = 99.9;
char newMessage[5];          // next message to be displayed if available

long previousMillisTemperature = 0;
long previousMillisOutTemperature = 0;
long previousMillisDisplay = 0;
long previousMillisButtons = 0;

long startMillisButtonUP = 0;
long startMillisButtonDOWN = 0;

long tempReadInterval = 1000;
long tempReadOutInterval = 5000;
long displayRefreshInterval = 200;
long buttonReadInterval = 20;

int showScreen = 1;

// TEMP
char cstr[1];
char buff[4];
// TEMP
  
void setup(void)
{
  u8x8.begin();
  u8x8.setPowerSave(0);
  
  Serial.begin(115200);
  if (sht.init()) {
      Serial.print("init(): success\n");
  } else {
      Serial.print("init(): failed\n");
  }
  sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); // only supported by SHT3x
  
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
  buttonMENU.attachClick(clickOK);
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
  sendSketchInfo("Thermostat SHT30", "1.0");

  // Present all sensors to controller
  present(0, S_HEATER);
  present(1, S_HUM);
  present(2, S_INFO);
  
  send(msg_setpoint.setSensor(0).set(wantedTemperature, 1));
  
  strcpy(newMessage, "ERR..");    // copy it in

  request(2, V_TEXT, 0);          // request value from controller
}

void loop(void)
{
  unsigned long currentMillis = millis();
  
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
    request(2, V_TEXT, 0);          // request value from controller
    //Serial.print("request V_TEXT OK\n");
  }
  
  /** REFRESH DISPLAY **/
  if(currentMillis - previousMillisDisplay > displayRefreshInterval) {
    previousMillisDisplay = currentMillis;   
    refreshDisplay();
    //Serial.print("refreshDisplay ");
  }
}

void receive(const MyMessage &message) {
  //Serial.print("Recieved: "); Serial.println(message.type); Serial.print("\n"); 
  if (message.type==V_HVAC_SETPOINT_HEAT) {
    Serial.print("Recieved val: "); Serial.println(message.data); Serial.print("\n"); 
    wantedTemperature = atof(message.data);
  }
  if (message.type==I_TIME) {
    Serial.print("Recieved val I_TIME: "); Serial.println(message.data); Serial.print("\n"); 
  }
  if (message.type==V_TEXT) {
    strcpy(newMessage, message.getString());    // copy it in
  } 
}

/** ----------- **/
/** SENSOR READ **/
/** ----------- **/
void readTemp()
{
  //Serial.print("Reading SHT30 Data...\n");
  if (sht.readSample()) {
      //Serial.print("SHT:\n");
      //Serial.print("  RH: ");
      actualHumidity = sht.getHumidity();
      //Serial.print(actualHumidity, 2);
      //Serial.print("\n");
      //Serial.print("  T:  ");
      actualTemperature = sht.getTemperature();
      //Serial.print(actualTemperature, 2);
      //Serial.print("\n");
  } else {
      Serial.print("Error in readSample()\n");
  }
  //Serial.print("Reading SHT30 Data...OK\n");
  
  send(msg_setpoint.setSensor(0).set(wantedTemperature, 1));
  if (! isnan(actualHumidity)) {  // check if 'is not a number'
    send(msg_humidity.setSensor(0).set(actualHumidity, 1));
  }
  if (! isnan(actualTemperature)) {  // check if 'is not a number'
    send(msg_temp.setSensor(0).set(actualTemperature, 1));
  }
}

/** ------- **/
/** DISPLAY **/
/** ------- **/
void refreshDisplay() {
  //Serial.print("Refreshing display...\n");

  switch(showScreen) {
    case 1:
      showDefault();
      break;
    case 2:
      showBig();
      break;
    default:
      showDefault();
      break;
  }
}

void showBig() {
  u8x8.noInverse();
  u8x8.setFont(u8x8_font_pressstart2p_f);
  
  dtostrf(wantedTemperature, 2, 1, buff);

  u8x8.draw2x2String(4, 8, buff);
  u8x8.drawUTF8(12, 8, "°");

  dtostrf(actualTemperature, 2, 1, buff);
  u8x8.drawUTF8(0, 15, buff);
  u8x8.drawUTF8(4, 15, "°");

  dtostrf(actualHumidity, 2, 1, buff);
  u8x8.drawUTF8(11, 15, buff);
  u8x8.drawUTF8(15, 15, "%");
}

void showDefault() {
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
  u8x8.setCursor(2,9);
  u8x8.inverse();
  u8x8.print(" Akt. pogoda ");
  u8x8.noInverse();
  u8x8.draw2x2String(2, 11, newMessage);
  u8x8.drawUTF8(12, 11, "°");
  //u8x8.setCursor(2,14);
  //u8x8.print(" Pada deszcz ");

  //u8x8.drawUTF8(0, 7, "----------------");
  //draw_line(7, 1);
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
  Serial.print("Button UP pressed\n "); 
  if(wantedTemperature + 0.1 <= 30) {
    wantedTemperature += 0.1;
    send(msg_setpoint.setSensor(0).set(wantedTemperature, 1));
  }
}

void clickDOWN() {
  Serial.print("Button DOWN pressed\n ");
  if(wantedTemperature - 0.1 >= 4) {
    wantedTemperature -= 0.1;
    send(msg_setpoint.setSensor(0).set(wantedTemperature, 1));
  }
}

void clickOK() {
  Serial.print("Button OK pressed\n ");
  if(showScreen == 2) {
    showScreen = 1;
  }
  else {
    showScreen += 1;
  }
  u8x8.clear();
}

void clickMENU() {
  Serial.print("Button MENU pressed\n ");
  if(wantedTemperature - 0.1 >= 4) {
    wantedTemperature -= 0.1;
    send(msg_setpoint.setSensor(0).set(wantedTemperature, 1));
  }
}

void longPressUPStart() {
  startMillisButtonUP = millis();
  Serial.print("Button UP start\n ");
}

void longPressUPStop() {
  Serial.print("Button UP stop\n ");
}

void longPressUP() {
  unsigned long currentMillis = millis();
  if(((startMillisButtonUP-currentMillis) % 100) == 0) {
    Serial.print("Button UP hold\n ");
    clickUP();
  }
}

void longPressDOWNStart() {
  startMillisButtonDOWN = millis();
  Serial.print("Button DOWN start\n ");
}

void longPressDOWNStop() {
  Serial.print("Button DOWN stop\n ");
}

void longPressDOWN() {
  unsigned long currentMillis = millis();
  if(((startMillisButtonDOWN-currentMillis) % 100) == 0) {
    Serial.print("Button DOWN hold\n ");
    clickDOWN();
  }
}


void longPressMENUStart() {
  startMillisButtonUP = millis();
  Serial.print("Button MENU start\n ");
}

void longPressMENUStop() {
  Serial.print("Button MENU stop\n ");
}

void longPressMENU() {
  unsigned long currentMillis = millis();
  if(((startMillisButtonUP-currentMillis) % 100) == 0) {
    Serial.print("Button MENU hold\n ");
    clickUP();
  }
}


void longPressOKStart() {
  Serial.print("Button OK start\n ");
}

void longPressOKStop() {
  Serial.print("Button OK stop\n ");
}

void longPressOK() {
}
