/*
 * Terminal.c
 *
 * Created: 5/30/2013 4:22:18 PM
 *  Author: mfolz
 

 Version 1.0
 ---------------
	- Beta build, kinda crappy but still functional 
1.2
	- Works better with the new rotation, made it Dependant on the devices current orientation. 
	- need to add functionality with float variables 
 */ 
#define F_CPU 16000000UL  // 16 MHz
#include "Terminal.h"
#include "../LCD_Libs/_SSD1289.h"
#include "../LCD_Libs/LCD_Draw.h"
#include <avr/pgmspace.h>
#include <avr/delay.h>
/* 
add scroll bar and add function to log data store it and reprint it  when you have access to the SD card 

Use terminal emulator first then use the terminal set properties, then printf is good for several lines
 roughly 20 lines of text. I don t recommend using any size above 1, it looks really ugly. I mean really REALLY UGLY.
 Due to the possible limitations of space for font (2-3kb of pgm) i didn't bother adding a larger font, hence anything 
 greater 1 will look terrible. Every pixel will be a 2x2 box or higher depending on the font. 
 

throw this in your main code if you want to use your display to output printf 

	FILE lcd_str = FDEV_SETUP_STREAM(TerminalChar, NULL, _FDEV_SETUP_RW);
	stdout = &lcd_str;

*/

void terminalemulator(void){
	struct Terminal_Program_Values	*pointTerminal = &TerminalProperties;
	struct LCD_Geometry *pointCurrent= &Current;
	struct LCD_Properties *pointProperties= &Properties;
	
	
	if (pointProperties->Rotation == 1 || pointProperties->Rotation == 2 )
	{
		pointTerminal->xResolution = pointProperties->xResolution;
		pointTerminal->yResolution = pointProperties->yResolution;
	}

	else 
	{
		pointTerminal->xResolution = pointProperties->yResolution;
		pointTerminal->yResolution = pointProperties->xResolution; 
	}
	
	pointTerminal->currentline = 3;
	pointTerminal->previousx= pointTerminal ->xResolution;
	pointTerminal->previousy= 0 ;	
	
	
	LCD_Fill_Rectangle(0,0,pointTerminal->xResolution,pointTerminal->yResolution, 0x000000);
	LCD_Write_Line("Microcrap Winderp [version 1.2]",10, (pointTerminal->yResolution - 15),1,0xFFFFF);
	LCD_Write_Line("Command Line Debugging System, use with printf",10, (pointTerminal->yResolution - 25),1,0xFFFFF);
}

void Terminal_Set_Line(char line){
	struct Terminal_Program_Values	*pointTerminal = &TerminalProperties;
	pointTerminal->currentline = line+3;	
}

void Terminal_Set_Properties(char line, char size, long int color){
	struct Terminal_Program_Values	*pointTerminal = &TerminalProperties;
	struct LCD_Geometry *pointCurrent= &Current;
	struct LCD_Properties *pointProperties= &Properties;
	
	pointTerminal->currentline = line+2;
	pointCurrent->color = color;
	pointCurrent->size = size; 
}

void TerminalChar(char c){
	struct Terminal_Program_Values	*pointTerminal = &TerminalProperties;
	struct LCD_Geometry *pointCurrent= &Current;
	struct LCD_Properties *pointProperties= &Properties;
	
	
	if (c == 0x0D || c == 0x0A) {
		 pointTerminal->currentline += 1;  
		 pointTerminal->previousx = 5;  
		 c = 0x3E;
	}
	
	else if (pointTerminal->previousx  > (pointTerminal->xResolution -(6*pointCurrent->size)) ){
		pointTerminal->currentline += 1;
		pointTerminal->previousx = 15;
	}
	
	else if( pointTerminal->currentline*(10*pointCurrent->size) > pointTerminal->yResolution){
		_delay_ms(100);
		LCD_Fill_Rectangle(0,0,pointTerminal->xResolution,pointTerminal->yResolution, 0x000000);
		Terminal_Set_Properties(2, 1, 0xFFFFFF);
	} 
	pointCurrent->xPosition = pointTerminal->previousx;
	pointCurrent->yPosition = pointTerminal->yResolution - pointTerminal->currentline*(10*pointCurrent->size);
	LCD_Write_Char(c);
	pointTerminal->previousx += (6* pointCurrent->size); 	
}

