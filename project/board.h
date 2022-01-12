const int BOARD_SIZE = 640;
const int BOARD_XY = 80;
const int WIN_SIZE = BOARD_SIZE + 2 * BOARD_XY;
const int SQUARE_SIZE = BOARD_SIZE / 8;

const int SCREEN_COLOR[3] = { 0, 0, 0 };
const int BOARD_COLOR[3] = { 0, 51, 0 };
const int BOARD_LINE_COLOR[3] = { 230, 255, 230 };
const int CIRCLE_COLORS[6] = { 0, 0, 0, 255, 255, 255 };

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

const int DRAW_DELAY = 200;


class Board
{
	int table[8][8]; // 0 - black;  1 - white; 2 - free
    int nrDiscs[2]; // nrDiscs[0] -> black; nrDiscs[1] -> white
    string nextMoveDir;

    string user1 = ""; // the name of the first logged user
    string user2 = ""; // the name of the second logged user
	SDL_Window * window;
	SDL_Surface * screen;
	SDL_Renderer * renderer;
	SDL_Rect square = { BOARD_XY, BOARD_XY, BOARD_SIZE, BOARD_SIZE };
	bool quit = false;

	void drawTable();
	void drawDiscs();
	void fillCircle(int cx, int cy, bool color);
	
	public:

    Board(string user1, string user2);
	bool isMovePossible(int i, int j, bool color);
	bool canMove(bool color);
	void makeMove(int i, int j, bool color);
	bool isGameEnded();
	int whoWon();

	void * draw();
    static void * threadWrapper(void * ptr);
};

Board::Board(string user1, string user2)
{
	// free squares need to be initialized with 2
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            table[i][j] = 2;
    
    table[3][3] = 1; table[3][4] = 0;
    table[4][3] = 0; table[4][4] = 1;

    nrDiscs[0] = 2;
    nrDiscs[1] = 2;


    // board configuration for presentation
    // -> one move left, only black can move
    // // free squares initialized with 1
    // for (int i = 0; i < 8; i++)
    //     for (int j = 0; j < 8; j++)
    //         table[i][j] = 1;
    
    // table[0][2] = 0; table[0][3] = 0; table[2][2] = 0; table[2][5] = 0; 
    // table[3][3] = 0; table[3][4] = 0; table[4][2] = 0; table[4][4] = 0; table[4][5] = 0;
    // table[5][1] = 0; table[5][2] = 0; table[5][5] = 0; 
    // table[6][1] = 0; table[6][2] = 0; 
    // table[6][3] = 0; table[6][4] = 0; 

    // for (int i = 0; i < 7; i++)
    //     table[i][7] = 1;    

    // table[7][7] = 0;
    // table[0][7] = 2;

    // nrDiscs[0] = 17;
    // nrDiscs[1] = 46;


    this->user1 = user1;
    this->user2 = user2;

    // create thread for drawing   
    pthread_t threadId;    					// identifier of the created thread

    pthread_create(&threadId, NULL, Board::threadWrapper, this);
}

void * Board::threadWrapper(void * ptr)
{
    Board * boardPtr = static_cast<Board*> (ptr);       // cast * ptr to Board class type
    boardPtr->draw();                       // access Board's non-static member draw()
    return NULL;                        
}

void Board::drawTable()
{
	int i; 
	
	// draw a green square     
    SDL_SetRenderDrawColor( renderer, BOARD_COLOR[0], BOARD_COLOR[1], BOARD_COLOR[2], 0 );        
    SDL_RenderFillRect( renderer, &square );
    SDL_SetRenderDrawColor( renderer, BOARD_LINE_COLOR[0], BOARD_LINE_COLOR[1], 
    						BOARD_LINE_COLOR[2], 0 ); 
    SDL_RenderDrawRect( renderer, &square );
        
    // draw the lines for the col
    for( i = 0; i < BOARD_SIZE; i += SQUARE_SIZE )
    	SDL_RenderDrawLine( renderer, COL_X + i, COL_Y1, COL_X + i, COL_Y2 );
        	
    // draw the lines for rows
    for( i = 0; i < BOARD_SIZE; i += SQUARE_SIZE )
    	SDL_RenderDrawLine( renderer, ROW_X1, ROW_Y + i, ROW_X2, ROW_Y + i );
}

void Board::drawDiscs()
{
	for ( int i = 0; i < 8; i++ )
    	for ( int j = 0; j < 8; j++ )
        	if ( table[i][j] != 2 )
        		fillCircle( CIRCLE_START_XY + CIRCLE_XY * j, CIRCLE_START_XY + CIRCLE_XY * i, 								 table[i][j] );
}

//link: https://gist.github.com/derofim/912cfc9161269336f722
void Board::fillCircle(int cx, int cy, bool color)
{
	for ( double dy = 1; dy <= CIRCLE_RADIUS; dy += 1.0 )
	{
		// This loop is unrolled a bit, only iterating through half of the
		// height of the circle.  The result is used to draw a scan line and
		// its mirror image below it.

		double dx = floor( sqrt( ( 2.0 * CIRCLE_RADIUS * dy) - ( dy * dy ) ) );
		
		SDL_SetRenderDrawColor( renderer, 
					CIRCLE_COLORS[color*3], CIRCLE_COLORS[1 + color*3], CIRCLE_COLORS[2 + color*3], 0 );
		SDL_RenderDrawLine( renderer, cx - dx, cy + dy - CIRCLE_RADIUS, 
							cx + dx, cy + dy - CIRCLE_RADIUS );
		SDL_RenderDrawLine( renderer, cx - dx, cy - dy + CIRCLE_RADIUS, 
							cx + dx, cy - dy + CIRCLE_RADIUS );
	}
}

bool Board::isMovePossible(int i, int j, bool color) 
{
    nextMoveDir = "";

    // i and j out of bound
    if (i < 0 || i >= 8 || j < 0 || j >= 8)
        return false;

    //deja avem un disc acolo
    if (table[i][j] != 2)
        return false;

    int ii; //linie pentru parsarea table
    int jj; //coloana pentru parsarea table

    // test if we can capture disc if on i, j
    // right
    if (j + 2 < 8 && table[i][j + 1] == int(!color)) 
    {
        for (jj = j + 2; jj < 8 && table[i][jj] == int(!color); jj++);
        if (jj < 8 && table[i][jj] == int(color)) 
            nextMoveDir += ":r";
    }

    // left
    if (j - 2 >= 0 && table[i][j - 1] == int(!color)) 
    {
        for (jj = j - 2; jj >= 0 && table[i][jj] == int(!color); jj--);
        if (jj >= 0 && table[i][jj] == int(color)) 
            nextMoveDir += ":l";
    }

    // above            
    if (i - 2 >= 0 && table[i - 1][j] == int(!color)) 
    {
        //verificam unde se termina sirul de discuri !color
        for (ii = i - 2; ii >= 0 && table[ii][j] == int(!color); ii--);
        //daca sirul se termina cu un disc de color, putem captura discul
        if (ii >= 0 && table[ii][j] == int(color))
            nextMoveDir += ":ab";
    }

    // below             
    if (i + 2 < 8 && table[i + 1][j] == int(!color))
    {
        for (ii = i + 2; ii < 8 && table[ii][j] == int(!color); ii++);
        if (ii < 8 && table[ii][j] == int(color)) 
            nextMoveDir += ":be";
    }

    // diagonally, above right
    if (i - 2 >= 0 && j + 2 >= 0 && table[i - 1][j + 1] == int(!color)) 
    {
        for (ii = i - 2, jj = j + 2; ii >= 0 && jj < 8 && table[ii][jj] == int(!color); ii--, jj++);
        if (ii >= 0 && jj < 8 && table[ii][jj] == int(color)) 
            nextMoveDir += ":ar";
    }

    // diagonally, above left
    if (i - 2 >= 0 && j - 2 >= 0 && table[i - 1][j - 1] == int(!color))
    {
        for (ii = i - 2, jj = j - 2; ii >= 0 && jj >= 0 && table[ii][jj] == int(!color); ii--, jj--);
        if (ii >= 0 && jj >= 0 && table[ii][jj] == int(color)) 
            nextMoveDir += ":al";
    }
    // diagonally, below right
    if (i + 2 < 8 && j + 2 < 8 && table[i + 1][j + 1] == int(!color)) 
    {
        for (ii = i + 2, jj = j + 2; ii < 8 && jj < 8 && table[ii][jj] == int(!color); ii++, jj++);
        if (ii < 8 && jj < 8 && table[ii][jj] == int(color)) 
            nextMoveDir += ":br";
    }
    // diagonally, below left
    if (i + 2 < 8 && j - 2 >= 0 && table[i + 1][j - 1] == int(!color)) 
    {
        for (ii = i + 2, jj = j - 2; ii < 8 && jj >= 0 && table[ii][jj] == int(!color); ii++, jj--);
        if (ii < 8 && jj >= 0 && table[ii][jj] == int(color)) 
            nextMoveDir += ":bl";
    }
    // check if we found directions from where we can capture discs
    if (nextMoveDir != "")
        return true;
    return false;
}

bool Board::canMove(bool color) 
{
    for (int i = 0; i < 8; i++) 
    {
        for (int j = 0; j < 8; j++) 
        {

            if (table[i][j] == int(!color)
                // verify if this is a case where capturing a disc is impossible
                &&
                /*search if we can capture the disc*/
                ( // vertically
                    isMovePossible(i - 1, j, color) || isMovePossible(i + 1, j, color)
                    // horizontally
                    ||
                    isMovePossible(i, j - 1, color) || isMovePossible(i, j + 1, color)
                    // diagonally to the right
                    ||
                    isMovePossible(i - 1, j - 1, color) || isMovePossible(i + 1, j + 1, color)
                    //diagonallly to the left
                    ||
                    isMovePossible(i - 1, j + 1, color) || isMovePossible(i + 1, j - 1, color)
                )
            )
                return true; // can make a move
        }
    }
    // can't capture any disc, so no move possible
    return false;
}

void Board::makeMove(int i, int j, bool color) 
{
    int ii, jj;

    table[i][j] = color;
    nrDiscs[color]++;

    if (nextMoveDir.find(":r", 0) != string::npos) 
    {
        for (jj = j + 1; table[i][jj] == int(!color); jj++)
            table[i][jj] = color;

        nrDiscs[color] += jj - j - 1;
        nrDiscs[int(!color)] -= jj - j - 1;
    }

    if (nextMoveDir.find(":l", 0) != string::npos) 
    {
        for (jj = j - 1; table[i][jj] == int(!color); jj--)
            table[i][jj] = color;

        nrDiscs[color] += j - jj - 1;
        nrDiscs[int(!color)] -= j - jj - 1;
    }

    if (nextMoveDir.find(":ab", 0) != string::npos) 
    {
        for (ii = i - 1; table[ii][j] == int(!color); ii--)
            table[ii][j] = color;

        nrDiscs[color] += i - ii - 1;
        nrDiscs[int(!color)] -= i - ii - 1;
    }

    if (nextMoveDir.find(":be", 0) != string::npos) 
    {
        for (ii = i + 1; table[ii][j] == int(!color); ii++)
            table[ii][j] = color;

        nrDiscs[color] += ii - i - 1;
        nrDiscs[int(!color)] -= ii - i - 1;
    }

    if (nextMoveDir.find(":ar", 0) != string::npos) 
    {
        for (ii = i - 1, jj = j + 1; table[ii][jj] == int(!color); ii--, jj++)
            table[ii][jj] = color;

        nrDiscs[color] += i - ii - 1;
        nrDiscs[int(!color)] -= i - ii - 1;
    }

    if (nextMoveDir.find(":al", 0) != string::npos) 
    {
        for (ii = i - 1, jj = j - 1; table[ii][jj] == int(!color); ii--, jj--)
            table[ii][jj] = color;

        nrDiscs[color] += i - ii - 1;
        nrDiscs[int(!color)] -= i - ii - 1;
    }

    if (nextMoveDir.find(":br", 0) != string::npos) 
    {
        for (ii = i + 1, jj = j + 1; table[ii][jj] == int(!color); ii++, jj++)
            table[ii][jj] = color;

        nrDiscs[color] += ii - i - 1;
        nrDiscs[int(!color)] -= ii - i - 1;
    }

    else if (nextMoveDir.find(":bl", 0) != string::npos) 
    {
        for (ii = i + 1, jj = j - 1; table[ii][jj] == int(!color); ii++, jj--)
            table[ii][jj] = color;

        nrDiscs[color] += ii - i - 1;
        nrDiscs[int(!color)] -= ii - i - 1;
    }
}

bool Board::isGameEnded() 
{
    if (nrDiscs[0] == 0 || nrDiscs[1] == 0)
        return true;

    if (nrDiscs[0] + nrDiscs[1] == 64)
        return true;

    if (!canMove(0) && !canMove(1))
        return true;

    return false;
}

int Board::whoWon() 
{
    if (nrDiscs[0] > nrDiscs[1])
        return 0;

    if (nrDiscs[1] > nrDiscs[0])
        return 1;

    return 2;
}

void * Board::draw()
{	
    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        std::cout << "Failed to initialize the SDL2 library: " << SDL_GetError() << "\n";
        exit(-1);
    }  
    
    SDL_Event e;
	string winNames = "The game between " + user1 + " and " + user2;
	const char * winName = winNames.c_str();  

    // create the window

    window = SDL_CreateWindow( winName,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               WIN_SIZE, WIN_SIZE,
                               0 );

    if( !window )
    {
        cout << "Failed to create window: " << SDL_GetError() << "\n";
        exit(-1);
    }

    SDL_Surface * window_surface = SDL_GetWindowSurface( window );

    if( !window_surface )
    {
       	cout << "Failed to get the surface from the window: " << SDL_GetError() << "\n";
        exit(-1);
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    
	while( !quit )
    {
    	// handle events on queue
    	while( SDL_PollEvent( &e ) != 0 )
        	// user requests quit
            if( e.type == SDL_QUIT )
            	quit = true;

        // clear screen
       	SDL_SetRenderDrawColor( renderer, SCREEN_COLOR[0], SCREEN_COLOR[1], SCREEN_COLOR[2], 0 );
        SDL_RenderClear( renderer );
       	
       	// draw
		drawTable();
		drawDiscs();
        
        // update screen
        SDL_RenderPresent( renderer );
        SDL_Delay( DRAW_DELAY );        
	}

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    pthread_detach(pthread_self());

    return NULL;
}