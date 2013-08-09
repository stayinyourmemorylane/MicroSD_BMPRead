/*
 * TouchScreen.h
 *
 * Created: 5/23/2013 2:33:08 PM
 *  Author: mfolz
 */ 


#ifndef TOUCHSCREEN_H_INCLUDED
#define TOUCHSCREEN_H_INCLUDED

void TestTouch(void);
void Color_Selector(long int colorArray[]);
void Paint_Program(void);
void Calibrate_TouchScreen(void);

struct Paint_Program_Values{
	long int Chosen_Color;
}PropertiesTouch;

#endif /* TOUCHSCREEN_H_ */