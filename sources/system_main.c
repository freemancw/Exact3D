/*
===========================================================================
File: 		system_main.c
Author: 	Clinton Freeman
Created on: Sep 29, 2010
===========================================================================
*/

#include "headers/SDL/SDL.h"
#include "headers/SDL/SDL_main.h"
#include "headers/SDL/SDL_opengl.h"

#include "headers/common.h"
#include "headers/mathlib.h"

#include "headers/input_public.h"
#include "headers/renderer_public.h"
#include "headers/world_public.h"

typedef struct
{
	int current;
	int updated;
	int delta;
	int accumulated;
}
Timer;

static Timer timer;

static void timer_init();
static void timer_update();

/*
 * SDL_main
 */
int SDL_main(int argc, char* argv[])
{
	SDL_Event		event;
	SDL_Surface		*screen, *icon;

	putenv(strdup("SDL_VIDEO_CENTERED=1"));

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    icon = SDL_LoadBMP("gui/icon.bmp");
    SDL_WM_SetIcon(icon, NULL);

	SDL_WM_SetCaption("Convex Hull Testing", "Convex Hull Testing");
	SDL_ShowCursor(SDL_DISABLE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, etrue);

	//Initialize window
    // | SDL_FULLSCREEN
    screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_OPENGL | SDL_NOFRAME);
    if(!screen)
	{
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

    SDL_WarpMouse(512.0, 384.0);

    //gameState = STATE_MAINMENU;

    timer_init();
    renderer_init();
    world_init();
    //model_init();

	//Main loop
	while(!user_exit)
	{
		//Handle input
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_KEYDOWN:
				input_keyDown(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				input_keyUp(event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				input_mouseMove(event.motion.x, event.motion.y);
				break;
			case SDL_QUIT:
				user_exit = etrue;
			}
		}

		timer_update();

		while(timer.accumulated >= TIMESTEP)
		{
			input_update();
			world_update();

			timer.accumulated -= TIMESTEP;
		}

		world_lerpPositions(timer.accumulated);
		renderer_drawFrame();
	}

    SDL_Quit();

    return EXIT_SUCCESS;
}

/*
 * timer_init
 * Initializes timer struct to zero, gets current time.
 */
static void timer_init()
{
	timer.updated = timer.delta = timer.accumulated = 0;
	timer.current = SDL_GetTicks();
}

/*
 * timer_update
 * Gets time since last update, accumulates the difference.
 */
static void timer_update()
{
	timer.updated = SDL_GetTicks();
	timer.delta   = timer.updated - timer.current;
	timer.current = timer.updated;
	timer.accumulated += timer.delta;
}
