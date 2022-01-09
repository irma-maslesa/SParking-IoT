#include <Firebase.h>
#include <FirebaseArduino.h>
#include <FirebaseCloudMessaging.h>
#include <FirebaseError.h>
#include <FirebaseHttpClient.h>
#include <FirebaseObject.h>

#include <Servo.h>

#include <ESP8266WiFi.h>

#define FIREBASE_HOST "ssparking-iot-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "Mb9yaUqFgg3ItNAxRDu22dbEMhONgZsWukh8jLS6"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

int TRIG = D6;                  //ultrasonic trig  pin
int ECHO = D5;                  // ultrasonic echo pin

int IRUlaz = D3;
int IRIzlaz = D1;

int servoUlazPin = D2;
int servoIzlazPin = D7;

long duration, distance;
int posUlaz, posIzlaz;

int mjestaNaParkingu = 3;
int autaNaParkingu = 0;

Servo servoUlaz;
Servo servoIzlaz;

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(IRUlaz, INPUT);
  pinMode(IRIzlaz, INPUT);

  servoUlaz.attach(servoUlazPin);
  servoUlaz.write(120);

  servoIzlaz.attach(servoIzlazPin);
  servoIzlaz.write(175);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Firebase.setInt("UkupnoMjesta", mjestaNaParkingu);
  autaNaParkingu = Firebase.getInt("TrenutnoZauzeto");
  
  if (Firebase.failed()) {
    Serial.print("Firebase error: ");
    Serial.println(Firebase.error());
  }

}

void loop() {
  digitalWrite(TRIG, LOW);         // make trig pin low
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);        // make trig pin high
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration / 2) / 29.1;      // take distance in cm

  int autoUslo = digitalRead(IRUlaz);

  if (autoUslo == LOW && autaNaParkingu < mjestaNaParkingu) {
    autaNaParkingu ++;
    Firebase.pushInt("BrojAuta", autaNaParkingu );
    Firebase.setInt("TrenutnoZauzeto", autaNaParkingu);

    if (Firebase.failed()) {
      Serial.print("Firebase error: ");
      Serial.println(Firebase.error());
    }

    for (posUlaz = 120; posUlaz >= 15; posUlaz -= 10) {        // change servo position
      servoUlaz.write(posUlaz);
      delay(15);
    }

    delay(2000);
    for (posUlaz = 15; posUlaz <= 120; posUlaz += 10) {       // change servo position
      servoUlaz.write(posUlaz);
      delay(15);
    }
  } else if (autoUslo == LOW && autaNaParkingu == mjestaNaParkingu) {
    Firebase.pushInt("NemaMjesta", 1 );

    if (Firebase.failed()) {
      Serial.print("Firebase error: ");
      Serial.println(Firebase.error());
    }

    Serial.print("Nema vise mjesta");
    Serial.print("  ");
  }

  int autoIzaslo = digitalRead(IRIzlaz);

  if (autoIzaslo == LOW && autaNaParkingu > 0) {
    autaNaParkingu --;
    Firebase.pushInt("BrojAuta", autaNaParkingu );
    Firebase.setInt("TrenutnoZauzeto", autaNaParkingu);

    if (Firebase.failed()) {
      Serial.print("Firebase error: ");
      Serial.println(Firebase.error());
    }

    for (posIzlaz = 175; posIzlaz >= 70; posIzlaz -= 10) {        // change servo position
      servoIzlaz.write(posIzlaz);
      delay(15);
    }

    delay(3000);

    for (posIzlaz = 70; posIzlaz <= 175; posIzlaz += 10) {       // change servo position
      servoIzlaz.write(posIzlaz);
      delay(15);
    }
  }

  Serial.print("Slobodna mjesta: ");
  Serial.println(mjestaNaParkingu - autaNaParkingu);

  Serial.print("Parking mjesto 1: SLOBODNO  ");

  Serial.print("Parking mjesto 2: ");
  if (distance < 7 && distance != 0) {

    Serial.print("ZAUZETO!  ");
    Serial.print(distance);
    Firebase.pushInt("PM2", 1 );

    if (Firebase.failed()) {
      Serial.print("Firebase error: ");
      Serial.println(Firebase.error());
    }
  }
  else {
    Firebase.pushInt("PM2", 0 );

    if (Firebase.failed()) {
      Serial.print("Firebase error: ");
      Serial.println(Firebase.error());
    }

    Serial.print("SLOBODNO! ");
    Serial.print(distance);
  }


  Serial.println("Parking mjesto 3: SLOBODNO  ");

  delay(3000);
}
