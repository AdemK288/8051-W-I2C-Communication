#include <8051.h>
#include "periphs.h"
#include "typedefs.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define pSDA P2_5
#define pSCL P2_6
#define clear_display                                0x01
#define goto_home                                    0x02          
#define cursor_direction_inc                         (0x04 | 0x02)
#define display_no_shift                             (0x04 | 0x00)
#define display_on                                   (0x08 | 0x04)
#define cursor_off                                   (0x08 | 0x00)
#define blink_off                                    (0x08 | 0x00)                              
#define _4_pin_interface                             (0x20 | 0x00)
#define _2_row_display                               (0x20 | 0x08)
#define _5x7_dots                                    (0x20 | 0x00)
#define BL_ON                                         1
#define BL_OFF                                        0
#define DAT                                           1
#define CMD                                           0

void LCD_init(void);
void LCD_toggle_EN(void);
void LCD_send(unsigned char value, unsigned char mode); 
void LCD_4bit_send(unsigned char lcd_data);            
void LCD_putstr(char *lcd_string);
void LCD_putchar(char char_data);
void LCD_clear_home(void);
void LCD_goto(unsigned char x_pos, unsigned char y_pos); 
static unsigned char bl_state;
static unsigned char data_value;
  

void delayI2C(unsigned char x){
    pSDA = x;
}
void delayClock(unsigned char x ){
    pSCL = x;
}

void delayms(unsigned char ms)
{
	unsigned char i;

	while (ms--) {
		for (i = 0; i < 120; i++);
	}
}

void initi2cLCD(){
    unsigned char address = 0x4F;
    pSCL = 1;   
    pSDA = 0;
    pSCL = 0;
    unsigned int mask;
    for (mask = 0x80; mask != 0; mask >>= 1){
        delayClock(0);
        if (address & mask){
            delayI2C(1);
        }
        else {
            delayI2C(0);
        }
        delayClock(1);
    }
    delayClock(0);
    pSDA = 1;
    delayClock(1);
}
void initi2cWLCD(){
    unsigned char address = 0x4E;
    pSCL = 1;   
    pSDA = 0;
    pSCL = 0;
    unsigned int mask;
    for (mask = 0x80; mask != 0; mask >>= 1){
        delayClock(0);
        if (address & mask){
            delayI2C(1);
        }
        else {
            delayI2C(0);
        }
        delayClock(1);
    }
    delayClock(0);
    delayI2C(0);
    delayClock(1);
}



void write_i2cLCD(unsigned char t){
    initi2cWLCD();

    unsigned int mask;
    for (mask = 0x80; mask != 0; mask >>= 1){
        delayClock(0);
        if (t & mask){
            delayI2C(1);
        }
        else {
            delayI2C(0);
        }
        delayClock(1); 
    }
    delayClock(0);
    delayI2C(1);
    delayClock(1);
    delayClock(0);
    delayClock(1);
    delayI2C(1);
}






unsigned char read_i2cLCD(){
    initi2cLCD();
    unsigned char i, data = 0;
    for (i = 0; i < 8; i++){
        delayClock(0);
        if (pSDA) {
            data |= 1;
        }
        if (i < 7){
            data <<= 1;
        }
        delayClock(1);
    }
    delayClock(0);
    delayI2C(0);
    delayClock(1);
    delayClock(0);
    delayClock(1);
    delayI2C(1);
    delayClock(0);
    delayClock(1);
    delayI2C(1);

    return data;
    
}


void LCD_init(void)
 {                       
   delayms(10);
   bl_state = BL_ON;
   data_value = 0x04;
   write_i2cLCD(data_value);
   delayms(10);
   LCD_send(0x33, CMD);
   LCD_send(0x32, CMD);
   LCD_send((_4_pin_interface | _2_row_display | _5x7_dots), CMD);         
   LCD_send((display_on | cursor_off | blink_off), CMD);     
   LCD_send((clear_display), CMD);         
   LCD_send((cursor_direction_inc | display_no_shift), CMD);        
 }   
  
void LCD_toggle_EN(void)
 {
   data_value |= 0x04;
   write_i2cLCD(data_value);
   delayms(1);
   data_value &= 0xF9;
   write_i2cLCD(data_value);
   delayms(1);
 }
    
  
void LCD_send(unsigned char value, unsigned char mode)
 {
   switch(mode)
   {
      case CMD:
      {
         data_value &= 0xF4;
         break;
      }
      case DAT:
      {
         data_value |= 0x01;
         break;
      }
   }
  
   switch(bl_state)
   {
      case BL_ON:
      {
         data_value |= 0x08;
         break;
      }
      case BL_OFF:
      {
         data_value &= 0xF7;
         break;
      }
   }
    write_i2cLCD(data_value);
   LCD_4bit_send(value);
   delayms(1);
 }
     
  
void LCD_4bit_send(unsigned char lcd_data)       
 {
   unsigned char temp = 0x00;
  
   temp = (lcd_data & 0xF0);
   data_value &= 0x0F;
   data_value |= temp;
   //PCF8574_write(data_value);
    write_i2cLCD(data_value);
   LCD_toggle_EN();
  
   temp = (lcd_data & 0x0F);
   temp <<= 0x04;
   data_value &= 0x0F;
   data_value |= temp;
   //PCF8574_write(data_value);
    write_i2cLCD(data_value);
   LCD_toggle_EN();
 }  
  

void LCD_putstr(char *lcd_string)
 {
   do
   {
     LCD_putchar(*lcd_string++);
   }while(*lcd_string != '\0') ;
 }
  

void LCD_putchar(char char_data)
 {
   if((char_data >= 0x20) && (char_data <= 0x7F))
   {
     LCD_send(char_data, DAT);
   }
 }
  
void LCD_clear_home(void)
 {
   LCD_send(clear_display, CMD);
   LCD_send(goto_home, CMD);
 }
  

void LCD_goto(unsigned char x_pos,unsigned char y_pos)
 {                                                   
   if(y_pos == 0)    
   {                              
     LCD_send((0x80 | x_pos), CMD);
   }
   else 
   {                                              
     LCD_send((0x80 | 0x40 | x_pos), CMD); 
   }
 } 




void initi2c(){
    unsigned char address = 0x41;
    pSCL = 1;   
    pSDA = 0;
    pSCL = 0;
    //unsigned char address = 0x41;
    unsigned int mask;
    for (mask = 0x80; mask != 0; mask >>= 1){
        delayClock(0);
        //delayClock(1);
        if (address & mask){
            delayI2C(1);
        }
        else {
            delayI2C(0);
        }
        delayClock(1);
    }
    delayClock(0);
    pSDA = 1;
    delayClock(1);
}
void initi2cW(){
    unsigned char address = 0x40;
    pSCL = 1;   
    pSDA = 0;
    pSCL = 0;
    //unsigned char address = 0x41;
    unsigned int mask;
    for (mask = 0x80; mask != 0; mask >>= 1){
        delayClock(0);
        //delayClock(1);
        if (address & mask){
            delayI2C(1);
        }
        else {
            delayI2C(0);
        }
        delayClock(1);
    }
    delayClock(0);
    delayI2C(0);
    delayClock(1);
}


void write_i2c(unsigned char t){
    initi2cW();

    unsigned int mask;
    for (mask = 0x80; mask != 0; mask >>= 1){
        delayClock(0);
        //delayClock(1);
        if (t & mask){
            delayI2C(1);
        }
        else {
            delayI2C(0);
        }
        delayClock(1); 
    }
    delayClock(0);
    delayI2C(1);
    delayClock(1);
    delayClock(0);
    delayClock(1);
    delayI2C(1);
}




unsigned char read_i2c(){
    initi2c();
    unsigned char i, data = 0;
    for (i = 0; i < 8; i++){
        delayClock(0);
        //delayClock(1);
        if (pSDA) {
            data |= 1;
        }
        if (i < 7){
            data <<= 1;
        }
        delayClock(1);
    }
    delayClock(0);
    delayI2C(0);
    delayClock(1);
    delayClock(0);
    delayClock(1);
    delayI2C(1);
    delayClock(0);
    delayClock(1);
    delayI2C(1);

    return data;
    
}



int digits[4] = {0, 0, 0, 0};
__code uchar table[] = { 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82,
	0xf8, 0x80, 0x90
};



void delay(unsigned char t)
{
	while (t--);
}



void display(void)
{
	P0 = table[digits[3]];
	P2_0 = 0;
	delayms(1);
	P2_0 = 1;

	P0 = table[digits[2]];
	P2_1 = 0;
	delayms(1);
	P2_1 = 1;

	P0 = table[digits[1]];
	P2_2 = 0;
	delayms(1);
	P2_2 = 1;

	P0 = table[digits[0]];
	P2_3 = 0;
	delayms(1);
	P2_3 = 1;
}
void incrementDigit(){
   display();
   digits[0] += 1;
    if (digits[0] > 9){
        digits[0] = 0;
        digits[1] += 1;

    }
    if (digits[1] > 9){
        digits[1] = 0;
        digits[0] = 0;
        digits[2] += 1;
    }
    if (digits[2] > 9){
        digits[2] = 0;
        digits[1] = 0;
        digits[0] = 0;
        digits[3] += 1;
    }

   if (digits[3] > 9) {
    for(int i = 0; i < 4; i++){
        digits[i] = 0;
    }

    }
    display();
}
void decrementDigit(){
    display();
    digits[0] -= 1;
    if (digits[0] < 0){
        digits[0] = 9;
        digits[1] -= 1;

    }
    if (digits[1] < 0){
        digits[1] = 9;
        digits[2] -= 1;
    }
    if (digits[2] < 0){
        digits[2] = 9;
        digits[3] -= 1;
    }

   if (digits[3] < 0) {
    for(int i = 0; i < 4; i++){
        digits[i] = 9;
    }

    }
    display();

}


unsigned int val = 0;
unsigned int inc = 3;
unsigned int dualCy0 = 0b10000000;
unsigned int dualCy1 = 0b00000001;
unsigned int option = 0;

int rev = 0;

/*void main (){

    while (1) { 
    delayms(2000);
    unsigned char toWrite;
    unsigned char dat = read_i2c();
    //delayms(100);
    unsigned char mask = 0x01;
    unsigned char bits[8];
    for(int n = 7;n >= 0;n--) {
        bits[n] = dat & mask; 
        dat = dat >> 1;
    }

    if (bits[0] == 0){
        toWrite = 0b11110111;
        //write_i2c(toWrite);

    }
    //delayms(100);
    }
}*/

void main(void)
{
   //char i = 0x00;
   LCD_init();
   LCD_clear_home();
   LCD_goto(0, 0);
   LCD_putstr("Hello World");
   LCD_goto(1, 1);
   LCD_putstr("hello");

//int val = 0;
	//P1 = 0xff; //port for LED copntrols
    unsigned char toWrite;
    while (1){
    unsigned int result = dualCy0 | dualCy1;
    unsigned char dat = read_i2c();
    //delayms(100);
    unsigned char mask = 0x01;
    unsigned char bits[8];
    for(int n = 7;n >= 0;n--) {
        bits[n] = dat & mask; 
        dat = dat >> 1;
    }
    if (bits[1] == 0){
        toWrite = 0b11111101;
        write_i2c(toWrite);
        LCD_clear_home();
        LCD_goto(0, 0);
        LCD_putstr("Button 2 Pressed");
    }
    if (bits[2] == 0){
        toWrite = 0b11111011;
        write_i2c(toWrite);
        LCD_clear_home();
        LCD_goto(0, 0);
        LCD_putstr("Button 3 Pressed");
    }
    if (bits[3] == 0) {
        toWrite = 0b11110111;
        write_i2c(toWrite);
        LCD_clear_home();
        LCD_goto(0, 0);
        LCD_putstr("Button 4 Pressed");
    }
    if (bits[0] == 0){
        toWrite = 0b11111110;
        write_i2c(toWrite);
        LCD_clear_home();
        LCD_goto(0, 0);
        LCD_putstr("Button 1 Pressed");

    }
    //digits[0] = bits[0];
    //digits[1] = bits[1];
    //digits[2] = bits[2];
    //digits[3] = bits[3];
    //display();

    if (K1 == 0){
    delayms(100);
    option += 1;
    }
    
    switch (option){
    case 0:
    delayms(100);
    if (val == 255){
        val = 0;
    }
    val += 1;
    P1_0 = !(( val & (1 << (0) ) ) ? 1 : 0 ); 
    P1_1 = !(( val & (1 << (1) ) ) ? 1 : 0 ); 
    P1_2 = !(( val & (1 << (2) ) ) ? 1 : 0 ); 
    P1_3 = !(( val & (1 << (3) ) ) ? 1 : 0 ); 
    P1_4 = !(( val & (1 << (4) ) ) ? 1 : 0 ); 
    P1_5 = !(( val & (1 << (5) ) ) ? 1 : 0 ); 
    P1_6 = !(( val & (1 << (6) ) ) ? 1 : 0 ); 
    P1_7 = !(( val & (1 << (7) ) ) ? 1 : 0 ); 
    break;
    case 1:
    delayms(100);
    if (inc >= 192){
    inc = 3;
    }
    P1_0 = !(( inc & (1 << (0) ) ) ? 1 : 0 ); 
    P1_1 = !(( inc & (1 << (1) ) ) ? 1 : 0 ); 
    P1_2 = !(( inc & (1 << (2) ) ) ? 1 : 0 ); 
    P1_3 = !(( inc & (1 << (3) ) ) ? 1 : 0 ); 
    P1_4 = !(( inc & (1 << (4) ) ) ? 1 : 0 ); 
    P1_5 = !(( inc & (1 << (5) ) ) ? 1 : 0 ); 
    P1_6 = !(( inc & (1 << (6) ) ) ? 1 : 0 ); 
    P1_7 = !(( inc & (1 << (7) ) ) ? 1 : 0 ); 
    inc += inc;
    break;
    case 2:
    delayms(100);
    if (result <= 24){
        //dualCy = 129;
        rev = 1;
    }
    else if(result >= 129){
        rev = 0;
    }
    P1_0 = !(( result & (1 << (0) ) ) ? 1 : 0 ); 
    P1_1 = !(( result & (1 << (1) ) ) ? 1 : 0 ); 
    P1_2 = !(( result & (1 << (2) ) ) ? 1 : 0 ); 
    P1_3 = !(( result & (1 << (3) ) ) ? 1 : 0 ); 
    P1_4 = !(( result & (1 << (4) ) ) ? 1 : 0 ); 
    P1_5 = !(( result & (1 << (5) ) ) ? 1 : 0 ); 
    P1_6 = !(( result & (1 << (6) ) ) ? 1 : 0 ); 
    P1_7 = !(( result & (1 << (7) ) ) ? 1 : 0 );
    if (rev == 1){
    dualCy0 = dualCy0 << 1;
    dualCy1 = dualCy1 >> 1;
    }
    else {
    dualCy0 = dualCy0 >> 1;
    dualCy1 = dualCy1 << 1;
    }
    break;
    case 3:
    display();
    if (K3 == 0){
       display();
       delayms(100);
       incrementDigit();
       //display();
       
    }
    if (K4 == 0){
       display();
       delayms(100);
       decrementDigit();
       //display();
    }   
    display();
    break;
}

}

    //val = 0;
} 
