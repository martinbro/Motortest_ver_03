/*
Motortest.ino
Program til stand alone test af motormodulet
Version 0.3
Board: DOIT ESP32 DEVKIT V1
Compiler: esp32 by Espressif Systems 2.0.14
Jørgen Friis 24-03-2024
***********************************************************************
Koden læser en værdi fra den serielle monitor på PC'en
og afhængigt af værdien gør programmet noget.

Kommandoer består af motornavn efterfulgt af 3 cifre:
SBMFxxx Styrbord hovedmotor på xxx % af fuld kraft frem
SBMRxxx Styrbord hovedmotor på xxx % af fuld kraft bak
BBMFxxx Bagbordbord hovedmotor på xxx % af fuld kraft frem
BBMRxxx Bagbordbord hovedmotor på xxx % af fuld kraft bak
SBROxxx Styrbord ror på xxx grader
BBROxxx Bagbord ror på xxx grader
FTRSxxx Forreste truster på xxx % af fuld kraft styrbord
FTRBxxx Forreste truster på xxx % af fuld kraft bagbord
BTRSxxx Bagerste truster på xxx % af fuld kraft styrbord
BTRBxxx Bagerste truster på xxx % af fuld kraft bagbord

Kommandoen kan også være aflæsning af fugtighedsmåler i %:
FUGT000

Kommandoen kan også være valg af forsyningsstreng
P05V000 5 V forsyning fra kreds A
P05V001 5 V forsyning fra kreds B
P12V000 12 V forsyning fra kreds A
P12V001 12 V forsyning fra kreds B

Kommandoen kan også være aflæsning af tacometer på skrueakslen
SBTA000 Styrbord tacometer aflæses i rpm.
BBTA000 Bagbord tacometer aflæses i rpm.

*/

#include "defines.h"
#include "WiFi.h"
#include "AsyncUDP.h"
#include <ESP32Servo.h>


const char* ssid = WIFI_SSID; //Enter SSID
const char* password = WIFI_PASSWD; //Enter Password

//name space
AsyncUDP udp;
Servo SBror;
Servo BBror;

// Tildeling af navne til benene på ESP 32
const int K0_pin = 4;
const int K1_pin = 5; //must be high during boot
const int BTR_PWM_pin = 16;
const int BTR_IN1_pin = 17;
const int BTR_IN2_pin = 18;
const int SBR_PWM_pin = 19;
const int FTR_PWM_pin = 21;
const int FTR_IN3_pin = 22;
const int FTR_IN4_pin = 23;
const int BBM_RPWM_pin = 25;
const int BBM_LPWM_pin = 26;
const int BBR_PWM_pin = 27;
const int SBM_RPWM_pin = 32;
const int SBM_LPWM_pin = 33;
const int STA_pin = 34;
const int BTA_pin = 35;
const int FUGT_pin = 39;

// Globale variable
char kommandoType[5];
int kommandoNummer;
const int maxCnt = 100;
volatile int cnt = 0;
String RCdata;

// Måleværdier
float fugt = 0;

// Kommandoer modtaget over WiFi
int ror = 0;
int energi = 0;
int mode = 0;

// Setpoints

// Kalibrering

// Funktioner


void setup() {
              
  // 1. Start seriel port
  Serial.begin(115200);
  while (!Serial) { delay(1); }
  Serial.println("setup() startet");

  // 2. kontakt wifi
  WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(100);
		Serial.print(".");
	}
	Serial.println(" Connected! to WiFi.localIP()");

  // Definer funktion af benene på ESP 32
  pinMode(K0_pin, OUTPUT);
  pinMode(K1_pin, OUTPUT);
  pinMode(BTR_PWM_pin, OUTPUT);
  pinMode(BTR_IN1_pin, OUTPUT);
  pinMode(BTR_IN2_pin, OUTPUT);
  pinMode(SBR_PWM_pin, OUTPUT);
  pinMode(FTR_PWM_pin, OUTPUT);
  pinMode(FTR_IN3_pin, OUTPUT);
  pinMode(FTR_IN4_pin, OUTPUT);
  pinMode(BBM_RPWM_pin, OUTPUT);
  pinMode(BBM_LPWM_pin, OUTPUT);
  pinMode(BBR_PWM_pin, OUTPUT);
  pinMode(SBM_RPWM_pin, OUTPUT);
  pinMode(SBM_LPWM_pin, OUTPUT);
  pinMode(STA_pin, INPUT);
  pinMode(BTA_pin, INPUT);
  pinMode(FUGT_pin, INPUT);

  // Initier kontakter: HIGH svarer til NC stillingen, som er den samme, som når der ikke er spænding på.
  digitalWrite(K0_pin, HIGH);
  digitalWrite(K1_pin, HIGH);
  
  // Initier motorer
  analogWrite(BTR_PWM_pin, 0);
  digitalWrite(BTR_IN1_pin, LOW);
  digitalWrite(BTR_IN2_pin, LOW);
  analogWrite(FTR_PWM_pin, 0);
  digitalWrite(FTR_IN3_pin, LOW);
  digitalWrite(FTR_IN3_pin, LOW);
  analogWrite(SBM_RPWM_pin, 0);
  analogWrite(SBM_LPWM_pin, 0);
  analogWrite(BBM_RPWM_pin, 0);
  analogWrite(BBM_LPWM_pin, 0);
  analogWrite(SBR_PWM_pin, 90);
  analogWrite(BBR_PWM_pin, 90);

  // Initier servomotorer
  SBror.attach(SBR_PWM_pin);
  BBror.attach(BBR_PWM_pin);

  // Initier tacometre


  Serial.println("setup() sluttet, starter overvågning af lokal WiFi");

  // 3. lytter efter indkommen beskeder på port:13 
  // (beskeder fra kommunikations board (1) til motorboard (3))
  // Vi forventer 3 heltal formateret som <ror,energi,mode>
  if(udp.listen(WiFi.localIP(),13)) {
    udp.onPacket([](AsyncUDPPacket packet) {
      if (packet.length() > 0){
        char buffer[packet.length()+1];
        //cast´er fra byte-array til char-array             
        RCdata = String( (char*) packet.data());
      }
    });
  }
}

void loop() {
  // analyserer data
  
  int openingBracket = RCdata.indexOf('<');
  int closingBracket = RCdata.indexOf('>');
  int firstComma = RCdata.indexOf(',');
  int secondComma = RCdata.indexOf(',', firstComma+1);
  String rorString = RCdata.substring(openingBracket+1,firstComma);
  ror = rorString.toInt();
  String energiString = RCdata.substring(firstComma+1,secondComma);
  energi = energiString.toInt();
  String modeString = RCdata.substring(secondComma+1,closingBracket);
  mode = modeString.toInt();

  // præsentere parsing af data
  Serial.printf("Ror = %d, Energi = %d, Mode = %d",ror,energi,mode);
  Serial.println("");
  
  //udfører ordrer

  if(mode > 0){
  //Ror:

  SBror.write(ror);
  BBror.write(ror);
  delay(100);
  //Motorer
  if((energi > 95 ) && (energi < 105)){ //lig stille
    Serial.print("Begge motorer holder stille");
    analogWrite(SBM_LPWM_pin, 0);
    analogWrite(SBM_RPWM_pin, 0);
    delay(100);
    analogWrite(BBM_LPWM_pin, 0);
    analogWrite(BBM_RPWM_pin, 0);
  }
  else if (energi <= 95){ //bak
    Serial.print("Begge motorer bakker");
    analogWrite(SBM_RPWM_pin, 0);
    analogWrite(SBM_LPWM_pin, (100-energi)*2);
    delay(100);
    analogWrite(BBM_RPWM_pin, (100-energi)*2);
    analogWrite(BBM_LPWM_pin, 0);
  }
  else if (energi >= 105){ //frem
    Serial.print("Begge motorer kører frem");
    analogWrite(SBM_RPWM_pin, (energi-100)*2);
    analogWrite(SBM_LPWM_pin, 0);
    delay(100);
    analogWrite(BBM_RPWM_pin, 0);
    analogWrite(BBM_LPWM_pin, (energi-100)*2);
  }
  delay(100);
  }
  else{

  }

}

