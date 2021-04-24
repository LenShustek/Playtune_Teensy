
     Playtune: An Arduino polyphonic music generator
     Teensy 3.1/3.2 version

                         About Playtune, generally

   Playtune is a family of music players for Arduino-like microcontrollers. They
   each intepret a bytestream of commands that represent a polyphonic musical
   score, and play it using different techniques.

   (1) The original Playtune, first released in 2011, uses a separate hardware timer
   to generate a square wave for each note played simultaneously. The timers run at twice
   the frequency of the note being played, and the interrupt routine flips the output bit.
   It can play only as many simultaneous notes as there are timers available. The sound
   quality? Buzzy square waves.
   https://github.com/LenShustek/arduino-playtune

   (2) The second ("polling") version uses only one hardware timer that interrupts often,
   by default at 20 Khz, or once every 50 microseconds. The interrupt routine determines
   which, if any, of the currently playing notes need to be toggled. It also implements
   primitive volume modulation by changing the duty cycle of the square wave.
   The advantage over the first version is that the number of simultaneous notes is not
   limited by the number of timers, only by the number of output pins. The sound quality
   is still "buzzy square waves".
   https://github.com/LenShustek/playtune_poll

   (3) The third version also uses only one hardware timer interrupting frequently, but
   uses the hardware digital-to-analog converter on high-performance microntrollers like
   the Teensy to generate an analog wave that is the sum of stored samples of sounds. The
   samples are scaled to the right frequency and volume, and any number of instrument
   samples can be used and mapped to MIDI patches. The sound quality is much better,
   although not in league with real synthesizers. It currently only supports Teensy.
   https://github.com/LenShustek/playtune_samp

   (4) The fourth version is an audio object for the PJRC Audio Library.
   https://www.pjrc.com/teensy/td_libs_Audio.html
   It allows up to 16 simultaneous sound generators that are internally mixed, at
   the appropriate volume, to produce one monophonic audio stream.
   Sounds are created from sampled one-cycle waveforms for any number of instruments,
   each with its own attack-hold-decay-sustain-release envelope. Percussion sounds
   (from MIDI channel 10) are generated from longer sampled waveforms of a complete
   instrument strike. Each generator's volume is independently adjusted according to
   the MIDI velocity of the note being played before all channels are mixed.
   www.github.com/LenShustek/Playtune_synth

   (5) This fifth version is for the Teensy 3.1/3.2, and uses the four Periodic Interval
   Timers in the Cortex M4 processor to support up to 4 simultaneous notes.
   It uses less CPU time than the polling version, but is limited to 4 notes at a time.
   (This was written to experiment with multi-channel multi-Tesla Coil music playing,
   where I use Flexible Timer Module FTM0 for generating precise one-shot pulses.
   But I ultimately switched to the polling version to play more simultaneous notes.)
   www.github.com/LenShustek/Playtune_Teensy

   For all versions, once a score starts playing, the processing happens in
   the interrupt routine.  Any other "real" program can be running at the same time
   as long as it doesn't use the timers or the output pins that Playtune is using.

   Each timer (tone generator) can be associated with any digital output pin,
   not just the pins that are internally connected to the timer. The exception is
   when the Teensy version is compiled for playing music on Tesla coils, in which
   case the outputs must be on digital pins 20, 21, 22, and 23.

   The easiest way to hear the music is to connect each of the output pins to a resistor
   (500 ohms, say).  Connect other ends of the resistors together and then to one
   terminal of an 8-ohm speaker.  The other terminal of the speaker is connected to
   ground.  No other hardware is needed!  But using an amplifier is nicer.

  ****  The public Playtune interface  ****

   There are five public functions and one public variable.

   void tune_initchan(byte pin)

    Call this to initialize each of the tone generators you want to use.  The argument
    says which pin to use as output.  The pin numbers are the digital "D" pins
    silkscreened on the Arduino board.  Calling this more times than your processor
    has timers will do no harm, but will not help either.

   void tune_playscore(byte *score)

    Call this pointing to a "score bytestream" to start playing a tune.  It will
    only play as many simultaneous notes as you have initialized tone generators;
    any more will be ignored.  See below for the format of the score bytestream.

   boolean tune_playing

    This global variable will be "true" if a score is playing, and "false" if not.
    You can use this to see when a score has finished.

   void tune_stopscore()

    This will stop a currently playing score without waiting for it to end by itself.

   void tune_delay(unsigned int msec)

    Delay for "msec" milliseconds.  This is provided for compatibility with other versions
    of Playtune that break the "delay' function if you use all of your processor's
    timers for generating tones. (This one doesn't, and delay() works fine.)

   void tune_stopchans()

    This disconnects all the timers from their pins and stops the interrupts.
    Do this when you don't want to play any more tunes.

   void tune_speed(unsigned int percent)

    New for the Teensy version, this changes playback speed to the specified percent
    of normal. The minimum is percent=20 (1/5 slow speed) and the maximum is
    percent=500 (5x fast speed).

   *****  The score bytestream  *****

   The bytestream is a series of commands that can turn notes on and off, and can
   start a waiting period until the next note change.  Here are the details, with
   numbers shown in hexadecimal.

   If the high-order bit of the byte is 1, then it is one of the following commands:

     9t nn  Start playing note nn on tone generator t.  Generators are numbered
            starting with 0.  The notes numbers are the MIDI numbers for the chromatic
            scale, with decimal 60 being Middle C, and decimal 69 being Middle A
            at 440 Hz.  The highest note is decimal 127 at about 12,544 Hz. except
            that percussion notes (instruments, really) range from 128 to 255 when
            relocated from track 9 by Miditones with the -pt option. This version of
            Playtune ignores those percussion notes.

      [vv]  If ASSUME_VOLUME is set to 1, or the file header tells us to,
            then we expect a third byte with the volume ("velocity") value from 1 to
            127. You can generate this from Miditones with the -v option.
            (Everything breaks for headerless files if the assumption is wrong!)
            This version of Playtune ignores volume information.

     8t     Stop playing the note on tone generator t.

     Ct ii  Change tone generator t to play instrument ii from now on.  Miditones will
            generate this with the -i option. This version of Playtune gives that to
            the Tesla coil routine, if so compiled, so it can decide which coil(s)
            to play the note on.

     F0     End of score: stop playing.

     E0     End of score: start playing again from the beginning.

   If the high-order bit of the byte is 0, it is a command to wait.  The other 7 bits
   and the 8 bits of the following byte are interpreted as a 15-bit big-endian integer
   that is the number of milliseconds to wait before processing the next command.
   For example,

     07 D0

   would cause a wait of 0x07d0 = 2000 decimal millisconds or 2 seconds.  Any tones
   that were playing before the wait command will continue to play.

   Playtune bytestream files generated by later versions of the Miditones progam using
   the -d option begin with a small header that describe what optional data is present
   in the file. This makes the file more self-describing, and this version of Playtune
   uses that if it is present.

    'Pt'   2 ascii characters that signal the presence of the header
     nn    The length (in one byte) of the entire header, 6..255
     ff1   A byte of flag bits, three of which are currently defined:
               80 velocity information is present
               40 instrument change information is present
               20 translated percussion notes are present
     ff2    Another byte of flags, currently undefined
     tt     The number (in one byte) of tone generators actually used in this music.
            We use that the scale the volume when combining simulatneous notes.

   Any subsequent header bytes covered by the count, if present, are currently undefined
   and are ignored.The score is stored in Flash memory ("PROGMEM") along with the program, because
   there's a lot more of that than data memory.


   ****  Where does the score data come from?  ****

   Well, you can write the score by hand from the instructions above, but that's
   pretty hard.  An easier way is to translate MIDI files into these score commands,
   and I've written a program called "Miditones" to do that.  See the separate
   documentation for that program, which is also open source at
   https://github.com/lenshustek/miditones

   ****  Nostalgia from me  ****

   Writing Playtune was a lot of fun, because it essentially duplicates what I did
   as a graduate student at Stanford University in about 1973.  That project used the
   then-new Intel 8008 microprocessor, plus three hardware square-wave generators that
   I built out of 7400-series TTL.  The music compiler was written in Pascal and read
   scores that were hand-written in a notation I made up, which looked something like
   this:  C  Eb  4G  8G+  2R  +  F  D#
   This was before MIDI had been invented, and anyway I wasn't a pianist so I would
   not have been able to record my own playing.  I could barely read music well enough
   to transcribe scores, but I slowly did quite a few of them. MIDI is better!

   Len Shustek, originally 4 Feb 2011, then updated occasionally over the years.
