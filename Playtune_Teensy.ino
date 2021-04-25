// test program for the Teensy version of Playtune

#include "Playtune.h"

// both samples have issues, as a result of retricting them to
// 4 channels, but I'm not a great MIDI editor!
#include "money_2coils.h"    // ABBA'S "Money Money Money"
#include "puttritz_4chan.h"  // Irving Berline's "Puttin' on the Ritz"

void tune_playnote(byte chan, byte note);
void tune_stopnote(byte chan);
  
void setup(void) {
   tune_initchan(2);
   tune_initchan(3);
   tune_initchan(4);
   tune_initchan(5); 
   
   for (int note=21; note<109; ++note) { // try all piano notes
    tune_playnote(0,note);
    delay(200);
    tune_stopnote(0);}
   }

void loop(void) {
   delay(1000);
   tune_playscore(puttritz_4chan);
   delay(2000);
   while (tune_playing) ;
   tune_playscore(money_2coils);
   while (tune_playing) ;
   delay(2000); }
