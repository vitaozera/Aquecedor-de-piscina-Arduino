
#include <SPI.h>

#include <EEPROMVar.h>
#include <EEPROMex.h>
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ClickButton.h>
#include <string.h>
#include <stdlib.h>

// CONSTANTS
const bool NA = HIGH;
const bool NF = LOW;
const unsigned long millisInADay = 86400000; // 24 hours
const unsigned long millisInAnHour = 3600000; // 1 hour
const int Contrast=15;
const int Brightnss = 48;
const int actionState_standby = 0;
const int actionState_filtration= 1;
const int actionState_heating = 2;
const int actionState_preHeating = 3;
const int actionState_heatingError = 4;
const int menuState_main = 0;
const int menuState_heating = 1;
const int menuState_setHeating = 2;
const int menuState_setFiltering = 3;
const int menuState_autoFiltering = 4;
const unsigned long defaultConfigurationSaveInterval = 2000;
const unsigned long defaultCheckTemperatureInterval = 30000;
const unsigned long defaultUpdateLCDInterval = 100;
const unsigned long LCDDimTime = 15000;
const unsigned long defaultPreHeatingDuration = 5000;
const int buttonsAmount = 3;
const int button_set = 0;
const int button_down = 1;
const int button_up = 2;
// skip 4 bytes for each data
const int memoryHeatingTemperature = 0;
const int memorySetHeating = 4;
const int memoryForcedFiltering = 8;
const int memoryFilteringDuration = 12;
const int memoryFilteringStart = 16;

// PINS
const int pin_relay0 = A3; // motor - velocity 0
const int pin_relay1 = A4; // motor - velocity 1
const int pin_relay2 = A5; // motor - velocity 2
const int pin_relay3 = 8; // compressor
const int pin_relay4 = 7; // water bomb
const int pin_lcd_data0 = 2; // data_0
const int pin_lcd_data1 = 3; // data_1
const int pin_lcd_data2 = 4; // data_2
const int pin_lcd_data3 = 5; // data_3
const int pin_lcd_RS = 12; // RS (Register Select)
const int pin_lcd_enable = 11; // Enable
const int pin_lcd_contrast = 6; // LCD contrast
const int pin_lcd_brightness = 9; // LCD brightness
const int pin_button_set = A0; // Set button
const int pin_button_down = A1; // Down button
const int pin_button_up = A2; // Up button
const int pin_termometer = 10; // Temperature Sensor
const int pin_pressure_switchs = 13; // Water and gas Pressure Switchs

// LCD
LiquidCrystal lcd(pin_lcd_RS, pin_lcd_enable, pin_lcd_data3,
                  pin_lcd_data2, pin_lcd_data1, pin_lcd_data0);

// Temperature sensor
OneWire oneWire(pin_termometer);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

// ClickButtons
ClickButton button[3] = {
  ClickButton (pin_button_set, LOW, CLICKBTN_PULLUP),
  ClickButton (pin_button_down, LOW, CLICKBTN_PULLUP),
  ClickButton (pin_button_up, LOW, CLICKBTN_PULLUP),
};

// GLOBAL VARIABLES
unsigned long time = 0;
unsigned long nextTimeCheckTemperature = 0;
unsigned long lastTimeUpdatedLCD = 0;
float water_temperature = 0.0;
int currentActionState = actionState_standby;
int currentMenuState = menuState_main;
unsigned long filteringCycleStartTime = 0;
unsigned long filteringCycleStopTime = 21600000; // 6 hours
unsigned long preHeatingStartedTime = 0;
int filteringCycleDuration = 6; // Filtering cycle duration in hours
int lastButtonPressed = -1;
boolean buttonPressed = false;
unsigned long lastButtonPressTime = 0;
boolean forcedFiltering = false; // Forced filtering - ON or OFF - User input
boolean autoFiltering = false; // Auto filtering - ON or OFF - Calculated by program
boolean heating = false; // Heating System - ON or OFF - Calculated by program
boolean preHeating = false; // Heating System [Bomb only (until with pressure)] - ON or OFF - Calculated by program
boolean setHeating = false; // Heating - On or OFF - User input
boolean heatingError = true; // Problem detected with the heating System
float heatingTemperature = 20;
boolean LCDBacklightIsOn = false;
boolean configurationChanged = false;
boolean pressureSwitchsActivated = false;

char LCDCurrentMessage[2][17]; // Vector of the strings being currently shown in the LCD

void setup() {
  // Setup pins
  pinMode(pin_relay0, OUTPUT);
  pinMode(pin_relay1, OUTPUT);
  pinMode(pin_relay2, OUTPUT);
  pinMode(pin_relay3, OUTPUT);
  pinMode(pin_relay4, OUTPUT);
  pinMode(pin_lcd_brightness, OUTPUT);

  // Needed to avoid relays turning on and off on startup
  currentActionState = actionState_standby;
  doCurrentActionState();

  pinMode(pin_pressure_switchs, INPUT);

  // Setup LCD
  lcd.begin(16, 2);
  analogWrite(pin_lcd_contrast,Contrast);

  // Setup sensors
  sensors.begin();
  sensors.getAddress(sensor1, 0);

  // Setup ClickButtons
  for (int i=0; i < buttonsAmount; i++)
  {
    button[i].debounceTime   = 20;   // Debounce timer in ms
    button[i].multiclickTime = 250;  // Time limit for multi clicks
    button[i].longClickTime  = 1000; // Time until long clicks register
  }

  // Recover saved configuration data
  recoverData();

}

void loop() {
  // Time since start. Resets back to 0 every 24 hours.
  time = (millis() % millisInADay);

  // Update water temperature
  updateWaterTemperature();

  // Check if the Pressure Switchs are all active or not
  checkPressureSwitchs();

  // Check whether auto filtering should be turned on or off
  checkAutoFiltering();

  // Check whether heating should be turned on or off
  checkHeating();

  // Check whether thre pre-heating should be turned on or off
  checkPreHeating();

  // Check Button Clicks
  checkButtonClicks();

  // Perform button click action
  doButtonClickAction();

  // Selects currentActionState
  actionStateSelector();

  // Do current action state
  doCurrentActionState();

  // Update LCD
  updateLCD();

  // Check to see if there is any configuration to be saved
  checkConfigurationChange();
}

// Checks whether heating should be turned on or off
void checkHeating() {

  if(!setHeating) {
    heating = false;
    return;
  }

  if(heating) {
    if(water_temperature > heatingTemperature + 0.5) {
      heating = false;
    }
    else {
      heating = true;
    }
  }
  else {
    if(water_temperature < heatingTemperature - 0.5) {
      heating = true;
    }
    else {
      heating = false;
    }
  }
}

// Check wheter or not the Pre Heating system should be on or not
// Only checks if the heating shoul be on
// The Check is based only in the Pressure Switchs for now
void checkPreHeating() {

  bool last_preHeating_state = preHeating;

  // Consider that no error is happening
  heatingError = false;
  preHeating = false;

  // If the hating is not on, stop the check
  if (!heating)
    return;

  // Check the state of the Pressure Switch System
  if (pressureSwitchsActivated){
    preHeating = false;
  }
  else{
    // If the pressure switch is off, and the pre Heating limit is reached, heating Error
    if( long(preHeatingStartedTime + defaultPreHeatingDuration)%millisInADay > time && last_preHeating_state){
      heatingError = true;
      preHeating = false;
    }
    else {
      preHeating = true;
    }
  }
  if(preHeating && last_preHeating_state==false)
    preHeatingStartedTime = time;
}

// Does button click action
void  doButtonClickAction() {
 if(!buttonPressed)
   return;

  buttonPressed = false;

  // Select click action
  switch(lastButtonPressed) {
    case button_set:
      switch(currentMenuState) {
        case menuState_main:
          currentMenuState = menuState_heating;
        break;
        case menuState_heating:
          currentMenuState = menuState_setHeating;
        break;
        case menuState_setHeating:
          currentMenuState = menuState_setFiltering;
        break;
        case menuState_setFiltering:
          currentMenuState = menuState_autoFiltering;
        break;
        case menuState_autoFiltering:
          currentMenuState = menuState_main;
        break;
      }
    break;
    case button_up:
      switch(currentMenuState) {
        case menuState_main:
          // do nothing
        break;
        case menuState_heating:
          changeHeatingTemperature(0.5);
          configurationChanged = true;
        break;
        case menuState_autoFiltering:
          changeFilteringCycleDuarion(1);
          configurationChanged = true;
        break;
        case menuState_setFiltering:
          forcedFiltering = !forcedFiltering;
          configurationChanged = true;
        break;
        case menuState_setHeating:
          setHeating = !setHeating;
          configurationChanged = true;
        break;
      }
    break;
    case button_down:
      switch(currentMenuState) {
        case menuState_main:
          // do nothing
        break;
        case menuState_heating:
          changeHeatingTemperature(-0.5);
          configurationChanged = true;
        break;
        case menuState_autoFiltering:
          changeFilteringCycleDuarion(-1);
          configurationChanged = true;
        break;
        case menuState_setFiltering:
          forcedFiltering = !forcedFiltering;
          configurationChanged = true;
        break;
        case menuState_setHeating:
          setHeating = !setHeating;
          configurationChanged = true;
        break;
      }
    break;
  }
}

// Selects action state
void actionStateSelector() {
  currentActionState = actionState_standby;
  // Check if its auto filtration time
  if(autoFiltering)
    currentActionState = actionState_filtration;

  // Check if filtration is forced
  if(forcedFiltering == true)
    currentActionState = actionState_filtration;

  // Check if heating is on
  if(heating)
    currentActionState = actionState_heating;

  // Check if the pre Heating shoul be on
  if(preHeating)
    currentActionState = actionState_preHeating;

  if(heatingError)
    currentActionState = actionState_heatingError;
}

// Do current action state action
void doCurrentActionState() {
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
    case actionState_preHeating:
      // Turn on Water Bomb
      switchWaterBomb(NA);
      // Turn on motor
      switchMotor(0, NF);
      // Turn on compressor
      switchCompressor(NF);
      break;
    case actionState_heatingError:
      // Turn off motor
      switchMotor(0, NF);
      // Turn off compressor
      switchCompressor(NF);
      // Turn of Water Bomb
      switchWaterBomb(NF);
      break;
  }
}

// Detects button clicks
void checkButtonClicks() {
  // Go through each button
  for (int i=0; i<buttonsAmount; i++)
  {
    // Update state of all buittons
    button[i].Update();

    // Mark pressed button and leave function
    if(button[i].clicks == 1) {
      lastButtonPressTime = time;
      if(!LCDBacklightIsOn){
        return;
      }
      switch(i) {
        case button_set:
          buttonPressed = true;
          lastButtonPressed = button_set;
          return;
        break;
        case button_up:
          buttonPressed = true;
          lastButtonPressed = button_up;
          return;
        break;
        case button_down:
          buttonPressed = true;
          lastButtonPressed = button_down;
          return;
        break;
      }
    }
  }
}

// Shows the two strings being stored in the vector, one in each line of the LCD;
// startPointLine0 and startPointLine1 indicate on wich position
// of the display each line should be displayed.
void LCDShow(char newMessage[2][17], int startPointLine0, int startPointLine1){
  boolean changed = false;
  if(strcmp(LCDCurrentMessage[0], newMessage[0]) || strcmp(LCDCurrentMessage[1], newMessage[1]))
    changed = true;

  if(changed){
    // Save the new strings being shown in the display
    strcpy(LCDCurrentMessage[0], newMessage[0]);
    strcpy(LCDCurrentMessage[1], newMessage[1]);

    // Print both strings to the LCD
    lcd.clear();
    lcd.setCursor(startPointLine0,0);
    lcd.print(newMessage[0]);
    lcd.setCursor(startPointLine1,1);
    lcd.print(newMessage[1]);
  }

}

// Updates LCD data
void updateLCD() {
  char LCDNewMessage[2][17];
  char FloatStr[6];
  int startPointLine0=0, startPointLine1=0;

  if( (time - lastTimeUpdatedLCD) > defaultUpdateLCDInterval || long(time - lastTimeUpdatedLCD) < 0) {

    /* Dim display when not in use */
    if( (time - lastButtonPressTime) > LCDDimTime  || long(time - lastButtonPressTime) < 0) {
     analogWrite(pin_lcd_brightness, 0);
     LCDBacklightIsOn = false;
     currentMenuState = menuState_main;
    }
    else {
     analogWrite(pin_lcd_brightness, Brightnss);
     LCDBacklightIsOn = true;
    }
    switch(currentMenuState) {
      case menuState_main:
        dtostrf(water_temperature, 5, 2, FloatStr);
        sprintf(LCDNewMessage[0], "%s%cC", FloatStr, 223);
        startPointLine0 = 4;
        if(currentActionState == actionState_standby){
          sprintf(LCDNewMessage[1], "Em descanso");
          startPointLine1 = 2;
        }
        else if(currentActionState == actionState_filtration && forcedFiltering == false) {
          sprintf(LCDNewMessage[1], "Filt. auto");
          startPointLine1 = 3;
        }
        else if(currentActionState == actionState_filtration && forcedFiltering == true) {
          sprintf(LCDNewMessage[1], "Filt. forcada");
          startPointLine1 = 1;
        }
        else if(currentActionState == actionState_heating) {
          sprintf(LCDNewMessage[1], "Aquecendo");
          startPointLine1 = 3;
        }
        else if(currentActionState == actionState_preHeating) {
          sprintf(LCDNewMessage[1], "Inicio Aquec.");
          startPointLine1 = 1;
        }
        else if(currentActionState == actionState_preHeating) {
          sprintf(LCDNewMessage[1], "ERRO AQUEC.");
          startPointLine1 = 3;
        }
        break;
       case menuState_heating:
        sprintf(LCDNewMessage[0], "Aquecer ate");
        startPointLine0 = 2;
        dtostrf(heatingTemperature, 5, 2, FloatStr);
        sprintf(LCDNewMessage[1], "%s%cC", FloatStr, 223);
        startPointLine1 = 5;
       break;
       case menuState_setFiltering:
        sprintf(LCDNewMessage[0], "Forcar filtragem");
        startPointLine0 = 0;
        if(forcedFiltering){
          sprintf(LCDNewMessage[1], "Ligado");
          startPointLine1 = 5;
        }
        else{
          sprintf(LCDNewMessage[1], "Desligado");
          startPointLine1 = 3;
        }
       break;
       case menuState_autoFiltering:
        sprintf(LCDNewMessage[0], "Auto filtragem");
        startPointLine0 = 0;
        if(filteringCycleDuration){
          sprintf(LCDNewMessage[1], "%d H/dia", filteringCycleDuration);
          startPointLine1 = 4;
        }
        else{
          sprintf(LCDNewMessage[1], "Desligado");
          startPointLine1 = 3;
        }
       break;
       case menuState_setHeating:
        sprintf(LCDNewMessage[0], "Aquecimento");
        startPointLine0 = 3;
        if(setHeating){
          sprintf(LCDNewMessage[1], "Ligado");
          startPointLine1 = 5;
        }
        else{
          sprintf(LCDNewMessage[1], "Desligado");
          startPointLine1 = 3;
        }
       break;
    }
  LCDShow(LCDNewMessage, startPointLine0, startPointLine1);
  lastTimeUpdatedLCD = time;
  }
}


// Switches motors to NA or to NF
// If state = NF, all motors will go OFF.
// If state = NA, only ONE motor will go ON at a time
void switchMotor(int velocity, bool state) {
  switch(velocity) {
    case 0:
      // Sets all other relays to NF
      digitalWrite(pin_relay1, !NF);
      digitalWrite(pin_relay2, !NF);
      digitalWrite(pin_relay0, !state);
      break;
    case 1:
      // Sets all other relays to NF
      digitalWrite(pin_relay0, !NF);
      digitalWrite(pin_relay2, !NF);
      digitalWrite(pin_relay1, !state);
      break;
    case 2:
      // Sets all other relays to NF
      digitalWrite(pin_relay0, !NF);
      digitalWrite(pin_relay1, !NF);
      digitalWrite(pin_relay2, !state);
      break;
  }
}

// Switches compressor to NA or to NF
void switchCompressor(bool state) {
    digitalWrite(pin_relay2, !state);
}

// Switches water bomb to NA or to NF
void switchWaterBomb(bool state) {
    digitalWrite(pin_relay4, !state);
}

// Updates water temperature reading from sensor
void updateWaterTemperature() {
  if( time > nextTimeCheckTemperature )  {
    sensors.requestTemperatures();
    water_temperature = sensors.getTempC(sensor1);
    nextTimeCheckTemperature = (time + defaultCheckTemperatureInterval) % millisInADay;
  }
}

// Check if its filtering time
void checkAutoFiltering() {
  if(filteringCycleStartTime <= filteringCycleStopTime) {
    if(time > filteringCycleStartTime && time < filteringCycleStopTime)
      autoFiltering = true;
    else
      autoFiltering = false;
  }
  else {
    if(time > filteringCycleStartTime || time < filteringCycleStopTime)
      autoFiltering = true;
    else
      autoFiltering = false;
  }
}

// Change heating temperature
void changeHeatingTemperature(float k) {
 float auxTemperature = heatingTemperature + k;

 if(auxTemperature > 35.0) {
   heatingTemperature = 35;

 }
 else if(auxTemperature < 10.0) {
   heatingTemperature = 10;
 }
 else {
   heatingTemperature = auxTemperature;
 }
}

// Change filtering cycle duration
void changeFilteringCycleDuarion(int k) {
 int auxDuration = filteringCycleDuration + k;

 if(auxDuration > 24) {
   filteringCycleDuration = 24;

 }
 else if(auxDuration < 0) {
   filteringCycleDuration = 0;
 }
 else {
   filteringCycleDuration = auxDuration;
   filteringCycleStartTime = time;
   filteringCycleStopTime = filteringCycleStartTime + filteringCycleDuration*millisInAnHour;
 }
}

// Save some important variables to the persistent memory,
// reducing data loss on shutdown
void saveData(){
    EEPROM.updateFloat(memoryHeatingTemperature, heatingTemperature);
    EEPROM.updateBit(memorySetHeating, 0, setHeating);
    EEPROM.updateBit(memoryForcedFiltering, 0, forcedFiltering);
    EEPROM.updateInt(memoryFilteringDuration, filteringCycleDuration);
    EEPROM.updateLong(memoryFilteringStart, filteringCycleStartTime);
}

// Recover from Arduino's persistent memory some important data that
// should not be lost in case of a shutdown
void recoverData(){
    // Recover data from memory
    heatingTemperature = EEPROM.readFloat(memoryHeatingTemperature);
    setHeating = EEPROM.readBit(memorySetHeating, 0);
    forcedFiltering = EEPROM.readBit(memoryForcedFiltering, 0);
    filteringCycleDuration = EEPROM.readInt(memoryFilteringDuration);
    filteringCycleStartTime = EEPROM.readLong(memoryFilteringStart);

    // Rebuild some data based on what eas read from memory
    filteringCycleStopTime = (filteringCycleStartTime + filteringCycleDuration*millisInAnHour) % millisInADay;
}

// Check if any there was any change in any configuration;
// If there was, and the system does not receive any input in a certain time,
// it saves the new configuartions to the persistent memory.
void checkConfigurationChange(){
    if(!configurationChanged)
        return;
    if(time - lastButtonPressTime >= defaultConfigurationSaveInterval || long(time - lastButtonPressTime) < 0){
        saveData();
        configurationChanged = false;
    }
}

// Check if the Pressure Switchs are all active or not
// If they are active the heating can happen,
// else it must be stopped
void checkPressureSwitchs(){
    // Disabled for tests
    //pressureSwitchsActivated = digitalRead(pin_pressure_switchs);
    pressureSwitchsActivated = 1;
}
