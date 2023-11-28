#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 3
#define RST_PIN 4

MFRC522 rfid(SS_PIN, RST_PIN);

void setup(){
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
}

void loop(){
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;
  LeituraTag();
}

void LeituraTag(){
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  Serial.println("Cartao Detectado");
}
