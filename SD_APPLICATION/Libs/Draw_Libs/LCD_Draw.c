/*
 * LCD_Draw.h
 *
 * Created: 3/26/2013 5:51:24 PM
 *  Author: mfolz
 
 This Library currently work with the SSD1289, ST7735 and ST7735R -- Will be adding more libraries soon 
 -----------------------------------------------------------------
		color: 
		- Before you use any of the functions from this library use the
		corresponding library. colors may be put in 24bit however only 18-16bits worth
		of that color counts.  0xRRGGBB color.
		
		x,y,width and height:
		-Send width and height (make sure initial xCorridnate+width or yCorinate+height
		are within the valid LCD Resolutions)
		
		LCD_SendPixel():
			-this will draw a 1x1 pixel anyplace that is valid.
		
		LCD_Fill_Rectangle();  // fast library
			-draws a rectangle anywhere on the screen within the screens valid resolution. 
		
		LCD_Draw_Line(); 
			-Draws a line from any initial point (x0,y0) - > (x1, y1)
		
		LCD_Write_PGM();
			-Takes in a string from the program memory (long strings can take up valuable space on the ram)
			prints out on a string(no limit to the screen wont write chars beyond a valid resolution)
			
		LCD_Write_Line(); 
			-Same as the PGM, only difference is you can place a string directly without using PGM
		
		LCD_FillScreen(); 
			-Self explanatory, fills screen with desired color, again 0xRRGGBB color 
		
		LCD_Write_Char() 
			- I do not recommend using this function, you will need to set the variables for the structures. 
			
		LCD_Write_Style_Line();
			- Same as the write line except you will be using arial font. Do not use ~ or / keys. they will not print properly
		
			
	
 */ 

#include "ArialFont.h"
#include "font.h"
#include "LCD_Draw.h"
#include "../LCD LIBS/_SSD1289.h"
#include "../Atmeg_Libs/AtmegaPins.h"

long int colorArray[] = {
	0xFFFFFF		//	White
	, 0xFFFF00		//	Yellow
	, 0xFF00FF		//	Fuchsia
	, 0xFF0000		//	Red
	, 0xC0C0C0		//	Silver
	, 0x808080		//	Gray
	, 0x808000		//	Olive
	, 0x800080		//	Purple
	, 0x800000		//	Maroon
	, 0x00FFFF		//	Aqua
	, 0x00FF00		//	Lime
	, 0x008080		//	Teal
	, 0x008000		//	Green
	, 0x0000FF		//	Blue
	, 0x000080		//	Navy
	, 0x000000,		//	Black
};


void LCD_Geometry2(int xPosition, int yPosition, int size, long int color ){
	struct LCD_Geometry *pointCurrent= &Current;
	pointCurrent->xPosition = xPosition;
	pointCurrent->yPosition = yPosition;
	pointCurrent->size = size;
	pointCurrent->color = color;
}

void LCD_FillScreen (int color)
{
	struct LCD_Properties *pointProperties = &Properties;
	LCD_Fill_Rectangle(0,0,pointProperties->xResolution,pointProperties->yResolution, color);
}

void LCD_Fill_Rectangle(int x, int y, int width, int height, long int color)
{
	struct LCD_Properties *pointProperties = &Properties;
	struct LCD_Geometry *pointCurrent= &Current;
	int red, green, blue;
	int drawrow, drawcolumn;
	// without this the display goes nyanners 

	
	if (pointProperties->Rotation == 1)	
	{	
		pointCurrent->mem_xPosition= x; pointCurrent->mem_x1Position = ((x+width)-1);
		pointCurrent->mem_yPosition= y; pointCurrent->mem_y1Position = ((y+height)-1);
		if (x > pointProperties->xResolution || y > pointProperties->yResolution || y< 0 || x < 0)return;		
		if (x+width > pointProperties->xResolution) width= pointProperties->xResolution - (x+1);
		else if(height+y > pointProperties->yResolution) height = pointProperties->yResolution -(y+1);
		
	
	}
	else
	{
		pointCurrent->mem_xPosition= y; pointCurrent->mem_x1Position = ((y+height)-1);
		pointCurrent->mem_yPosition= x; pointCurrent->mem_y1Position = ((x+width)-1);
		if (x > pointProperties->yResolution || y > pointProperties->xResolution || y< 0 || x < 0)return;
		if (x+width  > pointProperties->yResolution) width = pointProperties->yResolution - (x+1);
		else if(height+y > pointProperties->xResolution) height= pointProperties->xResolution -(y+1);
	}
	
	red = (color >> 16);
	green = (color >> 8);
	blue = (color);
	
	CLEARBIT(SSD_1289_RCW_DDR, SSD_1289_CS);
	LCD_Address_set();
	SETBIT(SSD_1289_DA_PORT, SSD_1289_RS);
	
	for(drawcolumn=0; drawcolumn<=height; drawcolumn++){
		for (drawrow=0; drawrow<=width; drawrow++)
		{
			//LCD_SendPixel(drawcolumn, drawrow, color);
			LCD_SendPacket(red, green);
			LCD_SendPacket(blue, 0x00);
		}
	}
	SETBIT(SSD_1289_RCW_DDR, SSD_1289_CS);
}

void LCD_SendPixel(int x, int y, long int color)
{
	struct LCD_Properties *pointProperties = &Properties;
	struct LCD_Geometry *pointCurrent= &Current;
	int countx, county;
	int red, green, blue;
	pointCurrent->mem_xPosition= x; pointCurrent->mem_x1Position = (x+1);
	pointCurrent->mem_yPosition= y; pointCurrent->mem_y1Position = (y+1);
	
	red = (color >> 16);
	green = (color >> 8);
	blue = (color);
	// if xposition is greater than resolution return
	if (pointCurrent->mem_x1Position > pointProperties->xResolution || pointCurrent->mem_y1Position > pointProperties->yResolution){return;}
	if (pointCurrent->mem_x1Position <= 0 || pointCurrent->mem_y1Position <= 0 ){return;} // less than 0 return 
	CLEARBIT(SSD_1289_RCW_DDR, SSD_1289_CS);
	
	LCD_Address_set();
	SETBIT(SSD_1289_DA_PORT, SSD_1289_RS);
	LCD_SendPacket(red, green);
	LCD_SendPacket(blue, 0x00);
	SETBIT(SSD_1289_RCW_DDR, SSD_1289_CS);
}

void LCD_Write_Line(char *string, int x, int y, int size, long int color){
		struct LCD_Geometry *pointCurrent= &Current;
		struct LCD_Properties *pointProperties= &Properties;
		int charcount=0, xcolMAX= x;
		int space= -6; 
		
		if (pointProperties->Rotation == 1) space = 0x06; 
		
		while (*string){   // while char pointer != null
			/// allows the text to move to the next line, for both potrait and landscape modes. 
			if (pointProperties->Rotation == 1){ if(xcolMAX >= (pointProperties->xResolution)-5 ) {y+=(8*size); x=1; charcount=0; xcolMAX=0; } } // if char begins after 123 pixles drop one line and set x and char count to 0
			else {  if(xcolMAX <= 5 ){ y+=(10*size); x=318; charcount=0; xcolMAX=318;	}	 }			
		
			pointCurrent->xPosition = x+charcount;
			pointCurrent->yPosition = y;
			pointCurrent->size = size;
			pointCurrent->color= color;
			LCD_Write_Char(*string);  // writes char to screen
			string++; // incriment the string pointer +=1;
			charcount+=space*size;  // is the length of each character + space on the end each char is 7x5 //change char count if you want no space
			if (pointProperties->Rotation == 1)	xcolMAX += (space*size);
			else { xcolMAX += (space*size);  	USART_send(xcolMAX);} 
		}							
}

/// Store long strings of chars in the program memory, use this function to print it on screen.
void LCD_Write_PGM(const char *string, int x, int y, int size, long int color){
	struct LCD_Geometry *pointCurrent= &Current;
	int xColmax=x;
	int charcount=0;
	char izard;
	while (izard = pgm_read_byte(string)){     // read input string variable // while string != null do
		if((xColmax) >= 120) {y+=(8*size); x=1; charcount=0; xColmax=0;} // if char begins after 123 pixels drop one line and set x and char count to 0
		else{
			pointCurrent->xPosition = x+charcount;
			pointCurrent->yPosition = y;
			pointCurrent->size = size;
			pointCurrent->color= color;
			LCD_Write_Char(izard); // write char to screen//
			charcount+=6;
			string++;
		}
		xColmax+= (6*size);
	}
	return;
}

void LCD_Write_Char(char c){
	struct LCD_Geometry *pointCurrent= &Current;
	struct LCD_Properties *pointProperties= &Properties;
	int Xcol =0 , Yrow=0;
	int Byte =0 , count= 0;
	int Xrotation = 1; 
	
	if (pointProperties->Rotation == 0)  Xrotation= -1; 
	
	for (Xcol= 0; Xcol < 5; Xcol++) {
		Byte = pgm_read_byte(font+((c*5)+Xcol));   // Access the font array (Char input * 5) + Xcol//
		// i chose to store the font in program memory as it is fairly large and will take up a lot of RAM, since values are static nothign is lost.
			for (Yrow = 0; Yrow < 8; Yrow++)
			{	// since byte is a char or 8 bytes we make the loop look through 8 bits
				if (Byte & 0x01){   // draw pixel only when shifted byte and 0x01 are equal //
					LCD_Fill_Rectangle(pointCurrent->xPosition+((Xrotation*Xcol)*pointCurrent->size), pointCurrent->yPosition+(Yrow*pointCurrent->size),pointCurrent->size,pointCurrent->size, pointCurrent->color );
					}					
			Byte >>=1; // shift byte over 1
		}

	}
	return;
}

///for (Yrow = 8; Yrow > 0; Yrow--) flips the text 

void LCD_Draw_Line(int x0, int y0, int x1, int y1, int size, long int color){
	struct LCD_Geometry *pointCurrent = &Current; // point to the
	struct HT1632C_Properties *point_properties = &Current;
	
	int dx = abs(x1-x0), sx = x0 < x1 ? 1: -1;   // give the absolute value of x1 - x0 , if x0 is less than x1 increment otherwise count down ->
	int dy = abs(y1-y0), sy = y0 < y1 ? 1: -1;// same as x just counting with the y corridnates.
	int err =  dx - dy;// ((dx > dy)? dx : -dy)/2, e2;
	int e2=0;
	
	
	
	pointCurrent->color = color;
	
	for (;;){
		
		for (int x=0; x < size; x++)
		{
			 LCD_SendPixel(x0,y0+x, color);
		}
		
		if (x0== x1 && y0==y1) break;
		
		e2 = err *2 ;
		
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
	
}

void LCD_Draw_FillCircle(int x0, int y0, int radius, int fill, long int color)
{
	struct LCD_Geometry *pointCurrent= &Current;
	struct LCD_Properties *pointProperties= &Properties;
	
	unsigned char Mult_X = 1;
	unsigned char Mult_Y = 1;
		
	int x= radius, y = 0;
	int xChange = 1 - (radius << 1);
	int yChange = 0;
	int radiusError = 0;
	
		while(x >= y)
		{	
			Mult_X=1;
			Mult_Y=1;
				
			if (fill == 0){
				for ( int quadrents = 1; quadrents <= 4; quadrents++){
					if(quadrents/2 == 1) Mult_X *=-1;
					if(quadrents/3 == 1) Mult_Y *=-1;
					if(quadrents/4 == 1) {Mult_X *=-1; Mult_Y*=-1;}
				
					LCD_SendPixel((x*Mult_X + x0), (y*Mult_Y + y0), color);   // quadrent 1
					LCD_SendPixel((y*Mult_X + x0), (x*Mult_Y + y0), color);			
				}
			}			
			
			else{
				LCD_Draw_Line(x0 - x, y0 + y, x0 + x, y0 + y,1 , color);
				LCD_Draw_Line(x0 - x, y0 - y, x0 + x, y0 - y, 1, color);
				LCD_Draw_Line(x0 - y, y0 + x, x0 + y, y0 + x,1 ,color);
				LCD_Draw_Line(x0 - y, y0 - x, x0 + y, y0 - x, 1 ,color);
			}	
						
		y++;
		radiusError += yChange;
		yChange += 2;
	
		if(((radiusError << 1) + xChange) > 0) // when RadiusError * 2 + xChange is greater than 0
		{
			x--;    // lower the default x value which at start is = radius
			radiusError += xChange;   //
			xChange += 2;
		}
	}
}


void LCD_Write_Style_Line(char *string, int x, int y, int size, long int color){
	struct LCD_Geometry *pointCurrent= &Current;
	struct LCD_Properties *pointProperties= &Properties;
	unsigned char offset = 0x21; 
	int column;
	int charcount=0, space=0, xcolMAX= x;
	int increase =0;
		
	while (*string){   // while char pointer != null
		if (*string == 0x5C) offset = 0x22; 
		if (*string == '~')return;
		column = (arial_8ptDescriptors[(*string)-offset].columns)+2;
		if(xcolMAX >= ((pointProperties->xResolution)-column)-5) {
			 y+=(12*size); x=5; charcount=0; xcolMAX=0; 
			}  // if char begins after 123 pixles drop one line and set x and char count to 0
	
		else{
			
			pointCurrent->xPosition = x+charcount;
			pointCurrent->yPosition = y;
			pointCurrent->size = size;
			pointCurrent->color= color;
			LCD_Write_Style_Char(*string);  // writes char to screen
			string++; // incriment the string pointer +=1;
			charcount+=column*size;  // is the length of each character + space on the end each char is 7x5 //change char count if you want no space
			xcolMAX += column*size;
		}
	}							
}


void LCD_Write_Style_Char(unsigned char c){
	struct LCD_Geometry *pointCurrent= &Current;
	unsigned char offset = 0x21; 
	int difference, column, location;
	int drawbyte, charwidth;
	int yPosition=0;
	unsigned char byte;
	unsigned char charC;
	
	if (c == 0x5C) offset = 0x22; /// takes care of that '/' problem 
	if (c == '~') return;   
	charC = (c -offset); 
		
	column = arial_8ptDescriptors[charC].columns; // get the number of columns for the character 
	location = arial_8ptDescriptors[charC].location;  // get its initial location 
	difference = (arial_8ptDescriptors[(charC+1)].location - arial_8ptDescriptors[charC].location);// obtain the difference 
	
	for (int Yrow=0; Yrow < difference; Yrow++) 
	{ 
		byte= pgm_read_byte(arial_8ptBitmaps+(location+Yrow));

		for (int Xcol=0; Xcol < column; Xcol++)
		{
			if (Xcol == 8) {  // special chars wide chars + 8 in columns 
				Yrow+=1;
				byte = pgm_read_byte(arial_8ptBitmaps+(location+Yrow));  
			}
				
			if (byte & 0x80) {
				LCD_Fill_Rectangle(pointCurrent->xPosition+(Xcol*pointCurrent->size), pointCurrent->yPosition+(yPosition*pointCurrent->size),pointCurrent->size,pointCurrent->size, pointCurrent->color );
				//LCD_SendPixel(pointCurrent->xPosition+(Xcol),pointCurrent->yPosition+yPosition, pointCurrent->color);
			}
			byte <<= 1;
		}
		yPosition++;
	}	
	return; 	
}
