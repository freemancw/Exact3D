/*
===========================================================================
File:		timer.h
Author: 	Clinton Freeman
Created on: Sep 29, 2010
===========================================================================
*/

#ifndef TIMER_H_
#define TIMER_H_

typedef struct
{
	int current;
	int updated;
	int delta;
	int accumulated;
}
Timer;

extern Timer timer;

#endif /* TIMER_H_ */
