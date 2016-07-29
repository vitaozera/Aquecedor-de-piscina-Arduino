// CONSTANTS
boolean NA = HIGH;
boolean NF = LOW;
unsigned long filteringCycleStartTime = 0;
unsigned long filteringCycleStopTime = 21600; // 6 hours
int pin_relay0 = 0; // motor - velocity 0
int pin_relay1 = 1; // motor - velocity 1
int pin_relay2 = 2; // motor - velocity 2
int pin_relay3 = 3; // compressor
int pin_relay4 = 4; // water bomb



// GLOBAL VARIABLES
unsigned long time;

void setup() {
  pinMode(pin_relay0, OUTPUT);
  pinMode(pin_relay1, OUTPUT);
  pinMode(pin_relay2, OUTPUT);
  pinMode(pin_relay3, OUTPUT);
  pinMode(pin_relay4, OUTPUT);
}

void loop() {
  // Time since start
  time = millis();
  
   switchMotor(0, NA);
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


void LCDDisplay(char text[]) {
  
}

// Checks water temperature
double checkWaterTemperature() {

}

// Checks water flux
int waterFlux() {
  
}
