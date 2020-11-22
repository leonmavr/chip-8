#include <SDL2/SDL.h>
#include <memory>


class Display {
public:
	Display (unsigned w = 320, unsigned h = 640) {
		m_w = w;
		m_h = h;
		init();	
	};
	~Display () { };
	void drawBlock(unsigned x, unsigned y);
	//void reset();
	void close();

protected:
	unsigned m_w;
	unsigned m_h;
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;

	void init();
};


void Display::init() {
	// see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
	SDL_Init(SDL_INIT_EVERYTHING);
	m_window = SDL_CreateWindow("Display", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, m_w, m_h, SDL_WINDOW_SHOWN);
	m_renderer = SDL_CreateRenderer(m_window, -1, 0);

	// r, g, b, a
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    //Clear the renderer with the draw color
    SDL_RenderClear(m_renderer);
    //Update the renderer which will show the renderer cleared by the draw color
    SDL_RenderPresent(m_renderer);
    //Pause for 100 ms
    SDL_Delay(100);
}


void Display::close() {
	// see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
    //Destroy the class's renderer 
    SDL_DestroyRenderer(m_renderer);
    //Destroy the class's window
    SDL_DestroyWindow(m_window);
    //Close all the systems of SDL initialized at the top
    SDL_Quit();
}


void Display::drawBlock(unsigned x, unsigned y) {
	// define the block to draw ("pixel")
	SDL_Rect block;
	unsigned scaleX = static_cast<int>(m_w / 32);
	unsigned scaleY = static_cast<int>(m_h / 64);
	block.x = x * scaleX;
	block.y = y * scaleY;
	// The original Chip-8 display was 32x64
	block.w = scaleX;
	block.h = scaleY;

	SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255); // the rect color (solid red)
	SDL_RenderFillRect(m_renderer, &block);
	SDL_RenderPresent(m_renderer); // copy to screen

    SDL_Delay(500);
}
