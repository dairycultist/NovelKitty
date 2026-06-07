# This file is intentionally left unlicensed under Apache License, Version 2.0.

.PHONY: run clean

build/index.html: *.c nk/novel_kitty.c assets/*
	mkdir -p build
	emcc *.c nk/novel_kitty.c -o build/index.html -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --embed-file assets/
	sudo chmod -R 777 build
	cp nk/index.html build/index.html

run: build/index.html
	emrun build/index.html --no_browser

clean:
	rm build/index.js build/index.wasm build/index.html
