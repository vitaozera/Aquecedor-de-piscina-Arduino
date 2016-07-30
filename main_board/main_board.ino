 #include <LiquidCrystal.h>

// CONSTANTS
boolean NA = HIGH;
boolean NF = LOW;
unsigned long filteringCycleStartTime = 0;
unsigned long filteringCycleStopTime = 21600; // 6 hours
int Contrast=15;
int Brightnss = 64;

// PINS
int pin_relay0 = 7; // motor - velocity 0
int pin_relay1 = 1; // motor - velocity 1
int pin_relay2 = 2; // motor - velocity 2
int pin_relay3 = 3; // compressor
int pin_relay4 = 4; // water bomb
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

// LCD
LiquidCrystal lcd(pin_lcd_RS, pin_lcd_enable, pin_lcd_data3,
                  pin_lcd_data2, pin_lcd_data1, pin_lcd_data0);
                  
// GLOBAL VARIABLES
unsigned long time;

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
  
  // Update LCD
  updateLCD();
  
  //switchMotor(0, NA);
  
}

void loop() {
  // Time since start
  time = millis();
  
  // Check Button Clicks
  checkButtonClicks();
}

void checkButtonClicks() {
  int timePressed = 0;
  
  // While set button pressed
  while(digitalRead(pin_button_set) == LOW) {
    delay(100);
    timePressed += 100;
  }
  if(timePressed >= 100)
    // DEBUG - IMPLEMENT ACTION
    
  // While up button pressed
  while(digitalRead(pin_button_up) == LOW) {
    delay(100);
    timePressed += 100;
  }
  if(timePressed >= 100)
  // DEBUG - IMPLEMENT ACTION
  
    // While down button pressed
  while(digitalRead(pin_button_down) == LOW) {
    delay(100);
    timePressed += 100;
  }
  if(timePressed >= 100)
  // DEBUG - IMPLEMENT ACTION
}

void updateLCD() {
  lcd.setCursor(2, 0);
  lcd.print("Temp. Atual");
  lcd.setCursor(6, 1);
  lcd.print("27");
  lcd.print((char)223);
  lcd.print("C");
}

// Switches motors to NA or to NF
// If state = NF, all motors will go OFF.
// If state = NA, only ONE motor will go ON at a time
void switchMotor(int velocity, boolean state) {
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
void switchCompressor(boolean state) {
    digitalWrite(pin_relay3, state);
}

// Switches water bomb to NA or to NF
void switchWaterBomb(boolean state) {
    digitalWrite(pin_relay4, state);
}

// Checks water temperature
double checkWaterTemperature() {

}

// Checks water flux
int waterFlux() {
  
}
