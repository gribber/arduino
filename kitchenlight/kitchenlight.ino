
const int buttonpin = 0;  // 0 = first interuptpin = pin2 on uno
const int indicatorpin = 8;
const int ledpin = 9;
const int dimlevelpin = 0;
const int sensorpin = 4;

// dimmed level
int dimlevel = 32;
// full power level
int maxlevel = 255;
// speed of dimming
int dimspeed = 3;

// time in minutes before powering off leds while beeing in auto dimmed mode
long int powerofftime = 1;

// debounce delay in ms
int debouncedelay = 150;

int sensorstate = LOW;
int dimmed = 0;
unsigned long lastmovementtime = 0;
int poweredoff = 0;

volatile int programstate = 0;
volatile int programstatechanged = 0;
volatile unsigned long lastpressedtime = 0;


void setup() {
  pinMode(sensorpin, INPUT);
  pinMode(buttonpin, INPUT);
  pinMode(indicatorpin, OUTPUT);

  // set initial LED state
  digitalWrite(indicatorpin, HIGH);

  attachInterrupt(buttonpin, buttonpressed, RISING);
  Serial.begin(9600);
  dimup();
}

void loop() {
  // put your main code here, to run repeatedly:
  dimlevel = getdimlevel();
  Serial.print("Dimlevel: ");
  Serial.println(dimlevel);
  switch (programstate) {
    case 0:
      doauto();
      break;
    case 1:
      doallon();
      break;
    case 2:
      dodimmed();
      break;
    default:
      doauto();  
  }

}

int getdimlevel() {
  return (analogRead(dimlevelpin) / 4);
}

int doauto() {
  Serial.println("Program: Auto");
  if (programstatechanged == 1) {
    programstatechanged = 0;
    dimup();
  }
  sensorstate = digitalRead(sensorpin);
  if (sensorstate == HIGH) {
    lastmovementtime = millis();
    if (dimmed == 0) {
      analogWrite(ledpin, maxlevel);
    } else {
      dimup();
    }
  } else {
    if (dimmed == 1) {
      Serial.print("powerofftime: ");
      Serial.println(powerofftime * 60 * 1000);
      if ((millis() - lastmovementtime) > (powerofftime * 6 * 1000)) {
        Serial.println("powering off...");
        dimoff();
      } else {
        analogWrite(ledpin, dimlevel);
        Serial.println((millis()-lastmovementtime)/1000);
      }
    } else {
      dimdown();
    }
    
  }
}

int doallon() {
  Serial.println("Program: All ON");
  if (dimmed == 0) {
    analogWrite(ledpin, maxlevel);
  } else {
    dimup();
  }
}

int dodimmed() {
  Serial.println("Program: Dimmed");
  if (dimmed == 1) {
    dimdown();  
  } else {
    analogWrite(ledpin, dimlevel);
  }
  
}

int dimup() {
  int i, dimfrom;
  if (poweredoff) {
    dimfrom = 0;
  } else {
    dimfrom = dimlevel;
  }
  for (i=dimfrom; i<maxlevel; i++) {
    analogWrite(ledpin, i);
    delay(dimspeed);
  }
  dimmed = 0;
  poweredoff = 0;
}

int dimdown() {
  int i;
  for (i=maxlevel; i>dimlevel; i--) {
    analogWrite(ledpin, i);
    delay(dimspeed);
  }
  dimmed = 1;
}

int dimoff() {
  int i;
  if (!poweredoff) {
    for (i=dimlevel; i>=0; i--) {
     analogWrite(ledpin, i);
     // dimspeed * 10 as the dimlevel is usually low so dimming doesnt look like just power off
     delay(dimspeed*10);
   }
   poweredoff = 1;
  }
}

void buttonpressed() {
  if ((millis() - lastpressedtime) > debouncedelay) {
    programstatechanged = 1;
    lastpressedtime = millis();
    if (programstate >= 2) {
      programstate = 0;
    } else {
      programstate++;
    }
  }
}


