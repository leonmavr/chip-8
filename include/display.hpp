#ifndef DISPLAY_HPP
#define DISPLAY_HPP 
//#include <SDL2/SDL.h>
#include <unordered_map>
#include <cinttypes>
#include <any>
#include <vector>


class Display {
    public:
        Display (std::string iniFile) {
            m_w = 640; //std::any_cast<int>(m_iniSettings["i_screen_width"]);
            m_h = 320; //std::any_cast<int>(m_iniSettings["i_screen_height"]);
            init();									// initialise SDL resources
        };
        ~Display () { 
#if 0
            free(m_window);
            free(m_renderer);
#endif
        };

    protected:
        void cls();									// clear display window 
        void close();								// close display window 
        void renderAll(uint8_t (&array2D)[32][64]); // render the input 2D array in the display window; values of 1 rendered to white, 0 to black
        uint8_t m_display[32][64] = {0};

    private:
        unsigned m_w;
        unsigned m_h;
        std::vector <uint8_t> m_colourBg;
        std::vector <uint8_t> m_colourFg;
#if 0
        SDL_Window* m_window;
        SDL_Renderer* m_renderer;
#endif
        /**
         * @brief Initialises m_window and m_renderer, also clears the display window
         */
        void init();
        /**
         * @brief Draws pixel at scaled location (x, y) (scaled by size of display window). If colour == 0 draws black, else white
         *
         * @param x	x-ordinate of pixel to be drawn
         * @param y	y-ordinate of pixel to be drawn
         * @param colour Pixel colour; 0 for background, 1 for foreground 
         */
        void drawPixelXY(unsigned x, unsigned y, unsigned colour);
        /**
         * @brief Converts a RGB hex string to a R, G, B triplet
         *
         * @param strHex A hex string defining a colour, preceeded optionally by #, e.g. #AABBCC 
         * @param vecrgb A vector where the R, G, B components of the above colour will be written to 
         */
        void hex2rgb(std::string strHex, std::vector<uint8_t>& vecrgb);
};

#endif /* DISPLAY_HPP */
