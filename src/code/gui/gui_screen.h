//
// Created by screemer on 2019-01-24.
//
#pragma once

#include <SDL2/SDL_render.h>
#include "gui_sdl_wrapper.h"

//********************
// GuiScreen
//********************
class GuiScreen {
public:
    GuiScreen(SDL_Shared<SDL_Renderer> _renderer) : renderer(_renderer) {}
    SDL_Shared<SDL_Renderer> renderer;

    virtual void init() {};
    virtual void render()=0;
    virtual void loop()=0;

    void show()
    {
        init();
        render();
        loop();
    }
};
