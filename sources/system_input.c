/*
===========================================================================
File:		system_input.c
Author: 	Clinton Freeman
Created on: Sep 29, 2010

TODO: the input module should just forward player requests to the model
to handle and not do it directly.
===========================================================================
*/

#include "headers/SDL/SDL.h"

#include "headers/common.h"
#include "headers/mathlib.h"

#include "headers/input_public.h"
#include "headers/player.h"

static int keys_down[SDLK_LAST];

void input_keyDown(SDLKey k) { keys_down[k] = etrue; if(k == SDLK_ESCAPE || k == SDLK_q) user_exit = etrue; }
void input_keyUp  (SDLKey k) { keys_down[k] = efalse; }

/*
 * input_mouseMove
 */
void input_mouseMove(int dx, int dy)
{
	float halfWinWidth, halfWinHeight;

	static eboolean firstTime = etrue;

	if(firstTime)
	{
		firstTime = efalse;
		return;
	}

	halfWinWidth  = (float)WINDOW_WIDTH  / 2.0;
	halfWinHeight = (float)WINDOW_HEIGHT / 2.0;

	dx -= halfWinWidth; dy -= halfWinHeight;

	player_rotateZ(-dx/2.0);
	player_rotateX(dy/2.0);

	//Reset cursor to center
	SDL_WarpMouse(halfWinWidth, halfWinHeight);
}

static void input_increaseSpeed(float *out)
{
	//Increase the speed...
	*out += 0.65;

	//Clamp it
	if(*out > MAX_SPEED)
		*out = MAX_SPEED;
}

static void input_decreaseSpeed(float *out)
{
	//Decrease the speed...
	*out -= 0.65;

	//Clamp it
	if(*out < -MAX_SPEED)
		*out = -MAX_SPEED;
}

static void input_decaySpeed(float *out)
{
	(*out) *= 0.8;
}

/*
 * input_calcForwardSpeed
 */
static void input_calcForwardSpeed(float *old)
{
	//If we wish to move forward
	if(keys_down[SDLK_w])
		input_decreaseSpeed(old);

	//If we wish to move backward
	else if(keys_down[SDLK_s])
		input_increaseSpeed(old);

	else
		input_decaySpeed(old);
}

/*
 * input_calcStrafeSpeed
 */
static void input_calcStrafeSpeed(float *old)
{
	//If we wish to move left
	if(keys_down[SDLK_a])
		input_decreaseSpeed(old);

	//If we wish to move right
	else if(keys_down[SDLK_d])
		input_increaseSpeed(old);

	else
		input_decaySpeed(old);
}

/*
 * input_calcUpSpeed
 */
static void input_calcUpSpeed(float *old)
{
	//If we wish to move up
	if(keys_down[SDLK_SPACE])
		input_increaseSpeed(old);

	//If we wish to move down
	else if(keys_down[SDLK_c])
		input_decreaseSpeed(old);

	else
		input_decaySpeed(old);
}

/*
 * input_update
 */
void input_update()
{
	float forwardSpeed, strafeSpeed, upSpeed;

#ifdef CONVEXHULL
	if(keys_down[SDLK_i])
	{
		generateRandomPoints();
		initializeHull();
		generateHull();
	}
#endif

	player_getForwardSpeed (&forwardSpeed);
	player_getStrafeSpeed  (&strafeSpeed);
	player_getUpSpeed      (&upSpeed);

	input_calcForwardSpeed (&forwardSpeed);
	input_calcStrafeSpeed  (&strafeSpeed);
	input_calcUpSpeed      (&upSpeed);

	player_setForwardSpeed (forwardSpeed);
	player_setStrafeSpeed  (strafeSpeed);
	player_setUpSpeed      (upSpeed);
}
