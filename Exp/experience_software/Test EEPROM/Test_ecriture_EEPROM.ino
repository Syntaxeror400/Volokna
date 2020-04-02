#include <Arduino.h>
#include <Wire.h>

//Constantes
int const taille_table = 100;
int const max_8bit = pow(2,8);
int const max_tries = 64;
byte const data[taille_table] = {154,47,3,75,19,236,101,122,185,48,224,82,62,76,178,105,169,208,106,129,111,252,229,20,160,136,181,222,14,248,240,231,174,150,223,196,136,2,219,73,57,188,155,235,135,225,35,137,93,147,248,248,103,4,93,17,184,102,48,115,252,32,111,86,75,211,122,28,18,182,17,47,114,180,52,253,35,143,123,125,23,4,214,184,42,137,88,87,163,199,133,235,159,103,114,37,95,42,67,169};

int const address_offset = 0x50;   //Control offset defined for 24LC256 EEPORMs
int const EEPROM_address = 0x00;

int const pin_SDA_EEPROM = 20;
int const pin_SCL_EEPROM = 21;

bool const SHOW_SAVE_ERRORS = true;

//Variables
bool first = true;
bool second = false;
bool done = false;

void setup(){
  Serial.begin(9600);
  Serial.print("Launching...\n");
  Wire.begin();
  Serial.print("Launched\n\n");
}

byte eepromWrite(byte highAddress, byte lowAddress, byte data) {
  
  Wire.beginTransmission(address_offset | EEPROM_address);
  Wire.write(highAddress);
  Wire.write(lowAddress);
  Wire.write(data);
  return Wire.endTransmission();
}

byte eepromRead(byte highAddress, byte lowAddress){
  
  Wire.beginTransmission(address_offset | EEPROM_address);
  Wire.write(highAddress);
  Wire.write(lowAddress);
  Wire.requestFrom(address_offset | EEPROM_address, 1);
  while(!Wire.available());
  return Wire.read();
}

void save_eeprom(){
  byte addr[2]={0x0, 0x0}, b=0x0;
  int tries=0;

  for(int i=0; i<taille_table; i++){
    addr[0] = i % max_8bit;
    addr[1] = (i-addr[0])/max_8bit;
    b = eepromWrite(addr[1], addr[0], data[i]);
    
    if(b)
      if(tries < max_tries){
        i--;
        tries++;
      }else{
        if(SHOW_SAVE_ERRORS){
          Serial.print("Write aborted for address ");
          Serial.print(addr[1]);
          Serial.print(":");
          Serial.print(addr[0]);
          Serial.print(" after ");
          Serial.print(tries);
          Serial.print(" tries\n");
        }
        tries=0;
      }
    else if(tries>0){
        if(SHOW_SAVE_ERRORS){
          Serial.print("Success for address ");
          Serial.print(addr[1]);
          Serial.print(":");
          Serial.print(addr[0]);
          Serial.print(" after ");
          Serial.print(tries);
          Serial.print(" tries\n");
        }
        tries=0;
      }
    delay(5);
  }
}

void loop(){
  if(!done && !first){
    second=true;
  }
  
  if(first){
    Serial.print("Saving\n");
    save_eeprom();
    Serial.print("Saved\n\n");
    first=false;
  }

  if(second){
    Serial.print("Reading :\n\n");
    
    byte addr[2]={0x0, 0x0};
    for(int i=0; i<taille_table; i++){
      addr[0] = i % max_8bit;
      addr[1] = (i-addr[0])/max_8bit;
      Serial.print(i);
      Serial.print(" : ");
      Serial.print(data[i]);
      Serial.print(" - ");
      Serial.print(eepromRead(addr[1], addr[0]));
      Serial.print("\n");
     }
     Serial.print("\nDone\n");
    done = true;
    second = false;
  }
}
