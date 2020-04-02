#include <Arduino.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <L3G.h>

L3G gyro;
Adafruit_MMA8451 mma = Adafruit_MMA8451();


#define ADDRESS_EEPROM[8] = {
  0x50,
  0x51,
  0x52,
  0x53,
  0x54,
  0x55,
  0x56,
  0x57
}

#define ADDRESS_ACC = 0x50;


int pin_capteurs[5] = {A1, A2, A3, A4, A5};

int freq_echant = 1000; //Hz
int record_time = 6000; //6s

float tension_nominale_battery = 12.0; // V
float seuil_limit_bat = 10.0; // V
float rapport_pont_diviseur_batterie = 0.5;

int off_set_jauges[5] = {0,0,0,0,0};
int pin_battery = A21;

//Définition des tableaux de données
int data_strain[5][freq_echant*record_time] = 0; //5 cases pour les capteurs
int data_acc[3][freq_echant*record_time] = 0; // données accélération x, y, z repère fusée
int data_gyro[3][freq_echant*record_time] = 0; // données accélération x, y, z, repère fusée
float data_time[freq_echant*record_time] = 0.0; // prise précise du temps.

int pin_buzzer = 23;
int date_buzzer = 0;

int pin_wheatstone = 24;
int pin_led_erreur = 13;
int pin_button = A22;
int pin_WP = 39;
int pin_SDA_EEPROM = 38;
int pin_SCL_EEPRIM = 37;
int pin_TX_seq = 32;
int pin_RX_seq = 31;
int pin_led_save = 28;
int pin_led_on = 27;
int pin_SCL_ACC = 3;
int pin_SDA_ACC = 4;
int pin_dem_exp = A7;

byte highAddress;
byte lowAddress;
byte data;

bool etat_buzzer = false;
bool alarm_buzzer = false;
int delay_bip_alarm = 100;



bool on_record = false;
bool ready = false;
bool etat_led_ready = false;
int date_led_ready
int delay_ready_flash = 500;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin()

  if (!gyro.init())
  {
    Serial.println("Failed to autodetect gyro type!");
    while (1);
  }

  gyro.enableDefault();

  if (! mma.begin()) {
      Serial.println("Couldnt start");
      while (1);
    }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_8_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");

  pinMode(pin_battery, INPUT);
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_button, INPUT);
  pinMode(pin_led_erreur, OUTPUT);
  pinMode(pin_button, INPUT);
  pinMode(pin_WP, OUTPUT);
  pinMode(pin_led_save, OUTPUT);
  pinMode(pin_led_on, OUTPUT);
  digitalWrite(pin_led_on, HIGH);
  pinMode(pin_wheatstone, OUTPUT);
  pinMode(pin_dem_exp, INPUT);
  
  int i =0;
  for(i = 0; i<5;i++)
  {
    pinMode(pin_capteurs[i], INPUT);
  }


}

void eepromWrite(int index, byte highAddress, byte lowAddress, byte data) {

  Wire.beginTransmission(ADDRESS_EEPROM[index]);
  Wire.write(highAddress);
  Wire.write(lowAddress);
  Wire.write(data);
  Wire.endTransmission();
}

byte eepromRead(int index, byte highAddress, byte lowAddress){

  Wire.beginTransmission(ADDRESS_EEPROM[index]);
  Wire.write(highAddress);
  Wire.write(lowAddress);
  Wire.endTransmission();

  Wire.requestFrom(ADDRESS_EEPROM[index], 1) // one byte requested

  while (!Wire.avialable())
  {}
  return Wire.read();

}

bool check_battery() {

  int niveau = analogRead(pin_battery);
  float tension_lue = map(niveau, 0,1023, 0, 5.0);
  return tension_lue / rapport_pont_diviseur_batterie < seuil_limit_bat;
}

void save_eeprom(){

  int i = 0;
  int rank =0;
  int max_addr_0 = pow(2,7);
  int index_EE_acc = 5;
  int index_EE_gyro = 6;
  int index_EE_time = 7;
  byte addr[2]={0x0, 0x0};

  for (i=0; i<5;i++)
  {
    for(rank=0;rank<freq_echant*record_time;rank++)
    {
      addr[0] = rank % max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(i, addr[1], addr[0], byte(data_strain[i][rank]);
    }
  }
  for(rank=0; rank<freq_echant*record_time;rank+=3){ //les données d'un meme temps sont stocké à la suite acc_x, acc_y, acc_z
      addr[0] = rank % max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(index_EE_acc, addr[1], addr[0], byte(data_acc[0][rank]);
      addr[0] = (rank+1) % max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(index_EE_acc, addr[1], addr[0], byte(data_acc[1][rank+1]);
      addr[0] = (rank +2)% max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(index_EE_acc, addr[1], addr[0], byte(data_acc[2][rank+2]);
  }
  for(rank=0; rank<3*freq_echant*record_time;rank+=3){ //les données d'un meme temps sont stocké à la suite acc_x, acc_y, acc_z
      addr[0] = rank % max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(index_EE_gyro, addr[1], addr[0], byte(data_gyro[0][rank]);
      addr[0] = (rank+1) % max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(index_EE_gyro, addr[1], addr[0], byte(data_gyro[1][rank+1]);
      addr[0] = (rank +2)% max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(index_EE_gyro, addr[1], addr[0], byte(data_gyro[2][rank+2]);
  }
  for(rank=0; rank<freq_echant*record_time;rank++){
      addr[0] = rank % max_addr;
      addr[1] = (rank-addr[0])/max_addr;
      eepromWrite(index_EE_time, addr[1], addr[0], byte(data_time[rank]); 
  }

}

void set_offset() {
  int i = 0;
  for(i=0; i<4; i++) {
    off_set_jauges[i] = analogRead(pin_capteurs[i]);
  }
}

int lecture_jauge(int i){

  return analogRead(pin_capteurs[i])-off_set_jauges[i];
}


void record() {//enregistrement du vol jusqu'a la fin
  float t = 0.0;
  float T = 1/float(freq_echant); //en seconde 
  int i = 0;
  j = 0;
  sensors_event_t event;
  int t_ref = millis()


  while(j < int(record_time*freq_echant)) {
    t_ref = millis()
    digitalWrite(pin_wheatstone, HIGH);
    for(i=0; i<4; i++)
    {
      data_strain[i][j] = lecture_jauge(i);
    }
    digitalWrite(pin_wheatstone, LOW);
    
    t += T;
    j += 1;
    mma.getEvent(&event);
    gyro.read();

    data_acc[1][j] = event.acceleration.x;
    data_acc[2][j] = event.acceleration.y;
    data_acc[3][j] = event.acceleration.z;

    data_gyro[1][j] = gyro.g.x;
    data_gyro[2][j] = gyro.g.y;
    data_gyro[3][j] = gyro.g.z;

    data_time[j] = t;
    if (millis() - t_ref < T*1000){ //millis()-t_ref est le temps de l'operation
      delay( T*1000- millis()+t_ref); // on attend le temps restant
    }

  }

}
void loop() {
  // put your main code here, to run repeatedly:
  ready = true
  if (millis() % 10000 == 0) {} // toute les 10s
  
      alarm_buzzer = check_battery(); //on vérifie le niveau de batterie
  }

  if (alarm_buzzer and millis()-date_buzzer > delay_bip_alarm ){ // bip en alternance si l'alarme est déclanchée
      etat_buzzer = not etat_buzzer;
      if etat_buzzer {
        digitalWrite(pin_buzzer, HIGH);
      }
      else
      {
        digitalWrite(pin_buzzer, LOW);
      }
  }
  
  if (analogRead(pin_dem_exp) > 500) { //demarage exp
    on_record = true;
    ready = false;
    record();
  }

  if (analogRead(pin_button) > 500 and not on_record) { // remise à zero offset jauges appui court // save EEPROM appui long, on interdit le offset pendant le vol
    delay(1000);
    if (analogRead(pin_button) > 500) { // appuis long -> save
      digitalWrite(pin_led_save, HIGH);
      save_eeprom();
      digitalWrite(pin_led_save, LOW);
    }
    else{ //appui court -> offset zero
      digitalWrite(pin_led_on, LOW);
      set_offset();
      delay(100);
      digitalWrite(pin_led_on, HIGH);      
    }
  }

  if (ready and millis() - date_led_ready> delay_ready_flash) { //clignotement ready
    etat_led_ready = not etat_led_ready;
    if etat_led_ready{

      digitalWrite(pin_led_save, HIGH);
    }
    else{
      digitalWrite(pin_led_save, LOW);
    }
    }
    else{

      digitalWrite(pin_led_save, LOW);
  }
}
