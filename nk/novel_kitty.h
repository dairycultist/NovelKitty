/*
 * novel_kitty.h
 * The header file of the NovelKitty framework.
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

#ifndef NK_MAIN
#define NK_MAIN

#define MAX_CHOICE_COUNT 4

// screen size
#define WIDTH 512
#define HEIGHT 384

// width and height of text characters
#define CHAR_W 16
#define CHAR_H 16

void *nk_sequence(void *unused);

void set_person_left(const char *filepath); // to clear the value, use an empty string ("") or NULL
void set_person_right(const char *filepath);
void set_background(const char *filepath);
void set_text(const char *text);
void add_choice(const char *label);
void present();
int present_choices();

#endif