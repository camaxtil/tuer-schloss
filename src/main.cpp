#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266_SSL.h>
#include <SPI.h> 
#include <MFRC522.h> 
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define tuer_kontakt_pin D0
#define NEOPIN  D1
#define RST_PIN D2
#define tuer_oeffner_pin D3
#define SS_PIN  D4 
#define NUMPIXELS   16

//Bitte Fachnummer und RFID code eintragen
int Fachnummer = 2;
String Code_Schueler = "";
String Code_Admin = "";
String Code_Lehrer = "";

char ssid[] = "";
char pass[] = "";
char auth[] = "I-TFLto2vdWemRFfWsphbpzdvbxorjsy";
char daysOfTheWeek[7][12] = {"Sonntag","Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
String vorname = "";
String nachname = "";

String Code = "";
boolean firstrun_bool = true;
boolean entsperren_erlaubt = true;
boolean entsperrt = false;
boolean tueroffen = false;
boolean ausgewaehlt = false;
int rot = 0;
int gruen = 0;
int blau = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
MFRC522 mfrc522(SS_PIN, RST_PIN); 
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

String get_Time(){
    String time;
    timeClient.update();
    
    time = daysOfTheWeek[timeClient.getDay()];
    time = time + "  " + (timeClient.getHours() + 1);
    time = time + ":" + timeClient.getMinutes();
    time = time + ":" + timeClient.getSeconds() + " ";
    return time; 
}
void textausgabe(boolean fachnummer_ausgeben, String text){
    if(text.length() == 0){
        text = " ";
    }
    if(fachnummer_ausgeben == true){
        text = ("Fach " + (String) (Fachnummer) + " ") + text;
        text = get_Time() + text;
    }
    
    Blynk.virtualWrite(V3,text);
    Serial.println(text);
}
void set_Led(int rot, int blau, int gruen){
    pixels.clear();
    for(int i=0; i<NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(rot, blau, gruen)); 
    }
    pixels.show(); 
}
void open_door(){
    rot = analogRead(V4);
    blau = analogRead(V5);
    gruen = analogRead(V6);
    digitalWrite(tuer_oeffner_pin,HIGH);
    set_Led(rot,blau,gruen);
    while(digitalRead(tuer_kontakt_pin) == 1){
        delay(1000);
    }
    delay(2000);
    digitalWrite(tuer_oeffner_pin,LOW);
    while(digitalRead(tuer_kontakt_pin) == 0){
        delay(1000);
    }
    textausgabe(true,"Tür geschlossen");
    set_Led(0,0,0);
}
void firstrun(){
    if(firstrun_bool == true){
        firstrun_bool = false;
        textausgabe(true,": Ist online");
        set_Led(rot,gruen,blau);
    }
}
String int_to_ascii(int zahl){
    if(zahl == 65 || zahl == 97){
        return "a";
    }
    else if(zahl == 66 || zahl == 98){
        return "b";
    }
    else if(zahl == 67 || zahl == 99){
        return "c";
    }
    else if(zahl == 68 || zahl == 100){
        return "d";
    }
    else if(zahl == 69 || zahl == 101){
        return "e";
    }
    else if(zahl == 70 || zahl == 102){
        return "f";
    }
    else if(zahl == 71 || zahl == 103){
        return "g";
    }
    else if(zahl == 72 || zahl == 104){
        return "h";
    }
    else if(zahl == 73 || zahl == 105){
        return "i";
    }
    else if(zahl == 74 || zahl == 106){
        return "j";
    }
    else if(zahl == 75 || zahl == 107){
        return "k";
    }
    else if(zahl == 76 || zahl == 108){
        return "l";
    }
    else if(zahl == 77 || zahl == 109){
        return "m";
    }
    else if(zahl == 78 || zahl == 110){
        return "n";
    }
    else if(zahl == 79 || zahl == 111){
        return "o";
    }
    else if(zahl == 80 || zahl == 112){
        return "p";
    }
    else if(zahl == 81 || zahl == 113){
        return "q";
    }
    else if(zahl == 82 || zahl == 114){
        return "r";
    }
    else if(zahl == 83 || zahl == 115){
        return "s";
    }
    else if(zahl == 84 || zahl == 116){
        return "t";
    }
    else if(zahl == 85 || zahl == 117){
        return "u";
    }
    else if(zahl == 86 || zahl == 118){
        return "v";
    }
    else if(zahl == 87 || zahl == 119){
        return "w";
    }
    else if(zahl == 88 || zahl == 120){
        return "x";
    }
    else if(zahl == 89 || zahl == 121){
        return "y";
    }
    else if(zahl == 90 || zahl == 122){
        return "z";
    }else{
        return " ";
    }

}
void check_Code(){
    String read_code = (vorname + " " + nachname);
    if(read_code == Code_Admin){
        textausgabe(true,"Tür geöffnet von: " + read_code);
        open_door();
    }else if(read_code == Code_Lehrer){
        textausgabe(true,"Tür geöffnet von: " + read_code);
        open_door();
    }else if(read_code == Code_Schueler){
        if (entsperren_erlaubt){
            textausgabe(true,"Tür geöffnet von: " + read_code);
            open_door();
        }else{
            textausgabe(true,"Tür nicht geöffnet versucht von: " + read_code);
        }
    }else{
        textausgabe(true,"Tür nicht geöffnet versucht von: " +  read_code);
    }
}
void RFID(){
    vorname = "";
    nachname = "";
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  byte block;
  byte len;
  MFRC522::StatusCode status;
  if( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex
  byte buffer1[18];
  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(("Authentication failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
    ESP.reset();
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(("Reading failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
    ESP.reset();
    return;
  }
  //PRINT FIRST NAME
  for (uint8_t i = 1; i < 16; i++){
    if (buffer1[i] != 32){
        if (int_to_ascii(buffer1[i]) != " "){
            vorname = vorname + int_to_ascii(buffer1[i]);
        }
    }
  }

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
    ESP.reset();
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.print(mfrc522.GetStatusCodeName(status));
    ESP.reset();
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++) {
        if (int_to_ascii(buffer2[i]) != " "){
            nachname = nachname + int_to_ascii(buffer2[i]);
        }
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  check_Code();
}
void array_to_string(byte array[], unsigned int len, char buffer[]){
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}
BLYNK_WRITE(V4){
    rot = param.asInt(); 
    set_Led(rot,blau,gruen);
}
BLYNK_WRITE(V5){
    gruen = param.asInt(); 
    set_Led(rot,blau,gruen);
}
BLYNK_WRITE(V6){
    blau = param.asInt(); 
    set_Led(rot,blau,gruen);
}
BLYNK_WRITE(V7){
   if(param.asInt() == Fachnummer){
       ausgewaehlt = true;
   }else{
       ausgewaehlt = false;
   }
}
BLYNK_WRITE(V1){
    if(ausgewaehlt){
        if (param.asInt() == 1){
            textausgabe(true,"Geöffnet per Smartphone");
            open_door();
        }
    }
}
BLYNK_WRITE(V0){
   int wert = param.asInt();
   if(wert == 1 ){
       entsperren_erlaubt = true;
   }else{
       entsperren_erlaubt = false;
   }
   
}
void setup(){
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    Blynk.begin(auth,ssid,pass);
    pixels.begin();
    timeClient.begin();
    pinMode(tuer_kontakt_pin,INPUT_PULLDOWN_16);
}
void loop(){
    Blynk.run();
    firstrun();
    RFID();
}
