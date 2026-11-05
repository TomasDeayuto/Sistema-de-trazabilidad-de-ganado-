#inlude <spi.h>
#inlude <MFRC522.h>

//configuracion del lector 1
#define SS_1 53
#define RST_1 31
MFRC522 rfid1(SS_1, RST_1);
//configuracion del lector 2
#define SS_2 33
#define RST_2 35
MFRC522 rfid2(SS_2, RST_2);

void iniciarRFID() {
  SPI.begin();

  rfid1.PCD_Init;
  rfid2.PCD_Init;
}
String obtenerUID(MFRC522&lector){
  String uidStr="";

  for(byte i=0;i<lector.uid.size;i++){
    if(lector.uid.uidByte[i]<0x10)
      uidStr+="0";
  uidStr += String (lector.uid.uidByte[i],HEX);

  uidStr.toUpperCase();
  return uidStr;
 }
}
void leerRFID() {
  if(rfid1.PICC_IsNewCardPresent() && rfid1.PICC_ReadCardSerial()) {
    String uid=obtenerUID(rfid1);
    procesarTarjetaRFID(uid,1);
    rfid1.PICC_HaltA();
    rfid1.PCD_StopCrypto1();
  }
if(rfid2.PIC_IsNewCardPresent() && rfid2.PICC_ReadCardSerial()) {
  String uid=obtenerUID(rfid2);
  procesarTarjetaRFID(uid,2);
  rfid2.PICC_HaltA();
  rfid2.PCD_StopCrypto1();
  }
}












    
