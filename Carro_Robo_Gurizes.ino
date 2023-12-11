#include "SoftwareSerial.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include "dht.h"
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN    7
#define SS_PIN     8

MFRC522 rfid(SS_PIN, RST_PIN);

#define M11 5
#define M12 6
#define M21 9
#define M22 10
int velocidade = 255;

SoftwareSerial bluetooth(4, 3);
char MovimentoRobo;

#define InfraVermelho 0
int IF;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN A3
dht DHT;
float u,t;

int PinTrigger = 2;
int PinEcho = A0;
int duration, distance;

const int MPU=0x68;
int AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

#define Buzzer A2

void frente(){
  digitalWrite(M11, LOW);
  analogWrite(M12, velocidade);
  analogWrite(M21, velocidade);
  digitalWrite(M22, LOW);
}

void tras(){
  analogWrite(M11, velocidade);
  digitalWrite(M12, LOW);
  digitalWrite(M21, LOW);
  analogWrite(M22, velocidade);
}

void direita(){
  analogWrite(M11, velocidade);
  digitalWrite(M12, LOW);
  analogWrite(M21, velocidade);
  digitalWrite(M22, LOW);
}

void esquerda(){
  digitalWrite(M11, LOW);
  analogWrite(M12, velocidade);
  digitalWrite(M21, LOW);
  analogWrite(M22, velocidade);
}

void parar(){
  digitalWrite(M11, LOW);
  digitalWrite(M12, LOW);
  digitalWrite(M21, LOW);
  digitalWrite(M22, LOW);
}

void setup(){
  bluetooth.begin(115200);
  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(M11, OUTPUT);
  pinMode(M12, OUTPUT);
  pinMode(M21, OUTPUT);
  pinMode(M22, OUTPUT);
  digitalWrite(M11, LOW);
  digitalWrite(M12, LOW);
  digitalWrite(M21, LOW);
  digitalWrite(M22, LOW);
  pinMode(PinTrigger, OUTPUT);
  pinMode(PinEcho, INPUT); 
  pinMode(Buzzer, OUTPUT);
  digitalWrite(Buzzer, HIGH);
  
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0); 
  Wire.endTransmission(true);

  pinMode(InfraVermelho, INPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    while(1);
  }

  display.clearDisplay();
  display.drawRect(0, 0, display.width()-1, display.height()-1, 1);
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(4, 8);
  display.print("Carro Robo Autonomo");
  display.display();
  delay(5000);
}

void loop() {
  parar();
  IF = digitalRead(InfraVermelho);
  while(IF == 0){
    digitalWrite(SS_PIN, LOW);
    digitalWrite(RST_PIN, LOW);
    SensoresSecundarios();
    AcelerometroGiroscopio();
    OLED();
    digitalWrite(Buzzer, HIGH);
    if (bluetooth.available()){
      MovimentoRobo = bluetooth.read();
      if (MovimentoRobo == 'w'){
        velocidade = 255;
        frente();
        }
        if (MovimentoRobo == 'd'){
          velocidade = 255;
          direita();
        }
        if (MovimentoRobo == 's'){
          velocidade = 255;
          tras();
        }
        if (MovimentoRobo == 'a'){
          velocidade = 255;
          esquerda();
        }
        if (MovimentoRobo == 'z'){
          parar();
        }
    } 
    IF = digitalRead(InfraVermelho);
  }
  while(IF == 1){
    digitalWrite(SS_PIN, HIGH);
    digitalWrite(RST_PIN, HIGH);
    SensoresSecundarios();
    AcelerometroGiroscopio();
    OLED();
    Ultrassom();
    if(distance > 46){
      velocidade = 120;
      direita();
    }
    if(distance<=45 && distance > 4){
      velocidade = 255;
      frente();
    }
    if(distance<=4){
      parar();
      if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        bluetooth.println("TagLida");
        while(IF == 1){
          digitalWrite(Buzzer, LOW);
          velocidade = 220;
          tras();
          IF = digitalRead(InfraVermelho);
        }
        rfid.PICC_HaltA();
      }
    }
    IF = digitalRead(InfraVermelho);
  }
}

void OLED(){
  static unsigned long TempoOLED;

  if((millis() - TempoOLED) >=1000) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(4, 0);
    display.print("TEMPERATURA:"+ String(t) + "*C");
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(4, 8);
    display.print("UMIDADE:" + String(u)+ "%");
    display.display();

    TempoOLED = millis();
  }
}

void SensoresSecundarios(){
  static unsigned long TempoSensor;
  if((millis() - TempoSensor) >=6000) {
    DHT.read11(DHTPIN);
    u = DHT.humidity;
    t = DHT.temperature;
    bluetooth.print("TEMPUMID;");
    bluetooth.print(u);
    bluetooth.print(";");
    bluetooth.println(t);
    TempoSensor = millis();
  }
}
void Ultrassom(){
  digitalWrite(PinTrigger, HIGH);
  delay(1);
  digitalWrite(PinTrigger, LOW);
  duration = pulseIn(PinEcho, HIGH);
  distance = (duration/2)/29.1;
}

void AcelerometroGiroscopio(){
  static unsigned long TempoSensor;

  if((millis() - TempoSensor) >=1000) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU,14,true);  
    AcX=Wire.read()<<8|Wire.read();  //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
    AcY=Wire.read()<<8|Wire.read();  //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ=Wire.read()<<8|Wire.read();  //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp=Wire.read()<<8|Wire.read();  //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX=Wire.read()<<8|Wire.read();  //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY=Wire.read()<<8|Wire.read();  //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ=Wire.read()<<8|Wire.read();  //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    bluetooth.print("ACELGIROS");
    bluetooth.print(";");
    bluetooth.print(AcX);
    bluetooth.print(";");
    bluetooth.print(AcY);
    bluetooth.print(";");
    bluetooth.print(AcZ);
    bluetooth.print(";");
    bluetooth.print(GyX);
    bluetooth.print(";");
    bluetooth.print(GyY);
    bluetooth.print(";");
    bluetooth.println(GyZ);
    TempoSensor = millis();
  }
}
