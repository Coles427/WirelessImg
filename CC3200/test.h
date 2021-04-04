/*
 * test.h
 *
 *  Created on: Jan 27, 2021
 *      Author: Cole
 */

#ifndef TEST_H_
#define TEST_H_

void delay(unsigned long ulCount);
void lcdTestPattern(void);
void lcdTestPattern2(void);
void testdrawcircles(unsigned char radius, unsigned int color);
void testdrawrects(unsigned int color);
void testfastlines(unsigned int color1, unsigned int color2);
void testfillcircles(unsigned char radius, unsigned int color);
void testfillrects(unsigned int color1, unsigned int color2);
void testlines(unsigned int color);
void testroundrects();
void testtriangles();

#endif /* TEST_H_ */
