#include <Arduino.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// CONSTANTS
const bool NA = HIGH;
const bool NF = LOW;
const unsigned long filteringCycleStartTime = 0;
const unsigned long filteringCycleStopTime = 21600000; // 6 hours
const unsigned long millisInADay = 86400000; // 24 hours
const int Contrast=15;
const int Brightnss = 64;
const int actionState_standby = 0;
const int actionState_filtration= 1;
const int actionState_heating = 2;
const unsigned long defaultCheckTemperatureInterval = 10000;

// PINS
int pin_relay0 = A3; // motor - velocity 0
int pin_relay1 = A4; // motor - velocity 1
int pin_relay2 = A5; // motor - velocity 2
int pin_relay3 = 8; // compressor
int pin_relay4 = 7; // water bomb
int pin_lcd_data0 = 2; // data_0
int pin_lcd_data1 = 3; // data_1
int pin_lcd_data2 = 4; // data_2
int pin_lcd_data3 = 5; // data_3
int pin_lcd_RS = 12; // RS (Register Select)
int pin_lcd_enable = 11; // Enable
int pin_lcd_contrast = 6; // LCD contrast
int pin_lcd_brightness = 9; // LCD brightness
int pin_button_set = A0; // Set button
int pin_button_up = A1; // Up button
int pin_button_down = A2; // Down button
int pin_termometer = 10; // Temperature Sensor

// LCD
LiquidCrystal lcd(pin_lcd_RS, pin_lcd_enable, pin_lcd_data3,
                  pin_lcd_data2, pin_lcd_data1, pin_lcd_data0);

// Temperature sensor
OneWire oneWire(pin_termometer);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

// GLOBAL VARIABLES
unsigned long time = 0;
unsigned long lastTimeCheckedTemperature = 0;
float water_temperature = 0.0;
int currentActionState = actionState_standby;

void setup() {
  // Setup pins
  pinMode(pin_relay0, OUTPUT);
  pinMode(pin_relay1, OUTPUT);
  pinMode(pin_relay2, OUTPUT);
  pinMode(pin_relay3, OUTPUT);
  pinMode(pin_relay4, OUTPUT);
  pinMode(pin_lcd_brightness, OUTPUT);
  pinMode(pin_button_set, INPUT_PULLUP);

  // Setup LCD
  lcd.begin(16, 2);
  analogWrite(pin_lcd_contrast,Contrast);
  analogWrite(pin_lcd_brightness, Brightnss);

  // Setup sensors
  sensors.begin();
  sensors.getAddress(sensor1, 0);  

  // Defining default states
  currentActionState = actionState_standby;
}

void loop() {
  // Time since start. Resets back to 0 every 24 hours.
  time = (millis() % millisInADay);
  
  // Update water temperature
  updateWaterTemperature();

  // Check Button Clicks
  checkButtonClicks();
  
  // Selects currentActionState
  actionStateSelector();
  
  // Do current action state
  doActionState(currentActionState);
    
  // Update LCD
  updateLCD();
}

void actionStateSelector() {
  // Check is its filtration time
  if(poolFiltrationTime())
    currentActionState = actionState_filtration;
  
  // Check if filtration is forced to be on by user configuration. (fountaion, for example) - IMPLEMENT
  
  // Check if heating is on - IMPLEMENT
  
}

void doActionState(int currentActionState) {
  switch(currentActionState) {
    case actionState_standby:
      // Turn off motor
      switchMotor(0, NF);
      // Turn off compressor
      switchCompressor(NF);
      // Turn of Water Bomb
      switchWaterBomb(NF);
      break;
    case actionState_filtration:
      // Turn on Water Bomb
      switchWaterBomb(NA);
      // Turn off motor
      switchMotor(0, NF);
      // Turn off compressor
      switchCompressor(NF);
      break;
    case actionState_heating:
      // Turn on Water Bomb
      switchWaterBomb(NA);
      // Turn on motor
      switchMotor(0, NA);
      // Turn on compressor
      switchCompressor(NA);
      break;
  }
}

void checkButtonClicks() {
  int timePressed = 0;

  // While set button pressed
  while(digitalRead(pin_button_set) == LOW) {
    delay(100);
    timePressed += 100;
  }
  //if(timePressed >= 100)
    // DEBUG - IMPLEMENT ACTION

  // While up button pressed
  while(digitalRead(pin_button_up) == LOW) {
    delay(100);
    timePressed += 100;
  }
  //if(timePressed >= 100)
  // DEBUG - IMPLEMENT ACTION

    // While down button pressed
  while(digitalRead(pin_button_down) == LOW) {
    delay(100);
    timePressed += 100;
  }
  //if(timePressed >= 100)
  // DEBUG - IMPLEMENT ACTION
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(water_temperature, 2);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(4, 1);
  lcd.print("Standby");
}

// Switches motors to NA or to NF
// If state = NF, all motors will go OFF.
// If state = NA, only ONE motor will go ON at a time
void switchMotor(int velocity, bool state) {
  switch(velocity) {
    case 0:
      // Sets all other relays to NF
      digitalWrite(pin_relay1, NF);
      digitalWrite(pin_relay2, NF);
      digitalWrite(pin_relay0, state);
      break;
    case 1:
      // Sets all other relays to NF
      digitalWrite(pin_relay0, NF);
      digitalWrite(pin_relay2, NF);
      digitalWrite(pin_relay1, state);
      break;
    case 2:
      // Sets all other relays to NF
      digitalWrite(pin_relay0, NF);
      digitalWrite(pin_relay1, NF);
      digitalWrite(pin_relay2, state);
      break;
  }
}

// Switches compressor to NA or to NF
void switchCompressor(bool state) {
    digitalWrite(pin_relay3, state);
}

// Switches water bomb to NA or to NF
void switchWaterBomb(bool state) {
    digitalWrite(pin_relay4, state);
}

// Updates water temperature reading from sensor
void updateWaterTemperature() {
  if( (time - lastTimeCheckedTemperature) > defaultCheckTemperatureInterval )  {
    sensors.requestTemperatures();
    water_temperature = sensors.getTempC(sensor1);
    lastTimeCheckedTemperature = time;
  }
}

// Check if its filtering time
boolean poolFiltrationTime() {
  if(time < filteringCycleStopTime)
    return true;
  else
    return false; 
}


