/***************************************
 *                                     *
 * Author: Daniel Brall                *
 * Web:    http://www.bradan.eu/       *
 *                                     *
 ***************************************/

#ifndef _SED1520_H_
#define _SED1520_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

// port settings
#define SED1520_DATAPORT	PORTC
#define SED1520_DATAPIN		PINC
#define SED1520_DATADDR		DDRC

#define SED1520_CTRLPORT	PORTD
#define SED1520_CTRLDDR		DDRD

#define SED1520_RES			0
#define SED1520_RW			(1 << 0)
#define SED1520_E			(1 << 3)
#define SED1520_A0			(1 << 6)
#define SED1520_CS1			(1 << 1)
#define SED1520_CS2			(1 << 5)

// display settings
#define SED1520_SCREENWIDTH			122
#define SED1520_SCREENHEIGHT		32

// internal controller definitions
#define SED1520_CONTROLLER1					SED1520_CS1
#define SED1520_CONTROLLER2					SED1520_CS2

// internal command definitions
#define SED1520_CMDMODE_WRITE				0b01
#define SED1520_CMDMODE_DATA				0b10


#define SED1520_CMD_DISPLAYONOFF			0xAE
#define SED1520_CMD_DISPLAYSTARTLINE		0xC0
#define SED1520_CMD_SETPAGEADDRESS			0xB8
#define SED1520_CMD_SETCOLUMNADDRESS		0x00

#define SED1520_CMD_READSTATUS				0x00
#define SED1520_CMD_WRITEDISPLAYDATA		0x00
#define SED1520_CMD_READDISPLAYDATA			0x00

#define SED1520_CMD_SELECTADC				0xA0
#define SED1520_CMD_STATICDRIVEONOFF		0xA4
#define SED1520_CMD_SELECTDUTY				0xA8
#define SED1520_CMD_READMODIFYWRITE			0xE0
#define SED1520_CMD_END						0xEE
#define SED1520_CMD_RESET					0xE2

// internal flag definitions
#define SED1520_FLAG_BUSY					(1 << 7)
#define SED1520_FLAG_ADC					(1 << 6)
#define SED1520_FLAG_ONOFF					(1 << 5)
#define SED1520_FLAG_RESET					(1 << 4)

// internal timing definitions
#define DELAY_SHORT 						_delay_us(1.0);
#define DELAY_LONG 							DELAY_SHORT; DELAY_SHORT

// atomic device functions
uint8_t sed1520ExecuteCommand(uint8_t mode, uint8_t command, uint8_t parameter, uint8_t controller);
uint8_t sed1520BusyExecuteCommand(uint8_t mode, uint8_t command, uint8_t parameter, uint8_t controller);

// atomic graphical functions
void sed1520Init(void);
void sed1520SetDisplayOnOff(uint8_t on);
void sed1520SetStaticOnOff(uint8_t on);

void sed1520SetPixel(uint8_t x, uint8_t y, uint8_t value);
uint8_t sed1520GetPixel(uint8_t x, uint8_t y);

// internal more advanced graphical functions and structures
#define SED1520_IMGMODE_NORMAL					0
#define SED1520_IMGMODE_INVERTEDCOLORS			(1 << 0)
#define SED1520_IMGMODE_TRANSPARENTBLACK		(1 << 1)
#define SED1520_IMGMODE_TRANSPARENTWHITE		(1 << 2)

typedef uint8_t (*SED1520SHADERCALLBACK) (uint8_t, uint8_t, uint8_t, void*);

typedef struct {
	uint8_t x;
	uint8_t y;
	uint16_t width;
	uint16_t height;
	
	uint8_t* imgData;
	
	int16_t highestX;
	
	uint8_t mode;
} SED1520IMAGE;

uint8_t sed1520FillShader(uint8_t x, uint8_t y, uint8_t value, uint8_t* pixelValue);
uint8_t sed1520DrawImageShader(uint8_t x, uint8_t y, uint8_t value, SED1520IMAGE* img);

// more advanced graphical functions
void sed1520ShaderExecute(SED1520SHADERCALLBACK shaderCallback, uint8_t originX, uint8_t originY, uint8_t width, uint8_t height, void* additionalData);
void sed1520ClearScreen(void);
int16_t sed1520DrawImage(uint8_t* imgData, uint8_t mode, int16_t originX, int16_t originY, uint8_t clipX, uint8_t clipY, uint8_t clipWidth, uint8_t clipHeight);
void sed1520DrawText(char* text, uint8_t* font, uint8_t mode, int16_t originX, int16_t originY, uint8_t fixedWidth, int8_t deltaCharDist);
void sed1520DrawBox(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t filled, uint8_t pixelValue);
void sed1520DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixelValue);

#endif