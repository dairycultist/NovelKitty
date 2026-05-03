/*
 * game_kitty.c
 * The implementation of the GameKitty framework.
 * 
 * Copyright 2026 dairycultist
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <emscripten.h>

#include "game_kitty.h"

/*
 * rendering
 */
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *screen_buffer;

static SDL_Texture *tex_font;
static SDL_Texture *tex_textbox;
static SDL_Texture *tex_choicebox;

static void draw_texture(SDL_Texture *tex, int x, int y, int flip) {

	int w, h;

	SDL_QueryTexture(tex, NULL, NULL, &w, &h);

	SDL_Rect copy_rect = { 0, 0, w, h };
	SDL_Rect paste_rect = { x, y, w, h };

	SDL_RenderCopyEx(renderer, tex, &copy_rect, &paste_rect, 0.0, NULL, flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

static void draw_char(char c, int x, int y) {

	SDL_Rect copy_rect = { c * CHAR_W, 0, CHAR_W, CHAR_H };
	SDL_Rect paste_rect = { x, y, CHAR_W, CHAR_H };

	SDL_RenderCopyEx(renderer, tex_font, &copy_rect, &paste_rect, 0.0, NULL, SDL_FLIP_NONE);
}

static void draw_string(const char *string, int x, int y) {

	int dx = 0;
	int dy = 0;

	// draw
	while (*string != '\0') {

		if (*string == ' ') {
			dx++;
		} else if (*string >= 'A' && *string <= 'Z') {
			draw_char(*string - 'A', x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else if (*string >= 'a' && *string <= 'z') {
			draw_char(*string - 'a' + 32, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else if (*string == '\n') {
			dx = 0;
			dy++;
		} else if (*string == ':') {
			draw_char(26, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else if (*string == '!') {
			draw_char(27, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else if (*string == '.') {
			draw_char(28, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else if (*string == ',') {
			draw_char(29, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else if (*string == '\'') {
			draw_char(30, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else if (*string == '?') {
			draw_char(31, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		} else {
			draw_char(63, x + CHAR_W * dx++, y + (CHAR_H + 2) * dy);
		}

		string++;
	}
}

/*
 * emscripten main loop
 */
static SDL_Event event;
static SDL_Rect letterbox = { 0, 0, WIDTH * 2, HEIGHT * 2 };

static Event *curr_event;
static SDL_Texture *tex_person_left;
static SDL_Texture *tex_person_right;
static SDL_Texture *tex_background;

static int mouse_x, mouse_y, mouse_clicked;

static void main_loop() {

	mouse_clicked = 0;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {

			#define MIN(a, b) ((a) > (b) ? (b) : (a))
			#define ASPECT_RATIO (WIDTH / (float) HEIGHT)

			// dynamically change letterbox based on screen resize
			letterbox.w = MIN(event.window.data1, event.window.data2 * ASPECT_RATIO);
			letterbox.h = MIN(event.window.data2, event.window.data1 / ASPECT_RATIO);

			letterbox.x = (event.window.data1 - letterbox.w) / 2;
			letterbox.y = (event.window.data2 - letterbox.h) / 2;

		} else if (event.type == SDL_MOUSEMOTION) {

			mouse_x = (event.motion.x - letterbox.x) * WIDTH / letterbox.w;
			mouse_y = (event.motion.y - letterbox.y) * HEIGHT / letterbox.h;

		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			mouse_clicked = 1;
		} else if (event.type == SDL_MOUSEBUTTONUP) {
			mouse_clicked = 0;
		}
	}

	SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); 			// clear window to grey
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, screen_buffer); 				// set render target to screen_buffer
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 			// clear screen_buffer to black
	SDL_RenderClear(renderer);

	// trigger next event with mouse click
	if (mouse_clicked && curr_event->type != TYPE_TEXT_UNPASSABLE) {

		if (curr_event->type == TYPE_CHOICE) {

			// TODO check which choice was selected

		} else {

			// progress one step, potentially consuming multiple events
			do {

				curr_event++;

				switch (curr_event->type) {

					case TYPE_SET_PERSON_LEFT:

						if (tex_person_left)
							SDL_DestroyTexture(tex_person_left);

						if (curr_event->string)
							tex_person_left = IMG_LoadTexture(renderer, curr_event->string);
						else
							tex_person_left = NULL;
						break;
					
					case TYPE_SET_PERSON_RIGHT:

						if (tex_person_right)
							SDL_DestroyTexture(tex_person_right);

						if (curr_event->string)
							tex_person_right = IMG_LoadTexture(renderer, curr_event->string);
						else
							tex_person_right = NULL;
						break;

					case TYPE_SET_BACKGROUND:

						if (tex_background)
							SDL_DestroyTexture(tex_background);

						if (curr_event->string)
							tex_background = IMG_LoadTexture(renderer, curr_event->string);
						else
							tex_background = NULL;
						break;
				}

			} while (curr_event->type != TYPE_NULL && curr_event->type != TYPE_TEXT && curr_event->type != TYPE_TEXT_UNPASSABLE && curr_event->type != TYPE_CHOICE);
		}
	}

	// render
	if (tex_background)
		draw_texture(tex_background, 0, 0, 0);

	if (tex_person_left) {
		
		int h;
		SDL_QueryTexture(tex_person_left, NULL, NULL, NULL, &h);

		draw_texture(tex_person_left, 0, HEIGHT - h, 0);
	}

	if (tex_person_right) {
		
		int w, h;
		SDL_QueryTexture(tex_person_right, NULL, NULL, &w, &h);

		draw_texture(tex_person_right, WIDTH - w, HEIGHT - h, 1);
	}

	if (curr_event->type == TYPE_CHOICE) {

		int i = 0;

		// render all choices
		do {

			draw_texture(tex_choicebox, 16, 96 + i * 36, 0);
			draw_string(curr_event[i].string, 24, 104 + i * 36);

			i++;

		} while (curr_event[i].type == TYPE_CHOICE);

		// render text following it
		draw_texture(tex_textbox, 0, 288, 0);
		draw_string(curr_event[i].string, 8, 296);

	} else {

		draw_texture(tex_textbox, 0, 288, 0);
		draw_string(curr_event->string, 8, 296);
	}

	SDL_SetRenderTarget(renderer, NULL); 						// reset render target back to window
	SDL_RenderCopy(renderer, screen_buffer, NULL, &letterbox); 	// render screen_buffer
	SDL_RenderPresent(renderer); 								// present rendered content to screen
}

/*
 * main logic
 */
int main(void) {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "\x1b[31m[GameKitty] Error initializing SDL:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow("GameKitty", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);

	if (!window) {
		fprintf(stderr, "\x1b[31m[GameKitty] Error creating window:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
    }

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer) {
		fprintf(stderr, "\x1b[31m[GameKitty] Error creating renderer:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
	}

	screen_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);

	if (!screen_buffer) {
		fprintf(stderr, "\x1b[31m[GameKitty] Error creating screen buffer:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
	}

	tex_font = IMG_LoadTexture(renderer, "assets/font.png");

	if (!tex_font) {
		fprintf(stderr, "\x1b[31m[GameKitty] Could not read font texture\n\x1b[0m");
		return 1;
	}

	tex_textbox = IMG_LoadTexture(renderer, "assets/textbox.png");

	if (!tex_textbox) {
		fprintf(stderr, "\x1b[31m[GameKitty] Could not read textbox texture\n\x1b[0m");
		return 1;
	}

	tex_choicebox = IMG_LoadTexture(renderer, "assets/choicebox.png");

	if (!tex_choicebox) {
		fprintf(stderr, "\x1b[31m[GameKitty] Could not read choicebox texture\n\x1b[0m");
		return 1;
	}

	printf("\n[GameKitty] Good to go!\n\n");

	// init
	curr_event = get_start_events();

	// start program
	emscripten_set_main_loop(main_loop, 0, 1);

	// this code is never reached

	// SDL_DestroyTexture(tex_font);
	// SDL_DestroyTexture(tex_textbox);

	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(window);
	// SDL_Quit();

	return 0;
}