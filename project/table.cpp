#include <SDL2/SDL.h>
#include <iostream>

using namespace std; 


const int BOARD_SIZE = 800;
const int BOARD_XY = 80;
const int WIN_SIZE = BOARD_SIZE + 2 * BOARD_XY;
const int SQUARE_SIZE = BOARD_SIZE / 8;

const int SCREEN_COLOR[3] = { 0, 0, 0 };
const int BOARD_COLOR[3] = { 0, 51, 0 };
const int BOARD_LINE_COLOR[3] = { 230, 255, 230 };
const int DISC_COLORS[6] = { 0, 0, 0, 255, 255, 255 };

const int COL_X = BOARD_XY + SQUARE_SIZE;
const int COL_Y1 = BOARD_XY;
const int COL_Y2 = WIN_SIZE - BOARD_XY;
const int ROW_X1 = BOARD_XY;
const int ROW_X2 = WIN_SIZE - BOARD_XY;
const int ROW_Y = BOARD_XY + SQUARE_SIZE;

const int CIRCLE_PADDING = 15;
const int CIRCLE_RADIUS = (SQUARE_SIZE - CIRCLE_PADDING * 2) / 2;
const int CIRCLE_START_XY = BOARD_XY + CIRCLE_PADDING + CIRCLE_RADIUS;
const int CIRCLE_XY = (CIRCLE_PADDING + CIRCLE_RADIUS) * 2;

const int DRAW_DELAY = 1000;


//link: https://gist.github.com/derofim/912cfc9161269336f722
void fill_circle(SDL_Renderer *renderer, int cx, int cy, int radius, bool color)
{
	for (double dy = 1; dy <= radius; dy += 1.0)
	{
		// This loop is unrolled a bit, only iterating through half of the
		// height of the circle.  The result is used to draw a scan line and
		// its mirror image below it.

		double dx = floor(sqrt((2.0 * radius * dy) - (dy * dy)));
		int x = cx - dx;
		SDL_SetRenderDrawColor(renderer, 
					DISC_COLORS[color*3], DISC_COLORS[1 + color*3], DISC_COLORS[2 + color*3], 0);
		SDL_RenderDrawLine(renderer, cx - dx, cy + dy - radius, cx + dx, cy + dy - radius);
		SDL_RenderDrawLine(renderer, cx - dx, cy - dy + radius, cx + dx, cy - dy + radius);
	}
}


int main( int argc, char** argv )
{
	//initialize a window
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cout << "Failed to initialize the SDL2 library: " << SDL_GetError() << "\n";
        return -1;
    }

	//create the window
    SDL_Window *window = SDL_CreateWindow( "Game between ... ",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           WIN_SIZE, WIN_SIZE,
                                           0 );

    if( !window )
    {
        cout << "Failed to create window: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface( window );

    if( !window_surface )
    {
       	cout << "Failed to get the surface from the window: " << SDL_GetError() << "\n";
        return -1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    
    bool quit = false;
    SDL_Event e;
    int i, j;
    
    //While application is running
    while( !quit )
    {
    	//Handle events on queue
    	while( SDL_PollEvent( &e ) != 0 )
        {
        	//User requests quit
            if( e.type == SDL_QUIT )
            {
            	quit = true;
            }
        }

        //Clear screen
       	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );
        SDL_RenderClear( renderer );
        
        //drawBoard
        SDL_Rect rect = { BOARD_XY, BOARD_XY, BOARD_SIZE, BOARD_SIZE };        
        SDL_SetRenderDrawColor( renderer, 0, 51, 0, 0 );        
        SDL_RenderFillRect( renderer, &rect );
        SDL_SetRenderDrawColor( renderer, 230, 255, 230, 0 ); 
        SDL_RenderDrawRect( renderer, &rect );
        
        //draw the lines for the col
        for( i = 0; i < BOARD_SIZE; i += SQUARE_SIZE )
        	SDL_RenderDrawLine( renderer, COL_X + i, COL_Y1, COL_X + i, COL_Y2 );
        	
       	//draw the lines for rows
       	for( i = 0; i < BOARD_SIZE; i += SQUARE_SIZE )
        	SDL_RenderDrawLine( renderer, ROW_X1, ROW_Y + i, ROW_X2, ROW_Y + i );
        	
        /*for ( i = 0; i < 8; i++ )
        	for (j = 0; j < 8; j++)
        		fill_circle( renderer, CIRCLE_START_XY + CIRCLE_XY * i , CIRCLE_START_XY + CIRCLE_XY * j, CIRCLE_RADIUS, table[i][j] );*/
        		
       	fill_circle( renderer, CIRCLE_START_XY, CIRCLE_START_XY, CIRCLE_RADIUS, 1 );
        fill_circle( renderer, CIRCLE_START_XY + CIRCLE_XY * 2 , CIRCLE_START_XY, CIRCLE_RADIUS, 1 );
        
        //Update screen
        SDL_RenderPresent( renderer );
        SDL_Delay( DRAW_DELAY );
        
    }
    return 0;
}
