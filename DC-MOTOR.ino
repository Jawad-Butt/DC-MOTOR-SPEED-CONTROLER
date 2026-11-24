#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <Keypad.h>

hd44780_I2Cexp lcd;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
{'D','C','B','A'},
{'#','9','6','3'},
{'0','8','5','2'},
{'*','7','4','1'}
};

byte rowPins[ROWS] = {4, 5, 6, 7};
byte colPins[COLS] = {8, 9, A0, A1};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int stopButton = A2;
const int statusLED = A3;

const int STBY = 10;
const int PWMA = 11;
const int AIN1 = 12;
const int AIN2 = 13;

const int encoderA = 2;
const int encoderB = 3;

const int PPR = 330;
const int MAX_RPM = 280;

volatile unsigned long encoderPulses = 0;

String inputRPM = "";
int targetRPM = 0;
int actualRPM = 0;
int motorPWM = 0;

unsigned long previousTime = 0;
unsigned long previousControlTime = 0;

void encoderISR() {
encoderPulses++;
}

void setup() {
pinMode(stopButton, INPUT_PULLUP);
pinMode(statusLED, OUTPUT);

pinMode(STBY, OUTPUT);
pinMode(PWMA, OUTPUT);
pinMode(AIN1, OUTPUT);
pinMode(AIN2, OUTPUT);

pinMode(encoderA, INPUT_PULLUP);
pinMode(encoderB, INPUT_PULLUP);

digitalWrite(STBY, LOW);
digitalWrite(AIN1, HIGH);
digitalWrite(AIN2, LOW);
analogWrite(PWMA, 0);

attachInterrupt(digitalPinToInterrupt(encoderA), encoderISR, RISING);

lcd.begin(16, 2);
lcd.backlight();

lcd.setCursor(0, 0);
lcd.print("CIRCUITX READY");

showDisplay();
}

void loop() {
char key = keypad.getKey();

if (digitalRead(stopButton) == LOW) {
stopMotor();
inputRPM = "";
targetRPM = 0;
showDisplay();
delay(300);
}

if (key >= '0' && key <= '9') {
if (inputRPM.length() < 3) {
inputRPM += key;
digitalWrite(statusLED, HIGH);
showDisplay();
}
}

if (key == '*') {
stopMotor();
inputRPM = "";
targetRPM = 0;
showDisplay();
}

if (key == '#') {
if (inputRPM.length() > 0) {
targetRPM = inputRPM.toInt();
inputRPM = "";

if (targetRPM > MAX_RPM) {  
    targetRPM = MAX_RPM;  
  }  

  if (targetRPM > 0) {  
    startMotor();  
  } else {  
    stopMotor();  
  }  

  showDisplay();  
}

}

if (millis() - previousTime >= 1000) {
previousTime = millis();

noInterrupts();  
unsigned long pulses = encoderPulses;  
encoderPulses = 0;  
interrupts();  

actualRPM = (pulses * 60UL) / PPR;  

showDisplay();

}

if (millis() - previousControlTime >= 100) {
previousControlTime = millis();

if (targetRPM > 0) {  
  int error = targetRPM - actualRPM;  

  motorPWM += error * 0.8;  

  motorPWM = constrain(motorPWM, 0, 255);  

  analogWrite(PWMA, motorPWM);  
}

}
}

void startMotor() {
digitalWrite(AIN1, HIGH);
digitalWrite(AIN2, LOW);

digitalWrite(STBY, HIGH);

motorPWM = 100;
analogWrite(PWMA, motorPWM);

digitalWrite(statusLED, HIGH);
}

void stopMotor() {
targetRPM = 0;
motorPWM = 0;

analogWrite(PWMA, 0);
digitalWrite(STBY, LOW);

digitalWrite(statusLED, LOW);
}

void showDisplay() {
lcd.setCursor(0, 0);
lcd.print("Set:");

if (inputRPM.length() > 0) {
lcd.print(inputRPM);
} else {
lcd.print(targetRPM);
}

lcd.print(" RPM       ");

lcd.setCursor(0, 1);
lcd.print("Actual:");
lcd.print(actualRPM);
lcd.print(" RPM   ");
}