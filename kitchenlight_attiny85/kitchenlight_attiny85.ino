
const int buttonpin = 1;    // INT0, physical pin7 on tiny85, A1
const int indicatorpin = 3; // physical pin 2, A3
const int ledpin = 1;       // physical pin 6, D1 PWM
const int dimlevelpin = 2;  // physical pin 3, A2
const int sensorpin = 0;    // physical pin 5, D0 IN

// dimmed level
int dimlevel = 32;
// full power level
int maxlevel = 255;
// speed of dimming
int dimspeed = 3;

// time in minutes before powering off leds while beeing in auto dimmed mode
long int powerofftime = 60;

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
  pinMode(dimlevelpin, INPUT);
  pinMode(indicatorpin, OUTPUT);

  // set initial LED state
//  digitalWrite(indicatorpin, HIGH);

//  attachInterrupt(buttonpin, buttonpressed, RISING);
  MCUCR |= B00000011;    //watch for rising edge
  GIMSK |= B01000000;    //enable external interrupt
  SREG |= B10000000;     //global interrupt enable
   
  // initial light up
  dimup();
}

void loop() {
  // put your main code here, to run repeatedly:
  dimlevel = getdimlevel();
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
  int level;
  level = analogRead(dimlevelpin);
  if (level > 1024) {
    digitalWrite(indicatorpin, HIGH);
  } else {
    digitalWrite(indicatorpin, LOW);
  }
  return (level / 4);
}

int doauto() {
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
    if (dimmed == 1 || poweredoff == 1) {
      if ((millis() - lastmovementtime) > (powerofftime * 60 * 1000)) {
        dimoff();
      } else {
        analogWrite(ledpin, dimlevel);
      }
    } else {
      dimdown();
    }
    
  }
}

int doallon() {
  if (dimmed == 0) {
    analogWrite(ledpin, maxlevel);
  } else {
    dimup();
  }
}

int dodimmed() {
  if (dimmed == 0) {
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

ISR(INT0_vect) {
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


