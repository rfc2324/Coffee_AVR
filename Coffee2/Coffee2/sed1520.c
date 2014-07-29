/***************************************
 *                                     *
 * Author: Daniel Brall                *
 * Web:    http://www.bradan.eu/       *
 *                                     *
 ***************************************/

#include "sed1520.h"
#include <avr/pgmspace.h>

uint8_t sed1520ExecuteCommand(uint8_t mode, uint8_t command, uint8_t parameter, uint8_t controller) {
	uint8_t result = 0;
	
	SED1520_CTRLPORT &= ~(SED1520_E);
	
	DELAY_SHORT;
	
	if((mode & SED1520_CMDMODE_WRITE) != 0) {
		SED1520_DATADDR  = 0xff;
		SED1520_DATAPORT = command | parameter;
		
		SED1520_CTRLPORT &= ~(SED1520_RW);
	} else {
		SED1520_DATADDR  = 0x00;
		SED1520_DATAPORT = 0xff;
		
		SED1520_CTRLPORT |= SED1520_RW;
	}
	
	if((mode & SED1520_CMDMODE_DATA) != 0) {
		SED1520_CTRLPORT |= SED1520_A0;
	} else {
		SED1520_CTRLPORT &= ~(SED1520_A0);
	}
	
	if(controller & SED1520_CONTROLLER1) {
		SED1520_CTRLPORT &= ~(SED1520_CS1);
	} else {
		SED1520_CTRLPORT |= (SED1520_CS1);
	}
	
	if(controller & SED1520_CONTROLLER2) {
		SED1520_CTRLPORT &= ~(SED1520_CS2);
	} else {
		SED1520_CTRLPORT |= (SED1520_CS2);
	}
	
	DELAY_SHORT;
	
	SED1520_CTRLPORT |= SED1520_E;
	
	if((mode & SED1520_CMDMODE_WRITE) == 0) {
		result = SED1520_DATAPIN;
	}
	
	SED1520_CTRLPORT &= ~(SED1520_E);
	
	DELAY_SHORT;
	
	SED1520_CTRLPORT |= SED1520_CS1 | SED1520_CS2;
	
	return result;
}

uint8_t sed1520BusyExecuteCommand(uint8_t mode, uint8_t command, uint8_t parameter, uint8_t controller) {
	// wait for the busy flag
	while((sed1520ExecuteCommand(0, SED1520_CMD_READSTATUS, 0, controller) & SED1520_FLAG_BUSY) != 0);
	
	return sed1520ExecuteCommand(mode, command, parameter, controller);
}

// atomic graphical functions
void sed1520Init(void) {
	SED1520_DATADDR  |= 0xff;
	SED1520_DATAPORT  = 0x00;
	
	SED1520_CTRLDDR  |= SED1520_A0 | SED1520_RW | SED1520_E | SED1520_CS1 | SED1520_CS2 | SED1520_RES;
	
	SED1520_CTRLPORT |= SED1520_RES | SED1520_CS1 | SED1520_CS2;
	SED1520_CTRLPORT &= ~(SED1520_A0 | SED1520_RW | SED1520_E);
	
	DELAY_LONG;
	
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_RESET, 0, SED1520_CS1 | SED1520_CS2);
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_DISPLAYSTARTLINE, 0, SED1520_CS1 | SED1520_CS2);
}

void sed1520SetDisplayOnOff(uint8_t on) {
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_DISPLAYONOFF, on, SED1520_CS1 | SED1520_CS2);
}

void sed1520SetStaticOnOff(uint8_t on) {
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_STATICDRIVEONOFF, on, SED1520_CS1 | SED1520_CS2);
}

void sed1520SetPixel(uint8_t x, uint8_t y, uint8_t value) {
	uint8_t page = y / 8;
	uint8_t controller = 0;
	
	y -= page * 8;
	
	if(x >= SED1520_SCREENWIDTH / 2) {
		controller = SED1520_CONTROLLER2;
		x -= SED1520_SCREENWIDTH / 2;
	} else {
		controller = SED1520_CONTROLLER1;
	}
	
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETPAGEADDRESS, page, controller);
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETCOLUMNADDRESS, x, controller);
	
	// dummy read
	sed1520BusyExecuteCommand(SED1520_CMDMODE_DATA, SED1520_CMD_READDISPLAYDATA, 0, controller);
	
	if(value) {
		value = sed1520BusyExecuteCommand(SED1520_CMDMODE_DATA, SED1520_CMD_READDISPLAYDATA, 0, controller) | (1 << y);
	} else {
		value = sed1520BusyExecuteCommand(SED1520_CMDMODE_DATA, SED1520_CMD_READDISPLAYDATA, 0, controller) & ~(1 << y);
	}
	
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETCOLUMNADDRESS, x, controller);
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE | SED1520_CMDMODE_DATA, SED1520_CMD_WRITEDISPLAYDATA, value, controller);
}

uint8_t sed1520GetPixel(uint8_t x, uint8_t y) {
	uint8_t result;
	uint8_t page = y / 8;
	uint8_t controller = 0;
	
	y -= page * 8;
	
	if(x >= SED1520_SCREENWIDTH / 2) {
		controller = SED1520_CONTROLLER2;
		x -= SED1520_SCREENWIDTH / 2;
	} else {
		controller = SED1520_CONTROLLER1;
	}
	
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETPAGEADDRESS, page, controller);
	sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETCOLUMNADDRESS, x, controller);
	
	// dummy read
	sed1520BusyExecuteCommand(SED1520_CMDMODE_DATA, SED1520_CMD_READDISPLAYDATA, 0, controller);
	
	result = (sed1520BusyExecuteCommand(SED1520_CMDMODE_DATA, SED1520_CMD_READDISPLAYDATA, 0, controller) >> y) & 0x01;
	
	return result;
}

void sed1520ShaderExecute(SED1520SHADERCALLBACK shaderCallback, uint8_t originX, uint8_t originY, uint8_t width, uint8_t height, void* additionalData) {
	uint8_t pixelValue;
	uint8_t controller = SED1520_CONTROLLER1;
	uint8_t page;
	uint8_t counter = 0;
	
	while(controller != 0) {
		page = 0;
		
		if(originX + width >= counter && originX < counter + SED1520_SCREENWIDTH / 2) {
			for(uint8_t pageY = 0; pageY < SED1520_SCREENHEIGHT; pageY += 8, page ++) {
				if((pageY + 8) >= originY && pageY < originY + height) {
					uint8_t x = 0;
					
					sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETPAGEADDRESS, page, controller);
					
					if(originX > counter) {
						x = originX - counter;
					}
					
					sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETCOLUMNADDRESS, x, controller);
					
					sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_READMODIFYWRITE, 0, controller);
					
					while(x < SED1520_SCREENWIDTH / 2) {
						uint8_t newValue = 0;
						uint8_t realX = x + counter;
						
						if(realX >= originX + width) {
							break;
						}
						
						// dummy read
						sed1520BusyExecuteCommand(SED1520_CMDMODE_DATA, SED1520_CMD_READDISPLAYDATA, 0, controller);
						
						pixelValue = sed1520BusyExecuteCommand(SED1520_CMDMODE_DATA, SED1520_CMD_READDISPLAYDATA, 0, controller);
						
						for(uint8_t y = 0; y < 8; y ++) {
							if(y + pageY >= originY && y + pageY < originY + height) {
								newValue |= shaderCallback(realX, y + pageY, (pixelValue >> y) & 0x01, additionalData) << y;
							} else {
								newValue |= pixelValue & (0x01 << y);
							}
						}
						
						sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE | SED1520_CMDMODE_DATA, SED1520_CMD_WRITEDISPLAYDATA, newValue, controller);
						
						x ++;
					}
					
					sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_END, 0, controller);
				}
			}
		}
		
		switch(controller) {
			case SED1520_CONTROLLER1:
				controller = SED1520_CONTROLLER2;
				break;
			case SED1520_CONTROLLER2:
				controller = 0;
				break;
		}
		
		counter += SED1520_SCREENWIDTH / 2;
	}
}

void sed1520ClearScreen(void) {
	for(uint8_t y = 0; y < SED1520_SCREENHEIGHT / 8; y ++) {
		sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETPAGEADDRESS, y, SED1520_CS1 | SED1520_CS2);
		for(uint8_t x = 0; x < SED1520_SCREENWIDTH / 2; x ++) {
			sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE, SED1520_CMD_SETCOLUMNADDRESS, x, SED1520_CS1 | SED1520_CS2);
			sed1520BusyExecuteCommand(SED1520_CMDMODE_WRITE | SED1520_CMDMODE_DATA, SED1520_CMD_WRITEDISPLAYDATA, 0x00, SED1520_CS1 | SED1520_CS2);
		}
	}
}

uint8_t sed1520DrawImageShader(uint8_t x, uint8_t y, uint8_t value, SED1520IMAGE* img) {
	uint8_t screenX = x;
	
	x -= img->x;
	y -= img->y;
	
	uint16_t arrayPos = (uint16_t) x + (uint16_t) y * (uint16_t) img->width;
	uint8_t result = ((uint8_t) pgm_read_byte(img->imgData + arrayPos / 8) >> (arrayPos % 8)) & 0x01;
	
	if(result != 0x00 && screenX > img->highestX) {
		img->highestX = screenX;
	}
	
	if(img->mode & SED1520_IMGMODE_INVERTEDCOLORS) {
		result = !result;
	}
	
	if(img->mode & SED1520_IMGMODE_TRANSPARENTBLACK) {
		result &= value;
	}
	
	if(img->mode & SED1520_IMGMODE_TRANSPARENTWHITE) {
		result |= value;
	}
	
	return result;
}

int16_t sed1520DrawImage(uint8_t* imgData, uint8_t mode, int16_t originX, int16_t originY, uint8_t clipX, uint8_t clipY, uint8_t clipWidth, uint8_t clipHeight) {
	SED1520IMAGE img;
	
	img.x = originX;
	img.y = originY;
	img.width    = pgm_read_word(imgData); imgData += 2;
	img.height   = pgm_read_word(imgData); imgData += 2;
	img.imgData  = imgData;
	img.highestX = originX;
	img.mode     = mode;
	
	if(clipX >= img.width) {
		clipX = img.width - 1;
	}
	if(clipY >= img.height) {
		clipY = img.height - 1;
	}
	if(clipWidth + clipX > img.width) {
		clipWidth = img.width - clipX;
	}
	if(clipHeight + clipY > img.height) {
		clipHeight = img.height - clipY;
	}
	
	if(clipWidth == 0 && clipHeight == 0 && clipX == 0 && clipY == 0) {
		clipX = 0;
		clipY = 0;
		clipWidth  = img.width;
		clipHeight = img.height;
	}
	
	if(originX < 0) {
		clipWidth += originX;
		originX = 0;
	}
	if(originY < 0) {
		clipHeight += originY;
		originY = 0;
	}
	if(!(originX >= SED1520_SCREENWIDTH || originY >= SED1520_SCREENHEIGHT)) {
		img.x -= clipX;
		img.y -= clipY;
		
		sed1520ShaderExecute((SED1520SHADERCALLBACK) sed1520DrawImageShader, originX, originY, clipWidth, clipHeight, (void*) &img);
	}
	
	return img.highestX;
}

void sed1520DrawText(char* text, uint8_t* font, uint8_t mode, int16_t originX, int16_t originY, uint8_t fixedWidth, int8_t deltaCharDist) {
	uint16_t charWidth  = pgm_read_word(font) / 32;
	uint16_t charHeight = pgm_read_word(font + 2) / 3;
	int16_t x = originX;
	int16_t y = originY;
	int16_t charEndX = x;
	
	if(!fixedWidth) {
		deltaCharDist ++;
	}
	
	while(*text != 0) {
		uint8_t tableX;
		uint8_t tableY;
		
		if(*text > ' ' && *text < 128) {
			tableX = *text - ' ';
			
			tableY = 0;
			while(tableX >= 32) {
				tableX -= 32;
				tableY ++;
			}
			
			charEndX = sed1520DrawImage(font, mode, x, y, tableX * charWidth, tableY * charHeight, charWidth, charHeight);
		} else {
			charEndX = x + (charWidth * 3) / 2;
		}
		
		if(*text == '\n') {
			x = originX;
			y += charHeight;
		} else {
			if(fixedWidth) {
				x += charWidth + deltaCharDist;
			} else {
				x = charEndX + deltaCharDist;
			}
		}
		
		text ++;
	}
}

uint8_t sed1520FillShader(uint8_t x, uint8_t y, uint8_t value, uint8_t* pixelValue) {
	return *pixelValue;
}

void sed1520DrawBox(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t filled, uint8_t pixelValue) {
	if(filled) {
		sed1520ShaderExecute((SED1520SHADERCALLBACK) sed1520FillShader, x, y, width, height, (void*) &pixelValue);
	} else {
		width --;
		height --;
		
		sed1520DrawLine(x, y, x + width, y, pixelValue);
		sed1520DrawLine(x, y + height, x + width, y + height, pixelValue);
		
		sed1520DrawLine(x, y, x, y + height, pixelValue);
		sed1520DrawLine(x + width, y, x + width, y + height, pixelValue);
	}
}

void sed1520DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t pixelValue) {
	if(x1 == x2) {
		if(y1 > y2) {
			uint8_t swapValue;
			
			swapValue = y1;
			y1 = y2;
			y2 = swapValue;
		}
		
		for(uint8_t y = y1; y < y2; y ++) {
			sed1520SetPixel(x1, y, pixelValue);
		}
		sed1520SetPixel(x2, y2, pixelValue);
	} else if(y1 == y2) {
		if(x1 > x2) {
			uint8_t swapValue;
			
			swapValue = x1;
			x1 = x2;
			x2 = swapValue;
		}
		
		for(uint8_t x = x1; x < x2; x ++) {
			sed1520SetPixel(x, y1, pixelValue);
		}
		sed1520SetPixel(x2, y2, pixelValue);
	} else if(x1 > x2) {
		sed1520DrawLine(x2, y2, x1, y1, pixelValue);
	} else {
		uint8_t oldX = x1;
		uint8_t oldY = y1;
		int16_t nominator   = (int16_t) y2 -  (int16_t) y1;
		int16_t denominator = (int16_t) x2 -  (int16_t) x1;
		
		for(int16_t x = x1 + 1; x <= x2; x ++) {
			int16_t y = (x - (int16_t) x1) * nominator;
			
			y /= denominator;
			y += (int16_t) y1;
			
			sed1520DrawLine(oldX, oldY, oldX, (uint16_t) y, pixelValue);
			
			oldX = x;
			oldY = y;
		}
		
		sed1520SetPixel(x2, y2, pixelValue);
	}
}

