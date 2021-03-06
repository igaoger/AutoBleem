#include "gui_Menu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <cassert>
#include "../lang.h"

using namespace std;

//*******************************
// void GuiMenu::init()
//*******************************
void GuiMenu::init()
{
    gui = Gui::getInstance();
    font = gui->themeFont;

    maxVisible = atoi(gui->themeData.values["lines"].c_str());

    if (useSmallerFont) {
        // sometimes the left column will overwrite into the right column.
        // and the second column sometimes go off the right side.
        font = gui->themeFonts[FONT_15_BOLD];   // use a smaller font
        // compute the larger number of rows we can now display
        string themeFontSizeString = gui->themeData.values["fsize"];
        int themeFontSize = atoi(themeFontSizeString.c_str());
        maxVisible = ( ((float)themeFontSize) / ((float)15) ) * ((float) maxVisible);
        lastVisibleIndex = firstVisibleIndex + maxVisible - 1;
    }
}

//*******************************
// GuiMenu::adjustPageBy
//*******************************
void GuiMenu::adjustPageBy(int moveBy) {
    selected += moveBy;
    firstVisibleIndex += moveBy;
    lastVisibleIndex += moveBy;
}

//*******************************
// GuiMenu::computePagePosition
//*******************************
void GuiMenu::computePagePosition() {
    if (lines.size() == 0) {
        selected = 0;
        firstVisibleIndex = 0;
        lastVisibleIndex = 0;
    } else {
        bool AllLinesFitOnOnePage = lines.size() <= maxVisible;
        bool selectedIsOnTheFirstPage = selected < maxVisible;
        bool selectedIsOnTheLastPage = selected >= (lines.size() - maxVisible);

        if (AllLinesFitOnOnePage) {
            firstVisibleIndex = 0;
        } else if (selectedIsOnTheFirstPage) {
            firstVisibleIndex = 0;
        } else if (selectedIsOnTheLastPage) {
            firstVisibleIndex = lines.size() - maxVisible;
        } else {
            firstVisibleIndex = selected - (maxVisible / 2);
        }
        lastVisibleIndex = firstVisibleIndex + maxVisible - 1;
    }
}

//*******************************
// GuiMenu::renderLines
//*******************************
void GuiMenu::renderLines() {
    if (selected >= 0 && lines.size() > 0) {
        int row = firstRow;
        for (int i = firstVisibleIndex; i <= lastVisibleIndex; i++) {
            if (i < 0 || i >= lines.size()) {
                break;
            }
            if (menuType == Menu_Plain) {
                gui->renderTextLine(lines[i], row, offset, POS_LEFT, 0, font);
            } else if (menuType == Menu_TwoColumns) {
                gui->renderTextLineToColumns(lines_L[i], lines_R[i], xoffset_L, xoffset_R, row, offset, font);
            } else {
                assert(false);
            }
            row++;
        }
    }
}

//*******************************
// GuiMenu::renderSelectionBox
//*******************************
void GuiMenu::renderSelectionBox() {
    if (!lines.size() == 0) {
        gui->renderSelectionBox(selected - firstVisibleIndex + firstRow, offset, 0, font);
    }
}

//*******************************
// GuiMenu::render
//*******************************
void GuiMenu::render()
{
    SDL_RenderClear(renderer);
    gui->renderBackground();
    gui->renderTextBar();
    offset = gui->renderLogo(true);
    gui->renderTextLine(title, 0, offset, POS_CENTER);

    if (firstRender) {
        computePagePosition();
        firstRender = false;
    }
    renderLines();
    renderSelectionBox();

    gui->renderStatus(statusLine());
    SDL_RenderPresent(renderer);
}

//*******************************
// GuiMenu::statusLine
//*******************************
// the default status line for menus.  override if needed.
string GuiMenu::statusLine() {
    return _("Entry")+" " + to_string(selected + 1) + "/" + to_string(lines.size()) +
             "    |@L1|/|@R1| " + _("Page") +
             "   |@X| " + _("Select") +
             "   |@O| " + _("Close") + " |";
}

//*******************************
// GuiMenu::arrowDown
//*******************************
void GuiMenu::arrowDown() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    if (lines.size() > 1) {
        if (selected >= lines.size() - 1) {
            selected = 0;
            computePagePosition();
        } else if (selected == lastVisibleIndex) {
            adjustPageBy(1);
        } else {
            ++selected;
        }
    }
    render();
}

//*******************************
// GuiMenu::arrowUp
//*******************************
void GuiMenu::arrowUp() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    if (lines.size() > 1) {
        if (selected <= 0) {
            selected = lines.size() - 1;
            computePagePosition();
        } else if (selected == firstVisibleIndex) {
            adjustPageBy(-1);
        } else {
            --selected;
        }
    }
    render();
}

//*******************************
// GuiMenu::pageDown
//*******************************
void GuiMenu::pageDown() {
    Mix_PlayChannel(-1, gui->home_up, 0);
    if (lines.size() > 1) {
        if (lastVisibleIndex + maxVisible >= lines.size()) {
            selected = lines.size() - 1;
            computePagePosition();
        } else {
            adjustPageBy(maxVisible);
        }
    }
    render();
}

//*******************************
// GuiMenu::pageUp
//*******************************
void GuiMenu::pageUp() {
    Mix_PlayChannel(-1, gui->home_down, 0);
    if (lines.size() > 1) {
        if (firstVisibleIndex - maxVisible < 0) {
            selected = 0;
            computePagePosition();
        } else {
            adjustPageBy(-maxVisible);
        }
    }
    render();
}

//*******************************
// GuiMenu::doHome
//*******************************
void GuiMenu::doHome() {
    Mix_PlayChannel(-1, gui->home_down, 0);
    if (lines.size() > 1) {
        selected = 0;
        computePagePosition();
    }
    render();
}

//*******************************
// GuiMenu::doEnd
//*******************************
void GuiMenu::doEnd() {
    Mix_PlayChannel(-1, gui->home_down, 0);
    if (lines.size() > 1) {
        selected = lines.size() - 1;
        computePagePosition();
    }
    render();
}

//*******************************
// GuiMenu::doCircle
//*******************************
void GuiMenu::doCircle() {
    Mix_PlayChannel(-1, gui->cancel, 0);
    cancelled = true;
    menuVisible = false;
}

//*******************************
// GuiMenu::doCross
//*******************************
void GuiMenu::doCross() {
    Mix_PlayChannel(-1, gui->cursor, 0);
    cancelled = false;
    if (!lines.empty())
    {
        menuVisible = false;
    }
}

//*******************************
// GuiMenu::handlePowerShutdownAndQuit
//*******************************
// returns true if applicable event type and it was handled
bool GuiMenu::handlePowerShutdownAndQuit(SDL_Event &e) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.scancode == SDL_SCANCODE_SLEEP) {
            gui->drawText(_("POWERING OFF... PLEASE WAIT"));
            Util::powerOff();
            return true;    // but it will never get here
        }
    } else if (e.type == SDL_QUIT) {     // this is for pc Only
        menuVisible = false;
        return true;
    }
    return false;
}

//*******************************
// GuiMenu::loop
//*******************************
void GuiMenu::loop()
{
    menuVisible = true;
    while (menuVisible) {
        gui->watchJoystickPort();
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (handlePowerShutdownAndQuit(e))
                continue;

            switch (e.type) {
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_DOWN)
                        arrowDown();
                    if (e.key.keysym.sym == SDLK_UP)
                        arrowUp();
                    if (e.key.keysym.sym == SDLK_PAGEDOWN)
                        pageDown();
                    if (e.key.keysym.sym == SDLK_PAGEUP)
                        pageUp();
                    if (e.key.keysym.sym == SDLK_HOME)
                        doHome();
                    if (e.key.keysym.sym == SDLK_END)
                        doEnd();

                    if (e.key.keysym.sym == SDLK_RETURN)
                        doEnter();
                    if (e.key.keysym.sym == SDLK_DELETE)
                        doDelete();
                    if (e.key.keysym.sym == SDLK_TAB)
                        doTab();
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                        doEscape();

                break;

                case SDL_JOYAXISMOTION:
                case SDL_JOYHATMOTION:

                    if (gui->mapper.isDown(&e)) {
                        arrowDown();
                    }
                    if (gui->mapper.isUp(&e)) {
                        arrowUp();
                    }

                    break;
                case SDL_JOYBUTTONDOWN:
                    if (e.jbutton.button == gui->_cb(PCS_BTN_R1,&e)) {
                        pageDown();
                    };
                    if (e.jbutton.button == gui->_cb(PCS_BTN_L1,&e)) {
                        pageUp();
                    };

                    if (e.jbutton.button == gui->_cb(PCS_BTN_CIRCLE,&e)) {
                        doCircle();
                    };

                    if (e.jbutton.button == gui->_cb(PCS_BTN_CROSS,&e)) {
                        doCross();
                    };

                    if (e.jbutton.button == gui->_cb(PCS_BTN_TRIANGLE,&e)) {
                        doTriangle();
                    };

                    if (e.jbutton.button == gui->_cb(PCS_BTN_SQUARE,&e)) {
                        doSquare();
                    };

                    if (e.jbutton.button == gui->_cb(PCS_BTN_START,&e)) {
                        doStart();
                    };

                    if (e.jbutton.button == gui->_cb(PCS_BTN_SELECT,&e)) {
                        doSelect();
                    };
            }
        }
    }
}
