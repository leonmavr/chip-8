//#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include "display.hpp" 
#include "term.h" 


void Display::init() {
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    TPRINT_HIDE_CURSOR();
#if 0
    // see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
    SDL_Init(SDL_INIT_EVERYTHING);
    m_window = SDL_CreateWindow("Display", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, m_w, m_h, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);

    Display::cls();
    //Pause for 100 ms
    SDL_Delay(100);
#endif
}


void Display::close() {
#if 0
    // see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
    //Destroy the class's renderer 
    SDL_DestroyRenderer(m_renderer);
    //Destroy the class's window
    SDL_DestroyWindow(m_window);
    //Close all the systems of SDL initialized at the top
    SDL_Quit();
#endif
}


void Display::drawPixelXY(unsigned x, unsigned y, unsigned colour) {
#if 0
    // define the pixel to draw
    SDL_Rect pixel;
    // The original Chip-8 display was 64x32 - scale this up to show it properly
    unsigned scaleX = static_cast<unsigned>(m_w / 64);
    unsigned scaleY = static_cast<unsigned>(m_h / 32);
    pixel.x = x * scaleX;
    pixel.y = y * scaleY;
    pixel.w = scaleX;
    pixel.h = scaleY;

    if (colour == 1) {
        // renderer, r, g, b, a
        SDL_SetRenderDrawColor(m_renderer, m_colourFg[0], m_colourFg[1], m_colourFg[2], 0xff);
    } else
        SDL_SetRenderDrawColor(m_renderer, m_colourBg[0], m_colourBg[1], m_colourBg[2], 0);
    SDL_RenderFillRect(m_renderer, &pixel);
#endif
}


void Display::renderAll(unsigned char(&array2D)[32][64]) {
    TPRINT_GOTO_TOPLEFT();
    for (unsigned row = 0; row < 32; row++) {
        for (unsigned col = 0; col < 64; col++) {
            //drawPixelXY(col, row, static_cast<unsigned>(array2D[row][col] != 0));
            if (array2D[row][col] != 0) {
                TPRINT_PRINT_AT(col, row+1, '#');
            } else {
                TPRINT_PRINT_AT(col, row+1, ' ');
            }
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
}


void Display::cls() {
    TPRINT_HIDE_CURSOR();
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    // clear the display array first
    for (int row = 0; row < 32; row++) {
        for (int col= 0; col < 64; col++){
            m_display[row][col] = 0;
            drawPixelXY(col, row, 0);
        }
    }
}


void Display::hex2rgb(std::string strHex, std::vector<uint8_t>& vecrgb) { }
