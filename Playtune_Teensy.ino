// test program for the Teensy version of Playtune
// (The ABBA "money money money" score has some bugs, but it's ok for test purposes.)

#include "Playtune.h"
#include "money_2coils.h"
#include "puttritz.h"

void tune_playnote(byte chan, byte note);
void tune_stopnote(byte chan);
  
void setup(void) {
   tune_initchan(2);
   tune_initchan(3);
   tune_initchan(4);
   tune_initchan(5); 
   
   for (int note=0; note<128; ++note) {
    tune_playnote(0,note);
    delay(500);
    tune_stopnote(0);}
   }

void loop(void) {
   delay(1000);
   tune_playscore(puttritz);
   delay(2000);
   while (tune_playing) ;
   tune_playscore(money_2coils);
   while (tune_playing) ;
   delay(2000); }
