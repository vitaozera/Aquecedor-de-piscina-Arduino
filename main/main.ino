// CONSTANTS
boolean ON = 1;
boolean OFF = 0;
unsigned long filteringCycleStartTime = 0;
unsigned long filteringCycleStopTime = 21600; // 6 hours

// GLOBAL VARIABLES
unsigned long time;

void setup() {
}

void loop() {
  // Time since start
  time = millis();
}

// Switches motors ON and OFF
void switchMotor(int velocity, boolean state) {
  switch(velocity) {
    case 0:
      switchRelay(0, state);
      break;
    case 1:
      switchRelay(1, state);
      break;
    case 2:
      switchRelay(2, state);
      break;
  }
}

// Switches compressor ON and OFF
void switchCompressor(boolean state) {
    switchRelay(3, state);
}

// Switches water bomb ON and OFF
void switchWaterBomb(boolean state) {
    switchRelay(4, state);
}

// Switches relays states
void switchRelay(int index, boolean state) {

}

void LCDDisplay(string text) {
  
}

// Checks water temperature
double checkWaterTemperature() {

}

// Checks water flux
int waterFlux() {
  
}
