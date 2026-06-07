/*
 * novel_kitty.c
 * The implementation of the NovelKitty framework.
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
#include <pthread.h>
#include <unistd.h>

#include "novel_kitty.h"

/*
 * rendering
 */
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *screen_buffer;

static SDL_Texture *tex_font;
static SDL_Texture *tex_textbox;
static SDL_Texture *tex_choicebox;
static SDL_Texture *tex_choicebox_hovered;

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
 * functions that abstract away visual novel logic
 */
static SDL_Event event;
static SDL_Rect letterbox = { 0, 0, WIDTH * 2, HEIGHT * 2 };

static const char *requested_person_left; // since threads can't perform texture operations
static SDL_Texture *tex_person_left;

static const char *requested_person_right;
static SDL_Texture *tex_person_right;

static const char *requested_background;
static SDL_Texture *tex_background;

static const char *text;

static int mouse_x, mouse_y;

static volatile int mouse_clicked;

static const char *choices[MAX_CHOICE_COUNT];
static int choice_count;

void set_person_left(const char *filepath) {
	requested_person_left = filepath ? filepath : "";
}

void set_person_right(const char *filepath) {
	requested_person_right = filepath ? filepath : "";
}

void set_background(const char *filepath) {
	requested_background = filepath ? filepath : "";
}

void set_text(const char *value) {
	text = value;
}

void add_choice(const char *label) {
	choices[choice_count] = label;
	choice_count++;
}

void present() {

	// wait until input
	while (!mouse_clicked)
		usleep(100000); // 0.1 seconds

	mouse_clicked = 0;
}

int present_choices() {

	int choice = -1;

	// check which choice was selected and change the state accordingly
	while (choice == -1) {

		usleep(100000); // 0.1 seconds

		if (mouse_clicked) {

			for (int i = 0; i < choice_count; i++) {

				if (mouse_x >= 16 && mouse_x < 496 && mouse_y >= 96 + i * 36 && mouse_y < 128 + i * 36) {

					choice = i;
					break;
				}
			}

			mouse_clicked = 0;
		}
	}

	// remove all choices
	choice_count = 0;

	return choice;
}

static void main_loop() {

	// fulfill requested textures
	if (requested_person_left) {

		if (tex_person_left)
			SDL_DestroyTexture(tex_person_left);

		if (requested_person_left[0] != '\0')
			tex_person_left = IMG_LoadTexture(renderer, requested_person_left);
		else
			tex_person_left = NULL;

		requested_person_left = NULL;
	}

	if (requested_person_right) {

		if (tex_person_right)
			SDL_DestroyTexture(tex_person_right);

		if (requested_person_right[0] != '\0')
			tex_person_right = IMG_LoadTexture(renderer, requested_person_right);
		else
			tex_person_right = NULL;

		requested_person_right = NULL;
	}

	if (requested_background) {

		if (tex_background)
			SDL_DestroyTexture(tex_background);

		if (requested_background[0] != '\0')
			tex_background = IMG_LoadTexture(renderer, requested_background);
		else
			tex_background = NULL;

		requested_background = NULL;
	}

	// get input
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
		}
	}

	SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); 			// clear window to grey
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, screen_buffer); 				// set render target to screen_buffer
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 			// clear screen_buffer to black
	SDL_RenderClear(renderer);

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

	if (text) {

		draw_texture(tex_textbox, 0, 288, 0);
		draw_string(text, 8, 296);
	}

	for (int i = 0; i < choice_count; i++) {

		if (mouse_x >= 16 && mouse_x < 496 && mouse_y >= 96 + i * 36 && mouse_y < 128 + i * 36)
			draw_texture(tex_choicebox_hovered, 16, 96 + i * 36, 0);
		else
			draw_texture(tex_choicebox, 16, 96 + i * 36, 0);
		draw_string(choices[i], 24, 104 + i * 36);
	}

	SDL_SetRenderTarget(renderer, NULL); 						// reset render target back to window
	SDL_RenderCopy(renderer, screen_buffer, NULL, &letterbox); 	// render screen_buffer
	SDL_RenderPresent(renderer); 								// present rendered content to screen
}

/*
 * initialization
 */
int main(void) {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Error initializing SDL:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
	}

	window = SDL_CreateWindow("NovelKitty", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);

	if (!window) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Error creating window:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
    }

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Error creating renderer:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
	}

	screen_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);

	if (!screen_buffer) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Error creating screen buffer:\n%s\n\x1b[0m", SDL_GetError());
		return 1;
	}

	tex_font = IMG_LoadTexture(renderer, "assets/font.png");

	if (!tex_font) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Could not read font texture\n\x1b[0m");
		return 1;
	}

	tex_textbox = IMG_LoadTexture(renderer, "assets/textbox.png");

	if (!tex_textbox) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Could not read textbox texture\n\x1b[0m");
		return 1;
	}

	tex_choicebox = IMG_LoadTexture(renderer, "assets/choicebox.png");

	if (!tex_choicebox) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Could not read choicebox texture\n\x1b[0m");
		return 1;
	}

	tex_choicebox_hovered = IMG_LoadTexture(renderer, "assets/choicebox_hovered.png");

	if (!tex_choicebox_hovered) {
		fprintf(stderr, "\x1b[31m[NovelKitty] Could not read hovered choicebox texture\n\x1b[0m");
		return 1;
	}

	// start program
	pthread_t unused;
    int result = pthread_create(&unused, NULL, nk_sequence, NULL);
    
    if (result) {
        fprintf(stderr, "\x1b[31m[NovelKitty] Failed to start nk_sequence thread\n\x1b[0m");
		return 1;
    }

	printf("\n[NovelKitty] Good to go!\n\n");

	emscripten_set_main_loop(main_loop, 0, 1);

	// this code is never reached

	// SDL_DestroyTexture(tex_font);
	// SDL_DestroyTexture(tex_textbox);
	// SDL_DestroyTexture(tex_choicebox);
	// SDL_DestroyTexture(tex_choicebox_hovered);

	// SDL_DestroyRenderer(renderer);
	// SDL_DestroyWindow(window);
	// SDL_Quit();

	return 0;
}