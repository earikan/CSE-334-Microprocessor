#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

/*notes*/    
#define D3  81745  
#define F3  68739 
#define G3  61237 
#define G3S 57802 
#define B3  48604 
#define D4  40871
#define D4S 38576 
#define E4  36411  
#define F4  34367
#define F4S 32439
#define G4  30617
#define G4S 28900
#define A4  27278  
#define A4S 25746  
#define B4  24301 
#define C5  22937  
#define C5S 21649 
#define D5  20434
#define G5  15308
#define MYSLEEP  20


/*SMOKE ON THE WATER - DEEP PURPLE*/
unsigned long firstSongNotes[] = {D3, F3, G3, D3, F3, G3S, G3, D3, F3, G3, F3, D3, MYSLEEP, D3, F3, G3, D3, F3, G3S, G3, D3, F3, G3, F3, D3, 0};
unsigned long firstNotesDurations[] = {
                                  45, 45, 75, 45, 45, 25,  75, 45, 45, 75, 45, 75, 2,  45, 45, 75, 45, 45, 25,  75,  45, 45, 75, 45, 75
       };


/*FELSEFE TASI - HARRY POTTER*/
unsigned long secondSongNotes[] = {E4, G4, F4S, E4, B4,  A4, F4S, E4, G4, F4S, D4S, F4, B3, MYSLEEP ,E4, G4, F4S, E4, B4, D5, C5S, C5, A4, C5, B4, A4, F4S, E4, 0};
unsigned long secondNotesDurations[] = {
                                  45,  17, 35,  55, 35,  80, 80,  45, 17,  35, 75, 35,  100, 5, 45, 17, 35,  55, 35, 70, 35,  75, 45, 17, 35, 75,  35, 60
       };


/*REQUIEM FOR A DREAM*/
unsigned long thirdSongNotes[] = {A4S,  A4, G4,  D4, A4S, A4,  G4,  D4,  A4S,  A4,  G4, D4,  C5, A4S,  A4,  A4S, A4S,  A4, G4,  D4, A4S, A4,  G4,  D4,  A4S,  A4,  G4, D4,  C5, A4S,  A4,  A4S, 0};
unsigned long thirdNotesDurations[] = {
                                  55,  55,  55,  55, 55,  55,  55,  55,  55,   55,  55,  55, 55,  55,  55,  55,  55,  55, 55,  55,  55, 55,  55,  55,   55,  55,  55,  55,  55,  55,  55,   55
       };


/*variables*/
unsigned long Tcount;
unsigned long firstSong5sIndex;
unsigned long secondSong5sIndex;
unsigned long thirdSong5sIndex;


/*functions*/
void playFirstSong5s();
void playSecondSong5s();
void playThirdSong5s();
void playNote(unsigned long note,unsigned long duration, int number);
void myDelay(unsigned long time);
void playFirstSong();
void playSecondSong();
void playThirdSong();
void showSegment(int number);


/*main*/
void main(void)
{
   
   DDRT  = 0x20;
   TSCR1 = 0x80; 
   TSCR2 = 0x02;  
   TCTL1 = 0x04;  
   DDRB  = 0xFF;
   PORTB = 0;
   
   DDRH = 0x00;
   
   myDelay(1000);


    /*when song plays, you can use PTH switches for change song*/
    for(;;){
      
      if((0^PTH) == 0x00){
           
         playFirstSong5s();
         myDelay(200);
        
         playSecondSong5s();
         myDelay(200);
        
         playThirdSong5s();
         
      } else if((1^PTH) == 0x00){  
         playFirstSong();
      } else if((2^PTH) == 0x00){     
         playSecondSong();  
      } else if((3^PTH) == 0x00) {
         playThirdSong();        
      }

      myDelay(300);

    }
    
    
    EnableInterrupts;
    
    for(;;){
      
    }
}
      


/*first song*/
void playFirstSong(){

  int j = 0;
  TIOS = 0x20;  

     while(firstSongNotes[j]){
        if(firstSongNotes[j] == MYSLEEP){
           TIOS  &= 0xDF;
           myDelay(1000);
           TIOS = 0x20;  
           ++j;
        }
        
        playNote(firstSongNotes[j]/4, firstNotesDurations[j], 1);     
        ++j;
    }
    
    TIOS  &= 0xDF; 

}


/*second song*/
void playSecondSong(){

  int j = 0;
  TIOS = 0x20;  

     while(secondSongNotes[j]){
        if(secondSongNotes[j] == MYSLEEP){
           TIOS  &= 0xDF;
           myDelay(600);
           TIOS = 0x20;  
           ++j;
        }
        
        playNote(secondSongNotes[j]/4, secondNotesDurations[j], 2);      
        ++j;
    }
    
    TIOS  &= 0xDF; 

}


/*third song*/
void playThirdSong(){

  int j = 0;
  TIOS = 0x20;  

     while(thirdSongNotes[j]){
        if(thirdSongNotes[j] == MYSLEEP){
           TIOS  &= 0xDF;
           myDelay(100);
           TIOS = 0x20;  
           ++j;
        }
        
        playNote(thirdSongNotes[j]/4, thirdNotesDurations[j], 3);   
        ++j;
    }
    
    TIOS  &= 0xDF; 

}


/*play note*/
void playNote(unsigned long note,unsigned long duration, int number){
   
   while(duration !=0){
   
     showSegment(number);
     Tcount = TCNT;
     Tcount = Tcount + note; 
     TC5 = Tcount;
     while (!(TFLG1 & 0x20));
     TFLG1 = TFLG1 | 0x20;
      
     if(TFLG2 & 0x80){
        duration--;
        TFLG2 = 0x80;
     }
     
    PORTB=0xFF;  
   }

   myDelay(30);
}


/*5s play first song*/
void playFirstSong5s(){

    int i = 11;
    int j = 0;
    TIOS = 0x20;  

     while(j < i){
        
        if(firstSongNotes[j] == MYSLEEP){
           TIOS  &= 0xDF;
           myDelay(1000);
           TIOS = 0x20;  
           ++j;
        }
        
        playNote(firstSongNotes[j]/4, firstNotesDurations[j], 1);      
        ++j;
    }
    
    TIOS  &= 0xDF; 

}


/*5s play second song*/
void playSecondSong5s(){
  
     int i = 13; 
     int j = 0;
     TIOS = 0x20;  

     while(j < i){
        
        if(secondSongNotes[j] == MYSLEEP){
           TIOS  &= 0xDF;
           myDelay(600);
           TIOS = 0x20;  
           ++j;
        }
        
        playNote(secondSongNotes[j]/4, secondNotesDurations[j], 2);     
        ++j;
    }
    
    TIOS  &= 0xDF; 
  
}


/*5s play third song*/
void playThirdSong5s(){
  
     int i = 15; 
     int j = 0;
     TIOS = 0x20;  

     while(j < i){
        
        if(thirdSongNotes[j] == MYSLEEP){
           TIOS  &= 0xDF;
           myDelay(100);
           TIOS = 0x20;  
           ++j;
        }
        
        playNote(thirdSongNotes[j]/4, thirdNotesDurations[j], 3);      
        ++j;
    }
    
    TIOS  &= 0xDF; 

}


/*delay*/
void myDelay(unsigned long time) 
{
    unsigned int i; 
    unsigned int j;
    
    for(i=0;i<time;i++)
      for(j=0;j<3999;j++);
}


/*segment*/
void showSegment(int number){

      DDRP = 0x0F;
      DDRB  = 0xFF;
      PTP = 0x07;
      
      
      if(number == 0)
          PORTB = 0xFE; 
      else if(number == 1)
         PORTB = 0x06; 
      else if(number == 2)
         PORTB = 0x5B; 
      else if(number == 3)
         PORTB = 0x4F; 

}


