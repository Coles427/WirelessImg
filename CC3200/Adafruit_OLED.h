/*
 * Adafruit_OLDED.h
 *
 *  Created on: Jan 27, 2021
 *      Author: Cole
 */

#ifndef ADAFRUIT_OLED_H_
#define ADAFRUIT_OLED_H_

void Adafruit_Init(void);
unsigned int Color565(unsigned char r, unsigned char g, unsigned char b);
void drawFastHLine(int x, int y, int w, unsigned int color);
void drawFastVLine(int x, int y, int h, unsigned int color);
void drawPixel(int x, int y, unsigned int color);
void fillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int fillcolor);
void fillScreen(unsigned int fillcolor);
void goTo(int x, int y);
void invert(char v);
void writeCommand(unsigned char c);
void writeData(unsigned char c);

#endif /* ADAFRUIT_OLED_H_ */
