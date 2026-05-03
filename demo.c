// This file is intentionally left unlicensed under Apache License, Version 2.0.

#include "gk/game_kitty.h"

static Event the_forest[], pick_left[], pick_right[], the_end[];

static Event the_forest[] = {
	
	{ TYPE_TEXT, "Hello!" },

	{ TYPE_SET_BACKGROUND, "assets/forest.png" },
	{ TYPE_TEXT, "Now we're in a forest." },

	{ TYPE_SET_PERSON_LEFT, "assets/person.png" },
	{ TYPE_TEXT, "Here's a person." },

	{ TYPE_SET_PERSON_RIGHT, "assets/person_angry.png" },
	{ TYPE_TEXT, "Here's another one, but angry." },

	{ TYPE_SET_PERSON_LEFT, "assets/person_angry.png" },
	{ TYPE_TEXT, "Now they're BOTH angry!" },

	{ TYPE_TEXT, "LEFT: I'm so angry, OMG!!" },

	{ TYPE_TEXT, "RIGHT: same lmfao!! THAT'S SO\nCRAZY??" },

	{ TYPE_CHOICE, "LEFT", pick_left },
	{ TYPE_CHOICE, "RIGHT", pick_right },
	{ TYPE_TEXT, "who is angrier?" },
};

static Event pick_left[] = {

	{ TYPE_SET_PERSON_RIGHT, 0 },
	{ TYPE_TEXT, "you picked left!" },

	{ TYPE_GOTO, 0, the_end }
};

static Event pick_right[] = {

	{ TYPE_SET_PERSON_LEFT, 0 },
	{ TYPE_TEXT, "you picked right!" },

	{ TYPE_GOTO, 0, the_end }
};

static Event the_end[] = {

	{ TYPE_SET_BACKGROUND, 0 },
	{ TYPE_SET_PERSON_LEFT, 0 },
	{ TYPE_SET_PERSON_RIGHT, 0 },
	{ TYPE_TEXT_UNPASSABLE, "The end...?" }
};

Event *get_start_events() {
	return the_forest;
}