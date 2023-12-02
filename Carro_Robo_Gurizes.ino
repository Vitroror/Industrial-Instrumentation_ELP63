#include "SoftwareSerial.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include "dht.h"
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 3
#define RST_PIN 4

MFRC522 rfid(SS_PIN, RST_PIN);
int Cartao = 0;

#define M11 5
#define M12 6
#define M21 7
#define M22 8
int velocidade = 255;

SoftwareSerial bluetooth(10, 9); //TX, RX (Bluetooth)
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
int PinEcho = 14;
int duration, distance;

const int MPU=0x68;
int AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

#define SensorVelocidade A1
int rpm;
volatile byte pulsos;
unsigned int pulsos_por_volta = 20;
void contador(){
pulsos++;
}

#define Buzzer A2

void frente(){
  digitalWrite(M11, LOW);
  digitalWrite(M12, velocidade);
  digitalWrite(M21, velocidade);
  digitalWrite(M22, LOW);
}

void tras(){
  digitalWrite(M11, velocidade);
  digitalWrite(M12, LOW);
  digitalWrite(M21, LOW);
  digitalWrite(M22, velocidade);
}

void esquerda(){
  digitalWrite(M11, velocidade);
  digitalWrite(M12, LOW);
  digitalWrite(M21, velocidade);
  digitalWrite(M22, LOW);
}

void direita(){
  digitalWrite(M11, LOW);
  digitalWrite(M12, velocidade);
  digitalWrite(M21, LOW);
  digitalWrite(M22, velocidade);
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
  pinMode(SensorVelocidade, INPUT);
  pinMode(Buzzer, OUTPUT);
  digitalWrite(Buzzer, HIGH);
  SPI.begin();
  rfid.PCD_Init();

  attachInterrupt(0, contador, FALLING);
  pulsos = 0;
  rpm = 0;

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
  while(IF == 1){
    Cartao = 0;
    SensoresSecundarios();
    AcelerometroGiroscopio();
    SensorVel();
    OLED();
    MovimentoRobo = bluetooth.read();
    digitalWrite(Buzzer, HIGH);
    velocidade = 255;
    if (MovimentoRobo == 'w'){
    frente();
    }
    if (MovimentoRobo == 'd'){
      direita();
    }
    if (MovimentoRobo == 's'){
      tras();
    }
    if (MovimentoRobo == 'a'){
      esquerda();
    }
    if (MovimentoRobo == 'z'){
      parar();
    }
    IF = digitalRead(InfraVermelho);
  }
  while(IF == 0){
    SensoresSecundarios();
    AcelerometroGiroscopio();
    SensorVel();
    OLED();
    Ultrassom();
    if(Cartao == 0){
      if(distance > 30){
      velocidade = 120;
      esquerda();
      }
      if(distance<=30 && distance > 15){
        velocidade = 60;
        frente();
      }
      if(distance<=15 && distance > 3){
        velocidade = 30;
        frente();
      }
      if(distance<=3){
        parar();
        if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) //VERIFICA SE O CARTÃO PRESENTE NO LEITOR É DIFERENTE DO ÚLTIMO CARTÃO LIDO. CASO NÃO SEJA, FAZ
          return; //RETORNA PARA LER NOVAMENTE
        LeituraTag();
      }
    }
    if(Cartao == 1){
      velocidade = 200;
      tras();
      digitalWrite(Buzzer, LOW);
    }
    IF = digitalRead(InfraVermelho);
  }
}

void LeituraTag(){
  rfid.PICC_HaltA(); //PARADA DA LEITURA DO CARTÃO
  rfid.PCD_StopCrypto1(); //PARADA DA CRIPTOGRAFIA NO PCD
  Cartao = 1;
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
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(4, 18);
    display.print("RPM: " +String(rpm));
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

void SensorVel(){
  static unsigned long TempoVel;
  if (millis() - TempoVel >= 1000){
    detachInterrupt(0);
    rpm = ((60 * 1000) / pulsos_por_volta ) /((millis() - TempoVel) * pulsos);
    pulsos = 0;
    TempoVel = millis();
    attachInterrupt(0, contador, FALLING);
  }
}
