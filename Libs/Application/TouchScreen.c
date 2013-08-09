/*
 * TouchScreen.c
 *
 * Created: 5/23/2013 2:32:59 PM
 *  Author: mfolz
 
	Test Touch()
	-
	-
	
	
	Paint Program();
	-
	-
 
 */ 

#include "TouchScreen.h"
#include "../LCD_LIBS/UTouch.h"
#include "../LCD_LIBS/_SSD1289.h"
#include "../LCD_LIBS/LCD_Draw.h"
#include "Demo_App.h"


void UTouch_InitTouch(char orientation);
void UTouch_WriteData(char data);
int UTouch_ReadData(void);
void UTouch_Temperature_Read(void);
void UTouch_read(void);
char UTouch_dataAvailable(void);

void TestTouch(void){
	struct XPT2046_Variables	*pointTouch = &Touch;
	
	if (UTouch_dataAvailable()){ // IF new touch screen data is Available 
		// Clear X & Y
		UTouch_read();
		LCD_Fill_Rectangle(0,200,100,20, colorArray[15]); 
		LCD_Fill_Rectangle(0,300,100,20, colorArray[15]);
		
		LCD_Write_Line("Y: ", 0,180, 2, colorArray[11]);
		LCD_Write_Line(IntToArray(pointTouch->xValue , 4), 0,300, 2,colorArray[0]);
		LCD_Write_Line("X: ",0,280, 2, colorArray[11]);
		LCD_Write_Line(IntToArray(pointTouch->yValue , 4), 0,200, 2, colorArray[0]);
	}
}


//// This is incomplete and doesnt work at all
void Calibrate_TouchScreen(void){
	struct XPT2046_Variables	*pointTouch = &Touch;
	int XCalibrate= 0, YCalibrate =0; 

	for (char Count=0 ; Count < 5; Count++ )	
	{		
		if (UTouch_dataAvailable()){ // IF new touch screen data is Available
			UTouch_read();
			UTouch_XYCoridnates(240,320);
			LCD_Fill_Rectangle(0,120,80,20, colorArray[15]);
			LCD_Fill_Rectangle(0,200,80,20, colorArray[15]);
			LCD_Fill_Rectangle(0,140,80,20, colorArray[15]);
			LCD_Fill_Rectangle(0,220,80,20, colorArray[15]);
			
			LCD_Write_Line("Y: ", 0,100, 2, colorArray[11]);
			LCD_Write_Line(IntToArray(pointTouch->yValue , 4), 0,120, 2, colorArray[0]);
			LCD_Write_Line(IntToArray(pointTouch->yPositionRes , 4), 0,140, 2, colorArray[0]);
			
			LCD_Write_Line("X: ",0,180, 2, colorArray[11]);
			LCD_Write_Line(IntToArray(pointTouch->xValue , 4), 0, 200, 2,colorArray[0]);
			LCD_Write_Line(IntToArray(pointTouch->xPositionRes , 4), 0,220, 2, colorArray[0]);				
			
		}
		
		else	LCD_Draw_FillCircle(20,20,10,1, colorArray[11]);		
	}
	
	
}

/// uses the color array 
void Paint_Program(void){
	struct XPT2046_Variables	*pointTouch = &Touch;
	struct Paint_Program_Values *pointTouchProgram =	&PropertiesTouch;
	
	
	for (char x=0; x < 16; x++)
	LCD_Fill_Rectangle(x*20,200,20,40,colorArray[x]);
	
	while (1){
		if (UTouch_dataAvailable()){ // IF new touch screen data is Available
			// Clear X & Y
			UTouch_read();
			UTouch_XYCoridnates(240,320);
			
			if (pointTouch->xPositionRes >= 200 && pointTouch->xPositionRes <= 240)
				pointTouchProgram->Chosen_Color = colorArray[((320- pointTouch->yPositionRes)/20)];
			else
				LCD_Draw_FillCircle(pointTouch->xPositionRes, 320- pointTouch->yPositionRes,1,1,pointTouchProgram->Chosen_Color);
		
		}
	}
}
