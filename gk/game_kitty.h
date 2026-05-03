/*
 * game_kitty.h
 * The header file of the GameKitty framework.
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

#ifndef GK_MAIN
#define GK_MAIN

// screen size
#define WIDTH 512
#define HEIGHT 384

// width and height of text characters
#define CHAR_W 16
#define CHAR_H 16

#define TYPE_NULL 0x00
#define TYPE_TEXT 0x01
#define TYPE_TEXT_UNPASSABLE 0x02
#define TYPE_CHOICE 0x03
#define TYPE_SET_PERSON_LEFT 0x04
#define TYPE_SET_PERSON_RIGHT 0x05
#define TYPE_SET_BACKGROUND 0x06

typedef struct {

	unsigned char type;
	char *string;
	// TODO for TYPE_CHOICE, have a pointer to a Scene
	// TODO for TYPE_CHOICE, encode flag stuff
	// - the flag (boolean) requirements for it to appear as a choice to select at all
	// - the flags it sets/unsets by using it

} Event;

// Flags let you create situations where you're only able to reach a scene after completing other scenes first.
// It's not insanely powerful, but it allows for stories more complex than simple dialogue trees.

Event *get_start_events();

#endif