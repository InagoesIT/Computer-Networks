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


class Graphics
{
	SDL_Window * window;
	SDL_Surface * surface;
	SDL_Renderer * renderer;
	SDL_Rect * square;
	bool quit = false;
	
	public:
	
	Graphics(string user1, string user2);
	~Graphics();
	void draw();
	void drawBoard();
	void drawDiscs();
	void fillCircle();
}

Graphics::Graphics(string user1, string user2)
{
	// initialize a window
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cout << "Failed to initialize the SDL2 library: " << SDL_GetError() << "\n";
        return -1;
    }

	// create the window
    SDL_Window *window = SDL_CreateWindow( "Game between " + user1 + " and " + user2,
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
    
    SDL_Rect square = { BOARD_XY, BOARD_XY, BOARD_SIZE, BOARD_SIZE };
}

Graphics::~Graphics
{
	delete window;
	delete surface;
	delete renderer;
	delete square;
}

void Graphics::draw()
{	
    SDL_Event e;
    
	while( !quit )
    {
    	// handle events on queue
    	while( SDL_PollEvent( &e ) != 0 )
        {
        	// user requests quit
            if( e.type == SDL_QUIT )
            {
            	quit = true;
            }
        }

        // clear screen
       	SDL_SetRenderDrawColor( renderer, SCREEN_COLOR[0], SCREEN_COLOR[1], SCREEN_COLOR[2], 0 );
        SDL_RenderClear( renderer );
       	
       	// draw
		drawBoard();
		drawDiscs();
        
        // update screen
        SDL_RenderPresent( renderer );
        SDL_Delay( DRAW_DELAY );        
	}
}


void Graphics::drawBoard()
{
	int i; 
	
	// draw a green square     
    SDL_SetRenderDrawColor( renderer, BOARD_COLOR[0], BOARD_COLOR[1], BOARD_COLOR[2], 0 );        
    SDL_RenderFillRect( renderer, &rect );
    SDL_SetRenderDrawColor( renderer, BOARD_LINE_COLOR[0], BOARD_LINE_COLOR[1], 
    						BOARD_LINE_COLOR[2], 0 ); 
    SDL_RenderDrawRect( renderer, &rect );
        
    // draw the lines for the col
    for( i = 0; i < BOARD_SIZE; i += SQUARE_SIZE )
    	SDL_RenderDrawLine( renderer, COL_X + i, COL_Y1, COL_X + i, COL_Y2 );
        	
    // draw the lines for rows
    for( i = 0; i < BOARD_SIZE; i += SQUARE_SIZE )
    	SDL_RenderDrawLine( renderer, ROW_X1, ROW_Y + i, ROW_X2, ROW_Y + i );
}


void Graphics::drawDiscs()
{
	for ( int i = 0; i < 8; i++ )
    	for ( int j = 0; j < 8; j++ )
        	if ( table[i][j] != 2 )
        		fill_circle( CIRCLE_START_XY + CIRCLE_XY * i, CIRCLE_START_XY + CIRCLE_XY * j, 								 table[i][j] );
}


//link: https://gist.github.com/derofim/912cfc9161269336f722
void Graphics::fillCircle (int cx, int cy, bool color)
{
	for ( double dy = 1; dy <= DISC_RADIUS; dy += 1.0 )
	{
		// This loop is unrolled a bit, only iterating through half of the
		// height of the circle.  The result is used to draw a scan line and
		// its mirror image below it.

		double dx = floor( sqrt( ( 2.0 * DISC_RADIUS * dy) - ( dy * dy ) ) );
		int x = cx - dx;
		
		SDL_SetRenderDrawColor( renderer, 
					DISC_COLORS[color*3], DISC_COLORS[1 + color*3], DISC_COLORS[2 + color*3], 0 );
		SDL_RenderDrawLine( renderer, cx - dx, cy + dy - DISC_RADIUS, 
							cx + dx, cy + dy - DISC_RADIUS );
		SDL_RenderDrawLine( renderer, cx - dx, cy - dy + DISC_RADIUS, 
							cx + dx, cy - dy + DISC_RADIUS );
	}
}


int main( int argc, char** argv )
{  
    return 0;
}
