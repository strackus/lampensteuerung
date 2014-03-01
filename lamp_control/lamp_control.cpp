// Do not remove the include below
#include "lamp_control.h"

/* Steuerung: Pegel erkennbar: Schalter-Abfrage aktiviert
 Pegel geht rauf: Gerät fährt rauf
 Pegel bleibt: Lampe wird ein/ausgeschaltet.
 Versuch Motorbremse
 Dimmen modifiziert
 */

// this constant won't change.  It's the pin number
// of the sensor's output:
const int
buttonPin = 2, //für Endschalter 1
PWM_A   = 3, //Motor
pingPin = 7, //US
BRAKE_A = 9,
led = 10,
DIR_A   = 12,
SNS_A   = A0,
mindHoehe = 60; //Mindesthöhe; darunter sollte die Lampe wieder rauffahren, da die Geste darunter nicth funktioniert
int
//a,//a ist die aktuelle Höhe der Lampe über dem Boden/Tisch
z,//Rückgabewert
ledState = 0, //0: aus, 1: ein, 2: raufgedimmt 3: runtergedimmt.
fadeVal = 0, //fadeVal: Dimm-Stärke 0 bis 255 (= nicht gedimmt)
mindH = 45; // Max Abstand der Hand vom US

void setup() {
  // Configure the A output
  pinMode(buttonPin, INPUT);
  pinMode(led, OUTPUT);
  pinMode(BRAKE_A, OUTPUT);  // Brake pin on channel A - Motor
  pinMode(DIR_A, OUTPUT);    // Direction pin on channel A
  // serial communication:
  Serial.begin(9600);
 // a = aKalib(); //Kalibrierwert der Lampenhöhe, wird zu Beginn und nach Motorfahrten ermittelt
  digitalWrite(led, LOW);
}

/////////////////////////////////////////////////////////////////
void loop()
{
  //Serial.print(a);
  Serial.print("l");
  if (aKalib() < mindH) { //|| abstand() < 20
    int x = trend();
    Serial.println(x);
    switch (x) {
    case -1:
      Serial.println("LOOP::case -1");
      motorRauf();
      break;
    case 0:
      Serial.println("LOOP::case 0");
      ledDimmen();
      break;
    case 1:
      Serial.println("LOOP::case 1");
      motorRunter();
      break;
    default:
      Serial.println("LOOP::default");
      ledSchalten();
      break;
    }
    Serial.print("loop::ENDE switch, a=");
    //Serial.print(a);
    Serial.print(" x= ");
    Serial.println(x);
  }
}
/////////////////////////////////////////////////////////////////
int trend() //ermittelt Trend aus 5 Messungen, RÜckgabe: -1: M. rauf, 0: LED Dimmen, +1: M. runter; >1: LED Schalten
{
  delay(100);
  Serial.print("trend(), z = ");
  z=0;
  Serial.println(z);
  int //Rückgabewert: z=0: kein rauf/runter
  cm1,
  cm2,cm3,cm4,cm5, // die unterschiedlichen Messwerte zur Ermittlung des Trends
  del = 50; //delay zwischen 2 Messungen
  cm1 = abstand();
  Serial.print(cm1);
  Serial.print("-1,");
  delay(del);
  cm2 = abstand();
  Serial.print(cm2);
  Serial.print("-2,");
  delay(del);
  cm3 = abstand();
  Serial.print(cm3);
  Serial.print("-3,");
  delay(del);
  cm4 = abstand();
  Serial.print(cm4);
  Serial.print("-4,");
  delay(del);
  cm5 = abstand();
  if (cm5 >  mindH + 20) {
    z= 10;
  } //war die Hand zu kurz drunter, kann kein Trend gebildet werden, bzw. ist die Intention: schalten
  Serial.print(cm5);
  Serial.print("-5. Trend (z) = ");
  Serial.println(z);
  int t = (cm2-cm1)+(cm3-cm2)+(cm4-cm3)+(cm5-cm4);
  if  (t > 4)   {
    Serial.print("trnd > 4: ");
    Serial.print(t);
    z += 1;  //ohne Fehlmessung Rückgabe 0 +1, d.h. runterfahren. Mit Fehlmessung: 10 - 1 = 9
  }
  else if (t < - 4) {
    Serial.print("trnd < - 4: ");
    Serial.print(t);
    z -= 1; //ohne Fehlmessung Rückgabe 0 -1, d.h. rauffahren. Mit Fehlmessung: 10 + 1 = 11 --> Schalten
  }
  //  else  {
  //    Serial.print("trnd ELSE: ");
  //    Serial.print(t);
  //    z += 2; //ohne Fehlmessung Rückgabe 0 +2, d.h. Dimmen. Mit Fehlmessung: 10 + 2 = 12 --> Schalten
  //  }
  Serial.print("ENDE trend, z = ");
  Serial.println(z);
  return z; //z=-1: rauf. z=0: Dimmen. z=1: runter z>8: schalten.
}
/////////////////////////////////////////////////////////////////
void raufBremsen() {
  Serial.print("raufBremsen");
  analogWrite(PWM_A, 90);
  delay(200);
  digitalWrite(BRAKE_A, HIGH);
  //digitalWrite(BRAKE_A, HIGH);
  //digitalWrite(BRAKE_A, HIGH);
  delay(500);
}
/////////////////////////////////////////////////////////////////
void runterBremsen() {
  Serial.print("runterBremsen");
  digitalWrite(DIR_A, HIGH);   // setting direction to HIGH the motor will spin forward
  analogWrite(PWM_A, 90);
  delay(200);
  raufBremsen();
}
/////////////////////////////////////////////////////////////////
void mindestHoehe() {
  Serial.println("mindestHoehe()");
  while (abstand() < mindHoehe) {
    Serial.println("mindestHoehe::while");
    digitalWrite(BRAKE_A, LOW);  // setting brake LOW disable motor brake
    digitalWrite(DIR_A, HIGH);   // setting direction to HIGH the motor will spin forward
    analogWrite(PWM_A, 255);
    delay(50);// Set the speed of the motor, 255 is the maximum value
//    if (digitalRead(buttonPin) == true) { //das wäre der Endschalter oben
//      etwasRunter();
//      break;
//    }
  }
  raufBremsen();
  //a = aKalib();
  //Serial.println(a);
  Serial.println("ENDE Mindesthoehe");
}
/////////////////////////////////////////////////////////////////
void ledSchalten() {  //int trnd

  pinMode(led, OUTPUT);
  Serial.print("ledSchalten()");

  switch (ledState) {
    //0: aus, 1: ein, 2: raufgedimmt 3: runtergedimmt.
  case 0: // LED ist aus
    digitalWrite(led, HIGH);
    Serial.print("case 0: LED eingeschaltet");
    ledState = 1; // eingeschaltet
    fadeVal = 255; //damit anschließend runtergedimmt werden kann
    break;
  case 1: //LED ist ein --> ausschalten
    Serial.print("case 1: LED ausgeschaltet");
    digitalWrite(led, LOW);
    ledState = 0; //aus
    fadeVal = 0; //wenn dann gedimmt wird, muss raufgedimmt werden
    break;
  default:
    Serial.print("default: LED ausgeschaltet");
    digitalWrite(led, LOW); // LED ist gedimmt -> ausschalten
    ledState = 0;
    fadeVal = 0;
  }
  do {
    delay(1);
    Serial.print("ledSchalten::while");
  }
  while (abstand() < mindH); //((abs(a - abstand()) > mindH));
  Serial.print("ledSchalten ENDE");
}
/////////////////////////////////////////////////////////////////
void ledDimmen() {
  //ledState 0: aus, 1: ein, 2: raufgedimmt 3: runtergedimmt.
  Serial.print("ledDimmen(), ledState = ");
  Serial.println(ledState);
  switch (ledState) {
  case 0:
    //LED ist aus --> raufdimmen
    ledRaufDimmen();
    break;
  case 1: //led ist eingeschaltet --> runterdimmen
    ledRunterDimmen();
    break;
  case 3: //weiter raufdimmen
    ledRaufDimmen();
    break;
  case 4: //weiter runterdimmen
    ledRunterDimmen();
  }
  Serial.println("ledDimmen() ENDE");
}
/////////////////////////////////////////////////////////////////
void ledRaufDimmen() {
  pinMode(led, OUTPUT);
  Serial.println("ledRunterDimmen");
  while (fadeVal < 255 && abstand() < mindH) {
    fadeVal +=5; // runterdimmen: fadeVal bis 255 raufzählen!
    analogWrite(led, fadeVal);
    delay(30);
  }
  while (abstand() < mindH) { //(aKalib() < 60) //((abs(a - abstand()) > mindH));
    delay(1);
    Serial.print(fadeVal);
    Serial.print(" = fadeVal");
  }
  if (fadeVal >= 255) {
    ledState = 1;
    fadeVal = 255;
    pinMode(led, OUTPUT);
    digitalWrite(led, HIGH);
    Serial.println("ledRaufDimmen - fadeVal == 255 - ENDE");
  }
}
/////////////////////////////////////////////////////////////////
void ledRunterDimmen() {
  pinMode(led, OUTPUT);
  Serial.println("ledRaufDimmen");
  while (fadeVal > 0 && abstand() < mindH) {
    fadeVal -=5; // raufdimmen: fadeVal bis 0 runterzählen!
    analogWrite(led, fadeVal);
    delay(20);
  }
  while (abstand() < mindH) {
    delay(1);
    Serial.print(fadeVal);
    Serial.print(" = fadeVal");
  }

  if (fadeVal < 20) { //wird kaum mehr gedimmt, brennt also
    ledState = 0;
    fadeVal = 0;
    pinMode(led, OUTPUT);
    digitalWrite(led, LOW);
    Serial.println("ledRunterDimmen - fadeVal == 0 - ENDE");
  }
}
/////////////////////////////////////////////////////////////////
void etwasRauf() {
  Serial.println("etwasRauf()");
  digitalWrite(BRAKE_A, LOW);  // setting brake LOW disable motor brake
  digitalWrite(DIR_A, HIGH);   // setting direction to HIGH the motor will spin forward
  analogWrite(PWM_A, 255);
  delay(700);
  raufBremsen();
  //a = aKalib();
  Serial.println("ENDE etwasRauf");
}
/////////////////////////////////////////////////////////////////
void etwasRunter() {
  Serial.println("etwasRunter()");
  digitalWrite(BRAKE_A, LOW);
  digitalWrite(DIR_A, LOW);   // setting direction to HIGH the motor will spin forward
  analogWrite(PWM_A, 255);     // Set the speed of the motor, 255 is the maximum value
  delay(700);
  runterBremsen();
  //a = aKalib();
  Serial.println("ENDE etwasRunter");
}
//////////////////////////////////////////////////////////////////
void motorRauf() // geschwindigkeit>0 rauf, <0 runter
{
  Serial.print("motorRauf()... a = ");
  //Serial.println(a);
  digitalWrite(BRAKE_A, LOW);
  while (abstand() < 60) {//(abstand() < a-20) {
    Serial.println("Motor rauf::while");  // setting brake LOW disable motor brake
    digitalWrite(DIR_A, HIGH);   // setting direction to HIGH the motor will spin forward
    analogWrite(PWM_A, 255);    // Set the speed of the motor, 255 is the maximum value
//    if (digitalRead(buttonPin) == true) { ////////////////////???????????????
//      etwasRunter();
//      break;
//    }
    delay(50);
  }
  runterBremsen(); //war mal: raufBremsen!
    delay(100);
  //a = aKalib();
  //Serial.println(a);
  Serial.print("ENDE motorRauf, a = ");
  //Serial.println(a);
}
/////////////////////////////////////////////////////////////////
void motorRunter() // geschwindigkeit>0 rauf, <0 runter
{
  Serial.println("motorRunter()... a = ");
  //Serial.println(a);
  digitalWrite(BRAKE_A, LOW);
  while (aKalib() < 60) {
    Serial.println("Motor runter:::while");
    digitalWrite(DIR_A, LOW);   // setting direction to HIGH the motor will spin forward
    analogWrite(PWM_A, 255);     // Set the speed of the motor, 255 is the maximum value
    delay(50);
    //    if (abstand() < 15) {
    //      etwasRauf();
    //      break;
    //    }
  }
  runterBremsen();
  mindestHoehe();
  delay(200);
  //a = aKalib();
  Serial.print("ENDE Motor runter, a = ");
  //Serial.println(a);
}
/////////////////////////////////////////////////////////////////
int aKalib() {
  int cm1 = abstand();
  int cm2 = abstand();
  int cm3 = abstand();
  int cm4 = abstand();
  int cm5 = abstand();

  int numbers[] = { cm1, cm2, cm3, cm4, cm5 };
  isort(numbers, sizeof(numbers));
  return (numbers[3]);
}
/////////////////////////////////////////////////////////////////
void isort(int *a, int n)
{
  for (int i = 1; i < n; ++i)
  {
    int j = a[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < a[k]); k--)
    {
      a[k + 1] = a[k];
    }
    a[k + 1] = j;
  }
}

int abstand()//int cm)
{
  //reads ultrasonic and returns distance in cm
  Serial.print(".");
  int r;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(pingPin, INPUT);
  r = pulseIn(pingPin, HIGH)/ 29 / 2;
  Serial.print(r) ;
  Serial.print(" ");
  delay(50);
  return r;
}
