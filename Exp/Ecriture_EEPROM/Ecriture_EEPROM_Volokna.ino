#include <Arduino.h>
#include "avrlib/i2c.h"


//I2C Constants
#define addressOffset   0x50                //Control offset defined for 24LC256 EEPORMs : 1010 0000
#define writeOrder      0x00
#define readOrder       0x01

//Constants
#define PRINT_ERR_MESSAGES true
int const TEST_SIZE = 64;
byte const TEST_DATA[TEST_SIZE] = {
147,249,69,214,114,217,181,16,59,28,184,55,123,181,28,101,12,200,164,135,212,25,208,168,140,252,125,60,215,172,64,244,82,248,50,141,193,157,65,31,241,58,233,223,116,200,164,242,210,169,103,252,118,
210,188,43,38,131,86,61,118,208,57,154
                                   };

byte const EEPROM_address = 0x00;

int const max_8bit = 256;
byte const pageSize = 64;
byte const maxTries = 64;


//Custom methods
void writeData(byte startAddr[], byte data[], int nBytes);
void tryToWrite(byte addr[], byte data[], byte nBytes);

byte writeByte(byte addr[], byte data);
byte writeBlock(byte addr[], byte data[]);
byte readByte(byte addr[]);
byte readSeq(byte start[], int nBytes, byte ret[]);

void getAddress(int i, byte addr[], byte start[]);
byte startCom(byte EEPROMaddr, bool writeMode);
byte sendAddr(byte regAddr[]);

void printErr(byte errCode);
void printErrSuccess(byte tries, byte addr[]);
void printErrAbort(byte code, byte addr[]);

//Arduino methods
void setup(){
  byte readData[TEST_SIZE]={0}, startAddr[2]={0};
  
  Serial.begin(9600);
  Serial.print("Launching...\n");
  i2cInit;
  Serial.print("I2C interface launched\n");
  Serial.print("Launched\n\n");
  
  Serial.print("Saving...\n");
  writeData(startAddr, TEST_DATA, TEST_SIZE);
  Serial.print("Saved\n\n");
  
  Serial.print("Reading and comparing...\n");
  readSeq(startAddr, TEST_SIZE, readData);
  for(int i=0; i<TEST_SIZE; i++){
    Serial.print(i);
    Serial.print(" - ");
    Serial.print(TEST_DATA[i]);
    Serial.print(" : ");
    Serial.print(readData[i]);
    Serial.print("\n");
  }
  Serial.print("Data read and compared\n\n");
  
  Serial.print("Done\n\n");
}

void loop(){}



//Methods details
void writeData(byte startAddr[], byte data[], int nBytes){
  byte addr[2], toWrite[pageSize]={0}, mod, modOffset;
  int i=0, firstBlockSize = 0;

  modOffset = (startAddr[1]*256+startAddr[0])%pageSize;
  if(modOffset !=0){
    firstBlockSize = min(pageSize-modOffset, nBytes);
    for(int i=0; i < firstBlockSize; i++)
      toWrite[i]= data[i];
    getAddress(0,addr, startAddr);
    tryToWrite(addr, toWrite, firstBlockSize);
  }
  for(int k=firstBlockSize; k<nBytes; k++){
    i = k + modOffset;
    mod = i%pageSize;
    if(mod == 0){
      if(i != firstBlockSize + modOffset)
        tryToWrite(addr, toWrite, pageSize);
      getAddress(k,addr,startAddr);
    }
    toWrite[mod] = data[k];
  }
  if(mod != 0){
    getAddress(nBytes, addr, startAddr);
    tryToWrite(addr, toWrite, mod);
  }}

void tryToWrite(byte addr[], byte data[], byte nBytes){
        byte b = 0xFF, tries = 0;
        bool abortWrite = false;
        
        while(b != 0x00 && !abortWrite){
          b = writeBlock(addr, data, nBytes);
          if(b != 0x00)
            abortWrite = tries++ >= maxTries;
        }
        
        if(b == 0x00){
          if(tries>0)
            printErrSuccess(tries, addr);
        }else
          printErrAbort(b, addr);
}

byte writeByte(byte addr[], byte data){
  byte comStatus;
  
  comStatus = startCom(EEPROM_address, true);
    if(comStatus != 0x00){
      printErr("WriteByte start",comStatus);
      return comStatus;
    }
  
  comStatus = sendAddr(addr);
    if(comStatus != 0x00){
      printErr("WriteByte addr",comStatus);
      return comStatus;
    }
  
  i2cSendByte(data);          //Data byte
  comStatus = i2cGetStatus();
  if(comStatus != 0x28){
      printErr("WriteByte data",comStatus);
      return comStatus;
  }
  
  i2cSendStop;                  //Stop
  i2cWaitForComplete;
  return 0x00;
}

byte writeBlock(byte addr[], byte data[], byte nBytes){
  byte comStatus;
  
  comStatus = startCom(EEPROM_address, true);
    if(comStatus != 0x00){
      printErr("WriteBlock start",comStatus);
      return comStatus;
    }
      
  comStatus = sendAddr(addr);
    if(comStatus != 0x00){
      printErr("WriteBlock addr",comStatus);
      return comStatus;
    }
    
  for(int i=0; i<nBytes; i++){      //Data bytes
    i2cSendByte(data[i]);
    comStatus = i2cGetStatus();
    if(comStatus != 0x28){
      printErr("WriteBlock data",comStatus);
      return comStatus;
    }
  }
  
  i2cSendStop;                  //Stop
  i2cWaitForComplete;
  return 0x00;
}

byte readByte(byte addr[]){
  byte comStatus;
  
  comStatus = startCom(EEPROM_address, true);
    if(comStatus != 0x00){
      printErr("ReadByte start",comStatus);
      return comStatus;
    }
  
  comStatus = sendAddr(addr);
    if(comStatus != 0x00){
      printErr("ReadByte addr",comStatus);
      return comStatus;
    }
  
  comStatus = startCom(EEPROM_address, false);
    if(comStatus != 0x00){
      printErr("ReadByte data",comStatus);
      return comStatus;
    }
    
  i2cReceiveByte(false);          //Asks for a byte
  
  i2cSendStop;                    //Stop
  return i2cGetReceivedByte();
}

byte readSeq(byte start[], int nBytes, byte ret[]){
  byte comStatus;
  
  comStatus = startCom(EEPROM_address, true);
    if(comStatus != 0x00){
      printErr("ReadSeq start1",comStatus);
      return comStatus;
    }
  
  comStatus = sendAddr(start);
    if(comStatus != 0x00){
      printErr("ReadSeq addr",comStatus);
      return comStatus;
    }
  comStatus = startCom(EEPROM_address, false);
    if(comStatus != 0x00){
      printErr("ReadSeq start2",comStatus);
      return comStatus;
    }
  
  for(int i=0; i<nBytes-1; i++){  //Get almost all bytes
    i2cReceiveByte(true);
    ret[i] = i2cGetReceivedByte();
  }
  i2cReceiveByte(false);           //Last byte
  ret[nBytes-1] = i2cGetReceivedByte();
  
  i2cSendStop;                    //Stop
  return 0x00;
}

byte startCom(byte EEPROMaddr, bool writeMode){
  byte order, comStatus;
  if(writeMode)
    order = writeOrder;
  else
    order = readOrder;
  
  i2cSendStart;                     //Start
  comStatus = i2cGetStatus();
  if(comStatus != 0x08){
    printErr("StartCom start",comStatus);
    return comStatus;
   }
  
  i2cSendByte(addressOffset | EEPROMaddr | order);
  comStatus = i2cGetStatus();      //EEPROM Address
  if(comStatus != 0x18){
      printErr("StartCom addr",comStatus);
       return comStatus;
    }
  
  return 0x00;
}

byte sendAddr(byte regAddr[]){
  byte comStatus;
  
  i2cSendByte(regAddr[1]);           //Register address high byte
  comStatus = i2cGetStatus();
  if(comStatus != 0x28){
      printErr("SendAddr high",comStatus);
      return comStatus;
  }
  
  i2cSendByte(regAddr[0]);           //Register address low byte
  comStatus = i2cGetStatus();
  if(comStatus != 0x28){
      printErr("SendAddr low",comStatus);
      return comStatus;
  }
  
  return 0x00;
}

void getAddress(int i, byte addr[], byte start[]){
  
  i += start[1]*256 + start[0];
  addr[0] = i%max_8bit;
  addr[1] = (i-addr[0])/max_8bit;
}

void printErr(String source, byte errCode){
  
  if(PRINT_ERR_MESSAGES){
    Serial.print(source);
    Serial.print(" - Error code : 0x");
    Serial.print(errCode, HEX);
    Serial.print("\n");
  }
}

void printErrSuccess(byte tries, byte addr[]){
  
  Serial.print("Succesfully wrote after ");
  Serial.print(tries);
  Serial.print(" tries at address ");
  Serial.print(addr[1] & 0xEF);
  Serial.print(":");
  Serial.print(addr[0]);
  Serial.print("\n");
}

void printErrAbort(byte code, byte addr[]){
  
  Serial.print("Writing process aborted after ");
  Serial.print(maxTries);
  Serial.print(" tries at address ");
  Serial.print(addr[1] & 0xEF);
  Serial.print(":");
  Serial.print(addr[0]);
  Serial.print(" with the final error code being : 0x");
  Serial.print(code, HEX);
  Serial.print("\n");
}
