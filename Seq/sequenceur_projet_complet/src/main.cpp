#include <LiquidCrystal.h>
#include <Arduino.h>
#include <Servo.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // pin D12, D11, D5, D4, D3, D2 used for display messages one the board
int pin_exp = A0;
int pin_telem = A1;
int pin_jack = 13;
int pin_buzzer = 10;
int pin_led_V = 8; //sequenceur sous tension
int pin_led_O = 7; //décollage de la fusée
int pin_led_R = 6; //deployment parachute
boolean led_R_status = false;
int pin_recover_ok = A5;
int pin_recover = 9;
Servo recover_servo;
int pin_battery_state = A2;

int T1 = 1500;
int T2 = 2500;

int ExpReadyCode = 1;
int ExpFailCode = 2;
int TelemReadyCode = 3;
int TelemFailCode = 4;

int battery_limit_voltage = 11;

int level_jack;
int level_recover;

int level_battery;
boolean warning_buzzer = false;
int date_millis_alert = millis();
boolean buzzer_status = false;
int delay_bip = 350;

boolean flight_in_progress = false;

int servo_init_rotation = 0; //angle de rotation initiale servo
int servo_final_rotation = 180; // angle de rotation pour ouverture recuperation
int time_up; //date du décollage
boolean recover_in_progress = false;
boolean recover_done = false;

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Initializing ...");

  pinMode(pin_jack, INPUT);
  pinMode(pin_exp, OUTPUT);
  pinMode(pin_telem, OUTPUT);
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_led_V, OUTPUT);
  pinMode(pin_led_O, OUTPUT);
  pinMode(pin_led_R, OUTPUT);
  pinMode(pin_recover_ok, INPUT);
  pinMode(pin_recover, OUTPUT);
  pinMode(pin_battery_state, INPUT);
  
  Serial.begin(9600);
  
  eraze_display();
  displayT1T2();
  test_sequence();
  battery_check();
  external_recover();
  
  digitalWrite(pin_led_V, HIGH);
  recover_servo.attach(pin_recover);


  eraze_display();
  displayT1T2();
  delay(2000);
  test_sequence();
  eraze_display();
  lcd.setCursor(0,0);
  lcd.print("Prêt !");
  
}

void displayT1T2(){
  lcd.setCursor(0, 0);
  lcd.print("T1 :");
  lcd.setCursor(4,0);
  lcd.print(T1);
  lcd.setCursor(9,0);
  lcd.print("ms");
  lcd.setCursor(0, 1);
  lcd.print("T2 :");
  lcd.setCursor(4,1);
  lcd.print(T2);
  lcd.setCursor(9,1);
  lcd.print("ms");
}

void test_sequence(){
  int r = 0;
  //test exp
  eraze_display();
  lcd.setCursor(0,0);
  lcd.print("Test Exp ...");
  analogWrite(pin_exp, 1023);
  for(int i = 0; i <10; i++){
    
    if (Serial.available() ){
      r = Serial.read();
      if (r = ExpReadyCode) {
        lcd.setCursor(0,1);
        lcd.print("OK !");
        break;}
    }
    delay(50);
  }
  analogWrite(pin_exp, 0);
  if (r != ExpReadyCode){
  lcd.setCursor(0,1);
  lcd.print('Exp fail !');
  }
  

  eraze_display();
  lcd.setCursor(0,0);
  lcd.print('Test LED ...');
  digitalWrite(pin_led_O, HIGH);
  digitalWrite(pin_led_R, HIGH);
  delay(1000);
  digitalWrite(pin_led_O, LOW);
  digitalWrite(pin_led_R, LOW);
  lcd.setCursor(0,1);
  lcd.print("OK !");
  
  //test telem
  eraze_display();
  lcd.setCursor(0,0);
  lcd.print("Test Telem ...");
  analogWrite(pin_telem, 1023);
  for(int i = 0; i <10; i++){
    
    if (Serial.available() ){
      r = Serial.read();
      if (r = TelemReadyCode) {
        lcd.setCursor(0,1);
        lcd.print("OK !");
        break;}
    }
    delay(50);
  }
  analogWrite(pin_telem, 0);
  if (r != TelemReadyCode){
  lcd.setCursor(0,1);
  lcd.print('Telem fail !');
  }  
  
  
  //test buzzer
  eraze_display();
  lcd.setCursor(0,0);
  lcd.print('Test BUZZ ...');
  digitalWrite(pin_buzzer, HIGH);
  delay(1000);
  digitalWrite(pin_buzzer, LOW);
  lcd.setCursor(0,1);
  lcd.print("OK !");
  
  //test parachute
  eraze_display();
  lcd.setCursor(0,0);
  lcd.print('Test sep...');
  int secure = analogRead(pin_recover_ok);
  lcd.setCursor(0,1);
  if (secure <100) {
    lcd.print("OK !");
  }
 else {
   lcd.print("Fail !");
 } 
 delay(1000);
  

}

void eraze_display(){
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
}

boolean battery_check(){
  int level_battery = analogRead(pin_battery_state);
  float Abattery_actual = map(level_battery, 0, 1023, 0, 5);
  float battery_actual = Abattery_actual / 0.3125; //cf le pont diviseur
  return battery_actual > battery_limit_voltage; 
  
}

boolean external_recover(){
  return false;
}



void loop() {

  

  if (millis()%10000 ==0 and (not battery_check()) ) {  //on ne test pas tout le temps la batterie mais toutes les 10s
    warning_buzzer = false;   
  }

  if (millis()%10000 ==0 and battery_check() ) { // on verifie de temps en temps si la batterie est chargée
    warning_buzzer = true;
  }




  if (not flight_in_progress and analogRead(pin_jack) > 100 ) { // décollage détécté
    flight_in_progress = true;
    digitalWrite(pin_exp, HIGH);
    digitalWrite(pin_telem, HIGH);
    digitalWrite(pin_led_O, HIGH);
    time_up = millis();
  }
  
  if ((time_up - millis() > T1) and (time_up - millis() < T2)) { //sauvegarde dans l'intervalle
    recover_in_progress = external_recover();
  }

  if (time_up - millis() >= T2) {//on force le deployement
    recover_in_progress = true;
  }

  if (recover_in_progress and not recover_done ){ // deployement
    recover_servo.write(servo_final_rotation);
    digitalWrite(pin_led_R, HIGH);
    recover_done = true;

  }

  if (recover_done) {
    if (analogRead(pin_recover_ok > 100)){// cas normal
      eraze_display();
      lcd.setCursor(0,0);
      lcd.print("Séparation OK !");

    }
    else{
      eraze_display();
      lcd.setCursor(0,0);
      lcd.print("ERREUR SEP !");
      warning_buzzer = true;
    }
  }
  

  if (warning_buzzer) { //alerte batterie ou recuperation
    if (millis() - date_millis_alert > delay_bip)  {
      buzzer_status = not buzzer_status;
      digitalWrite(pin_buzzer, buzzer_status);
      date_millis_alert = millis();
    }
  }
  
  
  
}

