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
#include "sail-manip.h"

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
    SAIL_TRY(sail_load_image_from_file(argv[1], &image));

    /* Convert to BPP32-RGBA. */
    struct sail_image *image_converted;
    SAIL_TRY(sail_convert_image_with_options(image, SAIL_PIXEL_FORMAT_BPP32_RGBA, NULL, &image_converted));

    /* We don't need the original image anymore. */
    sail_destroy_image(image);
    image = NULL;

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(image_converted->pixels,
                                                    image_converted->width,
                                                    image_converted->height,
                                                    32,
                                                    image_converted->bytes_per_line,
                                                    0x000000ff,
                                                    0x0000ff00,
                                                    0x00ff0000,
                                                    0xff000000);

    if (surface == NULL) {
        fprintf(stderr, "Failed to create surface: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;

    /* Create a new window and a renderer. */
    if (SDL_CreateWindowAndRenderer(800, 500, SDL_WINDOW_RESIZABLE, &window, &renderer) != 0) {
        fprintf(stderr, "Failed to create a window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetWindowTitle(window, "SDL SAIL demo");

    /* Scale textures nicely. */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    /* Create a new SDL texture. */
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == NULL) {
        fprintf(stderr, "Failed to create a texture: %s\n", SDL_GetError());
        return 1;
    }

    /* We don't need the image data anymore. */
    sail_destroy_image(image_converted);

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

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return SAIL_OK;
}
