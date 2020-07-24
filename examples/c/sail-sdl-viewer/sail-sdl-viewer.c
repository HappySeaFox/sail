/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "sail-common.h"
#include "sail.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <PATH TO IMAGE>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to init video: %s\n", SDL_GetError());
        return 1;
    }

    /* Load the image. */
    struct sail_image *image;

    /*
     * sail_read_file() reads the image and outputs pixels in the BPP32-RGBA pixel format.
     * If you need to control output pixel formats, consider switching to the deep diver API.
     */
    SAIL_TRY(sail_read_file(argv[1], &image));

    /* Create an SDL surface from the image data. */
    unsigned bytes_per_line;
    SAIL_TRY(sail_bytes_per_line(image->width, image->pixel_format, &bytes_per_line));

    const bool is_rgba = image->pixel_format == SAIL_PIXEL_FORMAT_BPP32_RGBA;

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(image->pixels,
                                                    image->width,
                                                    image->height,
                                                    is_rgba ? 32 : 24,
                                                    bytes_per_line,
                                                    0x000000ff,
                                                    0x0000ff00,
                                                    0x00ff0000,
                                                    is_rgba ? 0xff000000 : 0);

    if (surface == NULL) {
        fprintf(stderr, "Failed to create surface: %s\n", SDL_GetError());
        sail_finish();
        return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;

    /* Create a new window and a renderer. */
    if (SDL_CreateWindowAndRenderer(800, 500, SDL_WINDOW_RESIZABLE, &window, &renderer) != 0) {
        fprintf(stderr, "Failed to create a window: %s\n", SDL_GetError());
        sail_finish();
        return 1;
    }

    SDL_SetWindowTitle(window, "SDL SAIL demo");

    /* Scale textures nicely. */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    /* Create a new SDL texture. */
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == NULL) {
        fprintf(stderr, "Failed to create a texture: %s\n", SDL_GetError());
        sail_finish();
        return 1;
    }

    /* We don't need the image data anymore. */
    sail_destroy_image(image);

    SDL_FreeSurface(surface);

    while (true) {
        /* Handle events. */
        SDL_Event event;

        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }

        /* Draw the texture. */
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    sail_finish();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
