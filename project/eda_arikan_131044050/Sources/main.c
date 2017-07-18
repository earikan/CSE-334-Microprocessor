#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <stdlib.h>



//LCD DEFINES
#define LCD_DAT PORTK 
#define LCD_DIR DDRK
#define LCD_E   0x02
#define LCD_RS  0x01
#define CMD     0
#define DATA    1



//BUZZER NOTES
#define D3  81745  
#define F3  68739 
#define G3  61237 

unsigned long firstSongNotes[] = {D3, F3, G3, 0};
unsigned long firstNotesDurations[] = { 45, 45, 75, 45 };



//GAME LEVELS
#define EASY    1
#define MEDIUM  2
#define HARD    3

unsigned int TOTALDELAY = 100;



//FOR SERIAL COMMUNICATION 
const char menu[] = "Choose a level: 1.Easy 2.Medium \r\n\0";
const char option1[] = "Your choise is Easy! \r\n\0";  
const char option2[] = "Your choise is Medium! \r\n\0";  



//RANDOM NUMBER ARRAY
const int arraySize = 10;
int gameArray[15];



//WAIT TIME
unsigned long Tcount;
unsigned char delayTime;
void wait(int time);




//LCD Code Funtion Prototypes
void openlcd(void);              /* Initialize LCD display */
void put2lcd(char c, char type); /* Write command or data to LCD controller */
void puts2lcd (char *ptr);       /* Write a string to the LCD display */
 



//GAME MODULES
int chooseLevel(void);
void showGameStartMessage();
void fillRandomArray(int level);
void segment(int a, int index);
void number(int a);
void showGameStartMessage();
void playGame(int level);
void playNote(unsigned long note,unsigned long duration);
void takeInputForEasy(unsigned char twoDigitVerify[15][2], int counter);
int compareForEasy(unsigned char array[15][2], unsigned char digitVerifyArray[15][2], int digit, int counter);
void mSDelay(unsigned int itime);
int compareForMedium(unsigned char array[15][3], unsigned char digitVerifyArray[15][3], int digit, int counter) ;
void takeInputForMedium(unsigned char twoDigitVerify[15][3], int counter);
void mediumInitilaze();
void easyInitilaze();
void playFirstSong(void);



//KEYPAD
const unsigned char keypad[4][4] =
{
'1','2','3','A',
'4','5','6','B',
'7','8','9','C',
'*','0','#','D'
};
unsigned char column,row;




//OPEN LCD
void openlcd(void)
{
  LCD_DIR = 0xFF;     /* configure LCD_DAT port for output */
  mSDelay(100);       /* Wait for LCD to be ready */
  put2lcd(0x28, CMD); /* set 4-bit data, 2-line display, 5x7 font */
  put2lcd(0x0F, CMD); /* turn on display, cursor, blinking */
  put2lcd(0x06, CMD); /* move cursor right */
  put2lcd(0x01, CMD); /* clear screen, move cursor to home */
  mSDelay(2);         /* wait until "clear display" command complete */
}




//LCD CODE
void puts2lcd (char *ptr)
{
  while (*ptr) {        /* While character to send */
    put2lcd(*ptr, DATA);/* Write data to LCD */
    mSDelay(20);      /* Wait for data to be written */
    ptr++;              /* Go to next character */
  }
}




//LCD CODE
void put2lcd(char c, char type) 
{
  char c_lo, c_hi;
  c_hi = (c & 0xF0) >> 2;     /* Upper 4 bits of c */
  c_lo = (c & 0x0F) << 2;     /* Lower 4 bits of c */
  
  LCD_DAT &= (~LCD_RS);       /* select LCD command register */

  if (type == DATA)
    LCD_DAT = c_hi | LCD_RS;  /* output upper 4 bits, E, RS high */
  else
    LCD_DAT = c_hi;           /* output upper 4 bits, E, RS low */


  LCD_DAT |= LCD_E;           /* pull E signal to high */
  __asm(nop);                 /* Lengthen E */
  __asm(nop);
  __asm(nop);
  LCD_DAT &= (~LCD_E);        /* pull E to low */
  
  
  if (type == DATA)
    LCD_DAT = c_lo | LCD_RS;  /* output lower 4 bits, E, RS high */
  else
    LCD_DAT = c_lo;           /* output lower 4 bits, E, RS low */
  
  
  LCD_DAT |= LCD_E;           /* pull E to high */
  __asm(nop);                 /* Lengthen E */
  __asm(nop);
  __asm(nop);
  LCD_DAT &= (~LCD_E);        /*pull E to low */
  
  
  mSDelay(2);              /* Wait for command to execute */
}





//START GAME MESSAGE
void showGameStartMessage()
{
  char *msg1 = "  Number Mind";
  char *msg2 = "      Game";
  openlcd();                  // Initialize LCD display
  put2lcd(0x01,CMD);      
  puts2lcd(msg1);             // Send first line
  put2lcd(0xC0,CMD);          // move cursor to 2nd row, 1st column
  puts2lcd(msg2);             // Send second line
  put2lcd(0x0C,CMD);      
  

}




//CHOOSE LEVEL
int chooseLevel(void)
{
  int i = 0;
  unsigned char value = 0;
  
  SCI0BDH = 0x00;
  SCI0BDL = 26;
  SCI0CR1 = 0x00;
  SCI0CR2 = 0x0C; //open send and recieve
  
  /*SEND STRING - MENU*/ 
  while(i < menu[i]!='\0') 
  {  
      while(!(SCI0SR1 & SCI0SR1_TDRE_MASK));
      SCI0DRL = menu[i];
      ++i;      
  }
   
   
  /*RECIEVE CHOOSE FOR MENU*/  
  while(!(SCI0SR1 & SCI0SR1_RDRF_MASK));
      value = SCI0DRL;
      

  i = 0;    
  
  /*if user choise easy*/
  if((value - '0') == 1){
     while(i < option1[i]!='\0') {
    
        while(!(SCI0SR1 & SCI0SR1_TDRE_MASK));
        SCI0DRL = option1[i];
        ++i;
        
     }
     
     return EASY;
  }
  
  /*if user choise medium*/
  else if((value - '0') == 2){
      while(i < option2[i]!='\0') {
    
        while(!(SCI0SR1 & SCI0SR1_TDRE_MASK));
        SCI0DRL = option2[i];
        ++i;
        
     }
     
    return MEDIUM;
  } 
   
}




//PLAY GAME
void playGame(int level){

  if(level == EASY){
      fillRandomArray(EASY);
  }else if(level == MEDIUM){
      fillRandomArray(MEDIUM);
  }
}





//FILL GAME ARRAY WITH RANDOM NUMBERS
void fillRandomArray(int level){

  unsigned int seed;
   
  if(level == EASY){          /*generate numbers between 10 - 99 */
      
    for (seed = 1; seed <arraySize + 1; seed++) {
    
      gameArray[seed-1] = (int)(rand()%89)+10;
    }
    
  }else if(level == MEDIUM){  /*generate numbers between 100 - 999 */
  
    for (seed = 1; seed <arraySize + 1; seed++) {
    
      gameArray[seed-1] = rand()%899+100;
    }
    
  }

}




//NUMBERS SHOW SPEED SETTINGS
void speedSetting(){
  
  char *msg1 = "Set speed";
  
  DDRH = 0x00;   //PTH as input
  PIEH = 0xFF;   //enable PTH interrupt
  PPSH = 0x07;   //Make it Edge-Trig.
  
  __asm CLI;

  openlcd();                  // Initialize LCD display
  puts2lcd(msg1);             // Send first line

  
  mSDelay(1000);
 

}
    


//INTERRUPT
interrupt (((0x10000-Vporth)/2)-1) void PORTH_ISR(void)
{   
  int i = 0;
  char warning1[] = "Faster game x0.5 \r\n\0";
  char warning2[] = "Slower game x2.0 \r\n\0";
  char warning3[] = "Slower game x2.5 \r\n\0";
  DDRB = 0xFF;

  SCI0BDH = 0x00;
  SCI0BDL = 26;
  SCI0CR1 = 0x00;
  SCI0CR2 = 0x0C; //open send and recieve
  
  if((0x01^PTH) == 0x00) {
    
     while(i < warning1[i]!='\0') {
    
        while(!(SCI0SR1 & SCI0SR1_TDRE_MASK));
        SCI0DRL = warning1[i];
        ++i;
        
     }
     
     TOTALDELAY = 50;
      
  }
  else if((0x02^PTH) == 0x00) {
    
    while(i < warning2[i]!='\0') {
    
        while(!(SCI0SR1 & SCI0SR1_TDRE_MASK));
        SCI0DRL = warning2[i];
        ++i;
        
     }
     
    TOTALDELAY = 150; 
  }

  PIFH = PIFH | 0xFF;      //clear PTH Interupt Flags for the next round. Writing HIGH will clear the Interrupt flags
}   
  


//WAIT TIME
void wait(int time){
  
  int counter = 0;
      
  TSCR1 = 0x80;
  TSCR2 = 0x00;

  while(counter < time){
  
    TFLG2 = 0x80;
    
    while(!(TFLG2 & 0x80));
    counter++;

  }

}



//EASY MODE INITILAZE
void easyInitilaze(){
    
   int i;
   unsigned int a, k, m, n;
   int counter = 1;
   char *lostMessage = "Fail..";
   char *winMessage = "You win!";
   
   int flag = 0;
   
   int twoDigit[15][2];
   unsigned char twoDigitToChar[15][2]; 
   unsigned char twoDigitVerify[15][2];
   

    for( i = 0; i < arraySize; ++i){
          twoDigit[i][0] = gameArray[i]/10;   
          twoDigit[i][1] = gameArray[i]%10;
          
          twoDigitToChar[i][0] = twoDigit[i][0] + '0';
          twoDigitToChar[i][1] = twoDigit[i][1] + '0'; 
       }

       for( i = 0; i < arraySize; ++i){     
          for(a = 0; a < counter; ++a){ 
              for(k = 0; k < TOTALDELAY ; ++k ) {
                
                  for(m = 0; m < 200; ++m) {
                    segment(twoDigit[a][0], 2);
                  }
                  for(n = 0; n < 200; ++n) {
                    segment(twoDigit[a][1], 3);
                  }
                
                  DDRP = 0x0F;   //clean segment
                  PTP  = 0xFF;
             
              }
          
           }      
           
           //take user input and compare
            takeInputForEasy(twoDigitVerify, counter);
            if(compareForEasy(twoDigitToChar,twoDigitVerify, 2, counter) ==1){
              flag = 1; 
              break;
            }
           
           if(counter!=arraySize) 
            ++counter;       
       }
       
       if(flag == 1) {
           openlcd();                  // Initialize LCD display
           put2lcd(0x01,CMD);      
           puts2lcd(lostMessage); 
           playFirstSong();    
       } else {
           openlcd();                  // Initialize LCD display
           put2lcd(0x01,CMD);      
           puts2lcd(winMessage);  
       }

}


//MEDIUM MODE INITILZE
void mediumInitilaze(){

   int i;
   unsigned int a, k, m, n;
   int counter = 1;
   char *lostMessage = "Fail..";
   char *winMessage = "You win!";
   
   int flag = 0;
   
   int threeDigit[15][3];
   unsigned char threeDigitToChar[15][3]; 
   unsigned char threeDigitVerify[15][3];
  
  
    for( i = 0; i < arraySize; ++i){
          threeDigit[i][0] = gameArray[i]/100;
          threeDigit[i][1] = (gameArray[i]%100)/10;   
          threeDigit[i][2] = gameArray[i]%10;
          
          threeDigitToChar[i][0] = threeDigit[i][0] + '0';
          threeDigitToChar[i][1] = threeDigit[i][1] + '0'; 
          threeDigitToChar[i][2] = threeDigit[i][2] + '0';    
        
       }                                      
       
   
       for( i = 0; i < arraySize; ++i){     
          for(a = 0; a < counter; ++a){ 
              for(k = 0; k < TOTALDELAY ; ++k ) {
                
                  for(n = 0; n < 200; ++n) {
                    segment(threeDigit[a][0], 1);
                  }
                
                  for(m = 0; m < 200; ++m) {
                    segment(threeDigit[a][1], 2);
                  }
                  
                  for(n = 0; n < 200; ++n) {
                    segment(threeDigit[a][2], 3);
                  }
                  
    
                  DDRP = 0x0F;   //clean segment
                  PTP  = 0xFF;
             
              }
          
           }      
           
           //take user input and compare
            takeInputForMedium(threeDigitVerify, counter);
            if(compareForMedium(threeDigitToChar,threeDigitVerify, 3, counter) ==1){
              flag = 1; 
              break;
            }
           
           if(counter!=arraySize) 
            ++counter;       
       }
       
       if(flag == 1) {
           openlcd();                  // Initialize LCD display
           put2lcd(0x01,CMD);      
           puts2lcd(lostMessage);  
           playFirstSong();   
       } else {
           openlcd();                  // Initialize LCD display
           put2lcd(0x01,CMD);      
           puts2lcd(winMessage);  
       } 
      

}



//SHOW NUMBERS
void showAndTakeNumbers(int level){

   if(level == EASY){
      easyInitilaze();
   } else if(level == MEDIUM){  
     mediumInitilaze();    
   } 
   
}
  


//TAKE INPUT FROM USER
void takeInputForEasy(unsigned char twoDigitVerify[15][2], int counter){        
                  
   int i = 0;
   int j = 0;
   int flag = 0;
   DDRA = 0x0F;                           //MAKE ROWS INPUT AND COLUMNS OUTPUT
   
  
   while(1){                              //OPEN WHILE(1)
      do{                                 //OPEN do1
         PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
      }while(row == 0x00);                //WAIT UNTIL KEY PRESSED //CLOSE do1

      do{                                 //OPEN do2
         do{                              //OPEN do3
            mSDelay(1);                   //WAIT
            row = PORTA & 0xF0;           //READ ROWS
         }while(row == 0x00);             //CHECK FOR KEY PRESS //CLOSE do3
         
         mSDelay(15);                     //WAIT FOR DEBOUNCE
         row = PORTA & 0xF0;
      }while(row == 0x00);                //FALSE KEY PRESS //CLOSE do2

      while(1){                           //OPEN while(1)
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x01;                   //COLUMN 0 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 0
            column = 0;
            break;                        //BREAK OUT OF while(1)
         }
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x02;                   //COLUMN 1 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 1
            column = 1;
            break;                        //BREAK OUT OF while(1)
         }

         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x04;                   //COLUMN 2 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 2
            column = 2;
            break;                        //BREAK OUT OF while(1)
         }
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x08;                   //COLUMN 3 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 3
            column = 3;
            break;                        //BREAK OUT OF while(1)
         }
         row = 0;                         //KEY NOT FOUND
      break;                              //step out of while(1) loop to not get stuck
      }                                   //end while(1)

      

      if(row == 0x10){
         twoDigitVerify[i][j] = keypad[0][column] ;         //OUTPUT TO PORTB LED
      
      }
      else if(row == 0x20){
         twoDigitVerify[i][j] =keypad[1][column];
        
      }
      else if(row == 0x40){
         twoDigitVerify[i][j] =keypad[2][column] ;
         
      }
      else if(row == 0x80){
         twoDigitVerify[i][j] =keypad[3][column];
         
      }

      ++j;
        
      if(j % 2 == 0){
        
        j = 0;
        ++i;
      }
    
      
     if(i == counter)
        break;   

      do{
         mSDelay(15);
         PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
      }while(row != 0x00);                //MAKE SURE BUTTON IS NOT STILL HELD
      
   
   }                                      //CLOSE WHILE(1)
}                                        




//TAKE INPUT FROM USER
void  takeInputForMedium(unsigned char threeDigitVerify[15][3], int counter){    
                     
   int i = 0;
   int j = 0;
   int flag = 0;
   DDRA = 0x0F;                           //MAKE ROWS INPUT AND COLUMNS OUTPUT
   
  
   while(1){                              //OPEN WHILE(1)
      do{                                 //OPEN do1
         PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
      }while(row == 0x00);                //WAIT UNTIL KEY PRESSED //CLOSE do1

      do{                                 //OPEN do2
         do{                              //OPEN do3
            mSDelay(1);                   //WAIT
            row = PORTA & 0xF0;           //READ ROWS
         }while(row == 0x00);             //CHECK FOR KEY PRESS //CLOSE do3
         
         mSDelay(15);                     //WAIT FOR DEBOUNCE
         row = PORTA & 0xF0;
      }while(row == 0x00);                //FALSE KEY PRESS //CLOSE do2

      while(1){                           //OPEN while(1)
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x01;                   //COLUMN 0 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 0
            column = 0;
            break;                        //BREAK OUT OF while(1)
         }
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x02;                   //COLUMN 1 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 1
            column = 1;
            break;                        //BREAK OUT OF while(1)
         }

         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x04;                   //COLUMN 2 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 2
            column = 2;
            break;                        //BREAK OUT OF while(1)
         }
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x08;                   //COLUMN 3 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 3
            column = 3;
            break;                        //BREAK OUT OF while(1)
         }
         row = 0;                         //KEY NOT FOUND
      break;                              //step out of while(1) loop to not get stuck
      }                                   //end while(1)

      

      if(row == 0x10){
         threeDigitVerify[i][j] = keypad[0][column] ;         //OUTPUT TO PORTB LED
      
      }
      else if(row == 0x20){
         threeDigitVerify[i][j] =keypad[1][column];
        
      }
      else if(row == 0x40){
         threeDigitVerify[i][j] =keypad[2][column] ;
         
      }
      else if(row == 0x80){
         threeDigitVerify[i][j] =keypad[3][column];
         
      }

      ++j;
      
      if(j % 3 == 0){
        
        j = 0;
        ++i;
      }
    
      
     if(i == counter)
        break;   

      do{
         mSDelay(15);
         PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
      }while(row != 0x00);                //MAKE SURE BUTTON IS NOT STILL HELD
      
      
   }                                      //CLOSE WHILE(1)
}                                        



//COMPARE USER INPUT WITH GAME ARRAY
int compareForEasy(unsigned char array[15][2], unsigned char digitVerifyArray[15][2], int digit, int counter) {
  
 int flag = 0;
    int a,i;
  
      for(a = 0; a < counter; ++a){ 
        for(i = 0; i < digit; ++i){
           if(array[a][i] != digitVerifyArray[a][i])
             flag = 1;
        }
      }
    
     if(flag == 1)
      return 1;
     else
      return 0;

}


//COMPARE USER INPUT WITH GAME ARRAY
 int compareForMedium(unsigned char array[15][3], unsigned char digitVerifyArray[15][3], int digit, int counter) {
  
 int flag = 0;
    int a,i;
  
      for(a = 0; a < counter; ++a){ 
        for(i = 0; i < digit; ++i){
           if(array[a][i] != digitVerifyArray[a][i])
             flag = 1;
        }
      }
    
     if(flag == 1)
      return 1;
     else
      return 0;

}




//DELAY
void mSDelay(unsigned int itime){
unsigned int i; unsigned int j;
   for(i=0;i<itime;i++)
      for(j=0;j<4000;j++);
}



//SEVEN SEGMENT NUMBERS
void number(int a){
    DDRB= 0xFF;
      if(a == 1)
        PORTB = 0x06;
      else if(a == 2)
        PORTB = 0x5B;
      else if(a == 3)
        PORTB = 0x4F;
      else if(a == 4)
        PORTB = 0x66;
      else if(a == 5)
        PORTB = 0x6D;
      else if(a == 6)
        PORTB = 0x7D;
      else if(a == 7)
        PORTB = 0x07;
      else if(a == 8)
        PORTB = 0x7F;
      else if(a == 9)
        PORTB = 0x6F;
      
}


//SEVEN SEGMENT
void segment(int a, int index){

     DDRP = 0x0F;

   if(index == 0){
    
     PTP = 0x0E;   
      
     number(a);
   } else if(index == 1){
    
      PTP = 0x0D; 
      
       number(a);
   } else if(index == 2){
    
      
      PTP = 0x0B; 
      number(a);
   } else if(index == 3){
    
      
      PTP = 0x07;
     number(a);
   }
}

 
//FOR BUZZER
void playFirstSong(){

  int j = 0;
  TIOS = 0x20;  

     while(firstSongNotes[j]){

        playNote(firstSongNotes[j]/4, firstNotesDurations[j]);     
        ++j;
    }
    
    TIOS  &= 0xDF; 

}


//FOR BUZZER
void playNote(unsigned long note,unsigned long duration){
   
   while(duration !=0){
   
     
     Tcount = TCNT;
     Tcount = Tcount + note; 
     TC5 = Tcount;
     while (!(TFLG1 & 0x20));
     TFLG1 = TFLG1 | 0x20;
      
     if(TFLG2 & 0x80){
        duration--;
        TFLG2 = 0x80;
     }
     
  
   }

   mSDelay(30);
}




void main(void) {
  /* put your own code here */
   
   int level = 0;
   char *msg1 = "Lets Start!";
   
   DDRT  = 0x20;
   TSCR1 = 0x80; 
   TSCR2 = 0x00;  
   TCTL1 = 0x04;  
   DDRB  = 0xFF;
   PORTB = 0;
   DDRH = 0x00;
   
 	 EnableInterrupts;  

   srand(100);
    
   showGameStartMessage();
   playFirstSong();
   
   speedSetting();
   
   
   openlcd();                  // Initialize LCD display
   put2lcd(0x01,CMD);      
   puts2lcd(msg1);             // Send first line
   level = chooseLevel();
   playGame(level); 
   showAndTakeNumbers(level);

  
  for(;;) {
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
