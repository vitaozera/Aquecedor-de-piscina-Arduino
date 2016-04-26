void setup() {
  setupFilteringCycle(); // Configura o ciclo de filtragem automatica
  LCDDisplay(temp); // Mostra temperatura no LCD
}

void loop() {
  
}


void turnOnMotor(int velocity) {
  switch(velocity) {
    case 0:
      switchRelay(0);
      break;
    case 1:
      switchRelay(1);
      break;
    case 2:
      switchRelay(2);
      break;
  }
}

void switchRelay(int index) {

}

void setupFilteringCycle() {
  
}

void LCDDisplay() {
  
}

double checkWaterTemperature() {

}
