
#include <Arduino.h>
#include "lecture_jauges.h"

lecture_jauges::jauges_a_lire(int pin_capteurs[], int nbr)
{
  int jauges[nbr];
for (int i=0;i<nbr;i++){
  pinMode(pin_capteurs[i], INPUT);
  jauges[i]=pin_capteurs[i];
  }
nbr_jauges=nbr;
}

lecture_jauges::etalonnage_jauges()
{
  int etalon_des_jauges[nbr_jauges];
    digitalWrite(alim, HIGH);
  for (int i=0; i<nbr_jauges ; i++) {
    etalon_des_jauges[i]=analogRead(jauges[i]);
    
    }
  digitalWrite(alim,LOW);
  }

lecture_jauges::alim_pont(int pin)
{
pinMode(pin, OUTPUT);
alim=pin;
}

void lecture_jauges::lire_jauges()
{
  int lecture[nbr_jauges];
  digitalWrite(alim, HIGH);
  for (int i=0; i<nbr_jauges ; i++) {
    lecture[i]=(analogRead(jauges[i])-etalon_des_jauges[i]);
    
    }
  digitalWrite(alim,LOW);
  return lecture; 
}
