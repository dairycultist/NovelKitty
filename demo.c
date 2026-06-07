// This file is intentionally left unlicensed under Apache License, Version 2.0.

#include "nk/novel_kitty.h"

void sequence() {

	set_text("Hello!");
	present();

	set_background("assets/forest.png");
	set_text("Now we're in a forest.");
	present();

	set_person_left("assets/person.png");
	set_text("Here's a person.");
	present();

	set_person_right("assets/person_angry.png");
	set_text("Here's another one, but angry.");
	present();

	set_person_left("assets/person_angry.png");
	set_text("Now they're BOTH angry!");
	present();

	set_text("LEFT: I'm so angry, OMG!!");
	present();

	set_text("RIGHT: same lmfao!! THAT'S SO\nCRAZY??");
	present();

	add_choice("LEFT");
	add_choice("RIGHT");
	set_text("who is angrier?");
	int choice = present_choices();

	if (choice == 0) {
		set_person_right("");
		set_text("You picked LEFT!");
	} else {
		set_person_left("");
		set_text("You picked RIGHT!");
	}
	present();

	set_person_left("");
	set_person_right("");
	set_background("");
	set_text("the end :)");
	present();
}