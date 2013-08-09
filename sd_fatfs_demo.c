/* SD Card FAT File System Demo
 * http://frank.circleofcurrent.com/musicclock/
 * Copyright (c) 2011 Frank Zhao
 * 
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 */

// please see http://www.nongnu.org/avr-libc/user-manual/modules.html for info on these modules

#include "Fat/ff.h" // use FatFs from Elm-Chan.org
#include "Fat/ffconf.h"
#include "Fat/integer.h"
#include "Fat/diskio.h" // use FatFs from Elm-Chan.org (diskio used for disk initialization)

#include "Libs/LCD_LIBS/_SSD1289.h"


#include "Libs/SD/uSD_Card.h"

#include "Libs/LCD_LIBS/LCD_Draw.h"
#include "Libs/LCD_LIBS/Draw_Bitmap.h"

#include "Libs/Atmeg_Libs/Serial_Atmel328p.h"
#include "Libs/Application/Terminal.h"
#include "Libs/Application/TouchScreen.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h> // provides delays
#include <avr/power.h> // used for setting clock prescaler
#include <stdbool.h>

int routine(void); 
int Mikes_derpy(void);
bool SD_Mount(void);


void BMP_SD_READ(int x, int y); /// needs the file read8bytes lib draws pixle by pixle really slow
int test(void);

char File_Read8bytes(long int location);
int draw_bmp(int x, int y, char *filename); 

char File_Read_Custom_Length(long int location, int length, char *Output_Array);



FATFS filesystem;
FRESULT errCode;
uint8_t buff[64];
UINT read;
FIL file;	

// program entry point
int main()
{   
    int errCode; 
	struct LCD_Geometry *pointCurrent= &Current;
    struct LCD_Properties *pointProperties = &Properties;
    FILE lcd_str = FDEV_SETUP_STREAM(TerminalChar, NULL, _FDEV_SETUP_RW);
    stdout = &lcd_str;
	
	FRESULT rc;				/* Result code */
	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	UINT bw, br, i;
	
   	USART_init();
   	SPI_MasterInit();
   	
   	SSD1289_Initalize();
	  
   	UTouch_InitTouch(0);
    LCD_Fill_Rectangle(0,0, 320,240,0x000000);
    terminalemulator();
    Terminal_Set_Properties(2, 1, 0xFFFFFF);
	//while(1) 	{ Paint_Program();}
	    
	printf("Serial Default Output");
	/* Init the MMC */
	printf("mmc_init\n");
	 //disk_initialize(0);
	mmc_init();
	
	
	
	/// Drive must be mounted before use  // Drive 0 - signifies SD System 
	/// Halt on anything other than mounting system okay
	errCode = f_mount(0, &filesystem);
	if (errCode != FR_OK)
	{
		printf("Error: f_mount failed, error code: %d\r", errCode);
		printf("Please refer to error list"); 
		while (1); // freeze
	}
	
	printf("\n Mounted File System: Type SD");
	// Draw BMP	
	
	printf("\nCreate a new file (hello.txt).\n");
	rc = f_open(&file, "HELLO.TXT", FA_WRITE | FA_CREATE_ALWAYS);
	
	printf("\nWrite a text data. (Hello world!)\n");
	rc = f_write(&file, "Hello world!\r\n", 14, &bw);
	printf("%u bytes written.\n", bw);

	printf("\nClose the file.\n");
	rc = f_close(&file);

	while(1){
		//printf("\nOpen root directory.\n");
		rc = f_opendir(&dir, "");
		
		//printf("\nDirectory listing...\n");
		for (;;) {
			rc = f_readdir(&dir, &fno);		/* Read a directory item */
			if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		
			if (fno.fattrib & AM_DIR)
				printf("   <dir>  %s\n", fno.fname);
			else
				{
				//printf("%8lu  %s\n", fno.fsize, fno.fname);
				draw_bmp(0,0,fno.fname);
				}
		
		}
	}
}

/// file open -> file read -> file store 
/// use f seek to isolate sections of the bmp 
/// 64 bytes read at a time ... meaning there will be 1200 reads this will take a while

//// Make it resolution indipendant
//// make it work with the rotation 
//// 

int draw_bmp(int x, int y, char *filename){
	//Open file for reading! 
	errCode = f_open(&file, filename, FA_READ);
	if (errCode != FR_OK)
	{
		printf("Error: f_open failed, error code: %d\r", errCode);
		return; 
	}
//	printf("\n Opening %s", filename);
	
	errCode = f_lseek(&file, 1 );
	errCode = f_read(&file, buff , 1, &read);
//	printf("\n Opened %s printing stuffs", filename);
	
		
	
	// BMP SHIZ
	BMP_SD_READ(x,y);
		
	errCode = f_close(&file);
	if (errCode != FR_OK)
	{
		printf("Error: f_close failed, error code: %d\r", errCode);
	}
	return 0;
}


void BMP_SD_READ(int x, int y){
	// bmp must be 32bit dont like it suck my fucking cock 
	// assumes padding is 0 
	struct LCD_Properties *pointProperties = &Properties;
	struct LCD_Geometry *pointCurrent = &Current;
	long unsigned int bmp_start =0, bmp_width=0, bmp_height=0, bmp_raw_data=0, width_loop=0, height_loop=0;
	int padding=0; 
	int vertical_count =0, horizontal_count=0;   
	char color_array[960];
	
	// BMP READ ROUTINE // 
   if( File_Read8bytes(0) != 0x42 || File_Read8bytes(1) != 0x4D)return;
	//printf("\n This file is a valid BMP");
	//Get BMP Start ( start of image RR-GG-BB)
	bmp_start = (File_Read8bytes(13) << 24) +(File_Read8bytes(12) << 16)+ (File_Read8bytes(11) << 8)+ File_Read8bytes(10);
	//printf("\n  BMP_START: %d ", bmp_start);
	// Get Width and Height
	bmp_width =  (File_Read8bytes(0x15) << 24)+ (File_Read8bytes(0x14) << 16)+ (File_Read8bytes(0x13) << 8)+ File_Read8bytes(0x12);
	bmp_height = (File_Read8bytes(0x19) << 24) + (File_Read8bytes(0x18) << 16)+ (File_Read8bytes(0x17) << 8)+ File_Read8bytes(0x16);
	bmp_raw_data =(File_Read8bytes(0x25) << 24) + (File_Read8bytes(0x24) << 16)+ (File_Read8bytes(0x23) << 8)+ File_Read8bytes(0x22);
	
  	padding =  bmp_raw_data - (bmp_width*bmp_height*3); 
  	padding = padding/ bmp_height; 
	
	/// Change input blah blah entry mode 
	SSD1289_SendCommand(Entry_Mode, 0x40B8);   //65k color  - type B color input // type c 8-R 8-G 8-B 8-blank
		
	// there must be a better method D: 
	height_loop = bmp_height;
	width_loop = bmp_width;
	
	// This is used to draw images that are larger than the screen draw only the first 320x240 of the image  
	if (bmp_width > pointProperties->yResolution) {
		width_loop = pointProperties->yResolution;
	}
	if (bmp_height > pointProperties->xResolution){
		height_loop = pointProperties->xResolution; 	
	}
	
	// Put display in landscape mode swap the height and width values
	pointCurrent->mem_xPosition= y;
	pointCurrent->mem_x1Position = ((y+height_loop)-1);
	pointCurrent->mem_yPosition= x;
	pointCurrent->mem_y1Position = ((x+width_loop)-1);

	// CLEARBIT(SSD_1289_RCW_DDR, SSD_1289_CS);
	SSD1289_Address_set();

	for (int vertical_count = 0; vertical_count < height_loop; vertical_count++)
	{
		// Read one line from the SD card (store it in the color array BB - GG - RR) 
		File_Read_Custom_Length((bmp_start+(padding*vertical_count)+(vertical_count*3*bmp_width)) , (width_loop*3), color_array); 

		for (int horizontal_count = 0;  horizontal_count < width_loop; horizontal_count++ ) 	
		{		
			SSD1289_SendPacket(color_array[(horizontal_count*3)+2], color_array[(horizontal_count*3)+1]);
			SSD1289_SendPacket(color_array[(horizontal_count*3)], 0x00);			
		}/*end of horizontal_count */
	}/*end of vertical_count */
	
	return;
}



char File_Read_Custom_Length(long int location, int length, char *Output_Array){ 
	
	errCode = f_lseek(&file, location);
	if (errCode != FR_OK)
	{
		printf("Error: f_ failed, error code: %d\r", errCode);
		while (1); // freeze
	}	
	
	errCode = f_read(&file, Output_Array, length , &read);

	if (errCode != FR_OK)
	{
		printf("Error: f_close failed, error code: %d\r", errCode);
		while (1); // freeze
	}
	return 0; 	
}


char File_Read8bytes(long int location){
	uint8_t filebyte[1];  
	
	errCode = f_lseek(&file, location);
	if (errCode != FR_OK)
	{
		printf("Error: f_ failed, error code: %d\r", errCode);
		while (1); // freeze
	}
		
	
	errCode = f_read(&file, filebyte, 1, &read);

	if (errCode != FR_OK)
	{
		printf("Error: f_close failed, error code: %d\r", errCode);
		while (1); // freeze
	}
	//printf("0x%x, ", filebyte[0]);
	
	return filebyte[0]; 	
}

