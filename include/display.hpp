#ifndef DISPLAY_HPP
#define DISPLAY_HPP 
#include <SDL2/SDL.h>
#include <unordered_map>
#include <cinttypes>
#include <any>
#include <vector>
#include "ini_reader.hpp" 


class Display: virtual public IniReader {
    public:
        Display (std::string iniFile): IniReader(iniFile) {
            m_w = std::any_cast<int>(m_iniSettings["i_screen_width"]);
            m_h = std::any_cast<int>(m_iniSettings["i_screen_height"]);
            hex2rgb(std::any_cast<std::string>(m_iniSettings["s_colourBg"]), m_colourBg);
            hex2rgb(std::any_cast<std::string>(m_iniSettings["s_colourFg"]), m_colourFg);
            init();									// initialise SDL resources
        };
        ~Display () { 
            free(m_window);
            free(m_renderer);
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
        SDL_Window* m_window;
        SDL_Renderer* m_renderer;
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
