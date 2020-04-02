#ifndef lecture_jauges_h
#define lecture_jauges_h
#include <Arduino.h>

class lecture_jauges
{
  public:
  jauges_a_lire(int pin_capteurs[], int nbr);  //initialisation de pins des jauges à lire 
  alim_pont(int pin);                          //initialisation du pin pour alimenter les ponts 
  etalonnage_jauges();                         //etalonnage des jauges (tare)
  void lire_jauges();                          //fonction qui renvoie les valeurs des jauges sous forme d,une liste 
  private:                                     
  int alim;                                    //stocke le pin de l'alim du pont
  int jauges[];                                //stocke les pins ds jauges à lire 
  int nbr_jauges;                              //stocke le nombre de jauges à lire
  int etalon_des_jauges[];                     //stocke les valeurs initiales des jauges
  };
#endif 
