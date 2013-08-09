/*
 * _SSD1289.c
 *
 * Created: 4/18/2013 2:37:00 PM
 *  Author: mfolz
 */ 
/*
 * _SSD1289 
 *
 * Created: 3/24/2013 6:23:55 PM
 *  Author: Michael
 This SSD1289 Driver library is meant to be used with the LCD_Draw.h library:
        - Always initalize the LCD before you call any functions from the LCD_Draw Library
		- The library uses parallel programming, not spi. 
		- 18 bit color (262,144 colors) colors are defined 0xRRGGBB  
		- Use the LCD_Geometry structure to define the variables before you pass them


 ---------------------------------------------------------------------------
 I have made the 
 
 
 
 */ 

#include "_SSD1289.h"
#include "../Atmeg_Libs/AtmegaPins.h"

void SSD1289_SendCommand(int command_address, int command_data){
	/// Send Command Address
	CLEARBIT(SSD_1289_DA_PORT, SSD_1289_RS); // pull RS low to send command
	LCD_SendPacket(command_address >> 8, command_address);
	//// Send Command Data
	SETBIT(SSD_1289_DA_PORT, SSD_1289_RS); // pull RS high to send data
	LCD_SendPacket(command_data >> 8, command_data);
}

void LCD_SendPacket(char LowerByte, char UpperByte){
	SETBIT(SSD_1289_RCW_PORT,SSD_1289_RS); 
	SSD_1289_LB_PORT = LowerByte;
	SSD_1289_HB_PORT = UpperByte;
	CLEARBIT(SSD_1289_RCW_PORT, SSD_1289_WR); // pull WR low to send write data
	SETBIT(SSD_1289_RCW_PORT, SSD_1289_WR);
}

void SSD1289_Initalize(){
	struct LCD_Properties *pointProperties = &Properties;
	 SSD_1289_LB_DDR = 0xFF;
	 SSD_1289_HB_DDR = 0xFF;
	 
	 pointProperties->colorBytes = 16; 
	 pointProperties->xResolution = 240;
	 pointProperties->yResolution = 320;
	 
	 SSD_1289_RCW_DDR = (1 << SSD_1289_CS) | (1 << SSD_1289_WR) | (1 << SSD_1289_RESET);
	 SSD_1289_DA_DDR = (1 << SSD_1289_RS);
	 
	SETBIT(SSD_1289_RCW_PORT, SSD_1289_RESET);
	_delay_us(250);  
	CLEARBIT(SSD_1289_RCW_PORT, SSD_1289_RESET);
	_delay_us(250); 
	SETBIT(SSD_1289_RCW_PORT, SSD_1289_RESET);
	_delay_us(250); 
	
	SSD1289_SendCommand(Display_Control, 0x0021);
	SSD1289_SendCommand(Oscillator, 0x0001);
	SSD1289_SendCommand(Display_Control, 0x2B3F);
	SSD1289_SendCommand(Sleep_Mode, 0x0000);
	SSD1289_SendCommand(Display_Control, 0x0033);
	SSD1289_SendCommand(LCD_Waveform_Control, 0x0600);
	SSD1289_SendCommand(Driver_Output_Control ,0x2B3F); // check this
	SSD1289_SendCommand(Entry_Mode, 0x40B0);   //65k color  - type B color input // type c 8-R 8-G 8-B 8-blank 
	
	
	SSD1289_SendCommand(Power_Control_1, 0x6664);
	SSD1289_SendCommand(Power_Control_2, 0x0000);
	SSD1289_SendCommand(Power_Control_3, 0x080C); 
	SSD1289_SendCommand(Power_Control_4, 0x2B00);
	SSD1289_SendCommand(Power_Control_5, 0x0029);
	SSD1289_SendCommand(Compare_Register, 0x0000);
	SSD1289_SendCommand(Compare_Register_2, 0x0000);
	SSD1289_SendCommand(Horizontal_Porch, 0xEF1C);
	SSD1289_SendCommand(Frame_Frequency_Control, 0x6000); 
	SSD1289_SendCommand(Vertical_Porch, 0x0003); // 0x0017 vertical porch 
	SSD1289_SendCommand(Display_Control, 0x0233);  // can turn on and off the display; keeps image in gddr
	//SSD1289_SendCommand(0x000B, 0x0000);  // FRAMe cycle control 

	SSD1289_SendCommand(Gate_Scan_Position, 0x0000); /// Starting position of the gate driver G0
	SSD1289_SendCommand(Gamma_Control_1,0x0707);
	SSD1289_SendCommand(Gamma_Control_2,0x0204);
	SSD1289_SendCommand(Gamma_Control_3,0x0204);
	SSD1289_SendCommand(Gamma_Control_4,0x0502);
	SSD1289_SendCommand(Gamma_Control_5,0x0507);
	SSD1289_SendCommand(Gamma_Control_6,0x0204);
	SSD1289_SendCommand(Gamma_Control_7,0x0204);
	SSD1289_SendCommand(Gamma_Control_8,0x0502);
	SSD1289_SendCommand(Gamma_Control_9,0x0302);
	SSD1289_SendCommand(Gamma_Control_10,0x0302);
	
	/// Scroll control and  screen driving Position
	SSD1289_SendCommand(Vertical_Scroll_Control, 0x0000);
	SSD1289_SendCommand(Vertical_Scroll_Control_2, 0x0000);
	SSD1289_SendCommand(Screen_driving_position, 0x0000);
	SSD1289_SendCommand(Screen_driving_position_2, 0x013F);
	SSD1289_SendCommand(Second_screen_driving_position, 0x0000);
	SSD1289_SendCommand(Second_screen_driving_position_2, 0x013F);
}


void LCD_Address_set()
{
	struct LCD_Geometry *pointCurrent= &Current;  /* To declare p as a pointer of type struct point */
	struct LCD_Properties *pointProperties = &Properties;  /* To declare p as a pointer of type struct point */
	//if (pointCurrent->mem_xPosition >= pointProperties->xResolution || pointCurrent->mem_x1Position >= pointProperties->xResolution || pointCurrent->mem_yPosition > pointProperties->yResolution) return;
	
	SSD1289_SendCommand(Horizontal_Ram_Address_Pos,(pointCurrent->mem_x1Position<<8)+pointCurrent->mem_xPosition); // Horizontal ram address position
	SSD1289_SendCommand(Vertical_Ram_Address_Pos,pointCurrent->mem_yPosition);  // vertical ram address position
	SSD1289_SendCommand(Vertical_Ram_Address_Pos_2,pointCurrent->mem_y1Position);
	SSD1289_SendCommand(Horizontal_Ram_Address_X_Initial,pointCurrent->mem_xPosition); // initial settings for GDDRAM x
	SSD1289_SendCommand(Vertical_Ram_Address_Y_Initial,pointCurrent->mem_yPosition); // initial settings for GDDRAM y
	CLEARBIT(SSD_1289_DA_PORT, SSD_1289_RS); // pull RS low to send command
	LCD_SendPacket(0x00, 0x22);	  /// write to gddr
}

