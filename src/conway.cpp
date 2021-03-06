#include <iostream>
#include <SDL2/SDL.h>

using namespace std;

const int GRID_SIZE        = 21; /* height and width of the grid in cells     */

const int LIVE_CELL        = 1;  /* numerical value for a living cell         */
const int DEAD_CELL        = 0;  /* numerical value for a dead cell           */

const int REPRODUCE_NUM    = 3; /* more than this and cell dies of starvation */
const int OVERPOPULATE_NUM = 3; /* exactly this and cells reproduce           */
const int ISOLATION_NUM    = 2; /* less than this and cell dies of loneliness */

const int ANIMATION_RATE   = 250; /* update animation every 250 milliseconds  */
const int SCREEN_SIZE      = 800; /* height and width of screen in pixels     */
const int CELL_SIZE        = SCREEN_SIZE/GRID_SIZE; /* size of cell in pixels */

void init_grid               ( int grid[][GRID_SIZE], int size );
int  count_living_neighbours ( int grid[][GRID_SIZE], int size, int x, int y );
void update_cell             ( int grid[][GRID_SIZE], int size, int x, int y, 
                                    int num_neighbours );
void set_cell                ( int grid[][GRID_SIZE], int size, int x, int y, 
                                    int val );
void step                    ( int grid[][GRID_SIZE], int size );

bool initialize_display      ( );
void display_grid            ( int grid[][GRID_SIZE], int size );
void handle_events           ( int grid[][GRID_SIZE], int size );
void terminate_display       ( );

bool g_animating = false; /* controls whether or not simulation is animating */
bool g_user_quit = false; /* tells us when the user has quit the game */

SDL_Renderer *g_renderer = NULL; /* for drawing to the screen */
SDL_Window   *g_window   = NULL; /* the window given to us by the OS */

int main() {        
    Uint32 ticks;
    
    /* the grid on which the game is played */
    int grid[GRID_SIZE][GRID_SIZE];

    /* try to create a window and renderer. Kill the program if we
     * fail */
    if(!initialize_display()) {
        return 1;
    }
    
    /* sets every cell in the grid to be dead */
    init_grid( grid, GRID_SIZE );


    /* keep track of elapsed time so we can render the animation at a
     * sensible framerate */
    ticks = SDL_GetTicks();
    
    /* step the simulation forward until the user decides to quit */
    while(!g_user_quit) {
        /* button presses, mouse movement, etc */
        handle_events( grid, GRID_SIZE );

        /* draw the game to the screen */
        display_grid( grid, GRID_SIZE );

        /* advance the game if appropriate */
        if(g_animating && (SDL_GetTicks() - ticks) > ANIMATION_RATE ) {
            step( grid, GRID_SIZE );
            ticks = SDL_GetTicks();
        }
    }

    /* clean up when we're done */
    terminate_display();
    
    return 0;
}

/* Initializes the grid so that all cells are dead */
void init_grid( int grid[][GRID_SIZE], int size ) {
    int x,y;

    /* iterate over whole grid and initialize */
    for( y=0; y<size; y++ ) {
        for( x=0; x<size; x++ ) {
            grid[y][x] = DEAD_CELL;
        }
    }
}

/* counts the number of living neighbours around cell (x,y) */
int count_living_neighbours( int grid[][GRID_SIZE], int size, int x, int y ) {
    int i,j;
    int count;

    /* count the number of living neighbours */
    count = 0;
    for(i=y-1; i<=y+1; i++) {
        for(j=x-1; j<=x+1; j++) {
            /* make sure we don't go out of bounds */
            if( i >= 0 && j >= 0 && i < size && j < size) {
                /* if cell is alive, then add to the count */
                if( grid[i][j] == LIVE_CELL ) {
                    count++;
                }
            }
        }
    }

    /* our loop counts the cell at the center of the */
    /* neighbourhood. Remove that from the count     */
    if( grid[y][x] != DEAD_CELL ) {
        count--;
    }
    
    return count;
}

/* update a cell's state to living or dead depending on the number of */
/* neighbours it has                                                  */
void update_cell(int grid[][GRID_SIZE], int size, int x, int y, 
                                                        int num_neighbours) {
    if( num_neighbours == REPRODUCE_NUM ) {
         /* come to life due to reproduction */
        grid[y][x] = LIVE_CELL;
    } else if (num_neighbours > OVERPOPULATE_NUM 
                                        || num_neighbours < ISOLATION_NUM ) {
        /* die due to overpopulation/isolation */
        grid[y][x] = DEAD_CELL;
    }   
}

/* set the grid cell at coordinates (x,y) to the value passed as */
/* argument                                                      */
void set_cell( int grid[][GRID_SIZE], int size, int x, int y, int val ) {
    /* Ensure that we are within the bounds of the grid before trying to */
    /* access the cell                                                   */
    if ( x >= 0 && x < size && y>=0 && y< size ) {
        /* if x and y are valid, update the cell value */
        grid[y][x] = val;
    }
}

/* advance the game of life simulation by one step */
void step( int grid[][GRID_SIZE], int size ) {
    int x, y;
    int counts[GRID_SIZE][GRID_SIZE];

    /* advancing needs to be performed in two steps: count the neighbours for */
    /* every cell, then update the state of every cell. So, two for loops and */
    /* one extra 2D array                                                     */

    /* count the neighbours for each cell and store the count */
    for(y=0; y<size; y++) {
        for(x=0; x<size; x++) {
            counts[y][x] = count_living_neighbours(grid, size, x,y);
        }
    }

    /* update cell to living or dead depending on number of neighbours */
    for(y=0; y<GRID_SIZE; y++) {
        for(x=0; x<GRID_SIZE; x++) {
            update_cell(grid, size, x, y, counts[y][x]);            
        }
    }   
}

bool initialize_display () {
    /* initialize the library that does all the heavy lifting for
     * graphics */
    if( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "initialize_display - SDL_Init: %s\n", 
            SDL_GetError()
        );
        return false;
    }

    /* Try to create a window (so we can see the game) and a renderer  
     * (so we can draw to the window )                                 */
    if( SDL_CreateWindowAndRenderer( SCREEN_SIZE, SCREEN_SIZE, 0, 
                                        &g_window, &g_renderer ) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "initialize_display - SDL_CreateWindowAndRenderer - %s\n", 
            SDL_GetError()
        );
        return false;
    }

    /* all went well! Return true */
    return true;
}

void display_grid ( int grid[][GRID_SIZE], int size ) {
    
    /* Set draw colour to white */
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    
    /* clear the screen */
    SDL_RenderClear(g_renderer);

    /* Set draw colour to black */
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    /* Draw row lines */
    for(int i=0; i<size; i++) {
        SDL_RenderDrawLine(g_renderer, 
            
                0, CELL_SIZE*i,          /* x1, y1 */
                SCREEN_SIZE, CELL_SIZE*i /* x2, y2 */
        );
    }

    /* Draw column lines */
    for(int i=0; i<size; i++) {            
        SDL_RenderDrawLine(g_renderer,                 
                CELL_SIZE*i, 0,           /* x1, y1 */                
                CELL_SIZE*i, SCREEN_SIZE  /* x2, y2 */
        );
    }

    /* Set draw colour to blue */
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);

    /* Render the game of life */
    for( int x=0; x<size; x++ ) {
        for( int y=0; y<size; y++) {
            if(grid[y][x] == LIVE_CELL ) {
                SDL_Rect r = {
                    x*CELL_SIZE, /*   x    */
                    y*CELL_SIZE, /*   y    */
                    CELL_SIZE,   /* width  */
                    CELL_SIZE    /* height */
                };
                SDL_RenderFillRect(g_renderer, &r);
            }
        }
    }

    /* Update the display so player can see */
    SDL_RenderPresent(g_renderer);
}

void handle_events( int grid[][GRID_SIZE], int size ) {
    SDL_Event event;
    
    /* get all events from the event queue */
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT){
            /* user has quit the game */
            g_user_quit = true;
        }
        else if(event.type == SDL_MOUSEMOTION) {
            /* bring cells to life or kill them on mouse drag */
            if ( event.motion.state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK) ) {
                set_cell( 
                    grid,  
                    size, 
                    event.motion.x/CELL_SIZE, /* mouse x to grid index */
                    event.motion.y/CELL_SIZE, /* mouse y to grid index */
                    (event.motion.state & SDL_BUTTON_LMASK)? /* which button? */
                                            LIVE_CELL : DEAD_CELL); 
            }
        }
        else if( event.type == SDL_MOUSEBUTTONDOWN ) {
            /* bring a cell to life or kill it on a mouse click */
            set_cell( grid, 
                size, 
                event.button.x/CELL_SIZE, /* mouse x to grid index */
                event.button.y/CELL_SIZE, /* mouse y to grid index */
                (event.button.button==SDL_BUTTON_LEFT)? /* which button? */
                                        LIVE_CELL : DEAD_CELL);
        }
        else if( event.type == SDL_KEYDOWN ) {
            if (event.key.keysym.sym == SDLK_SPACE) {
                /* start/stop animation with space */
                g_animating = !g_animating;
            } else if (event.key.keysym.sym == SDLK_c ) {
                 /* clear the screen with c. Also stop animating */
                init_grid( grid, size );
                g_animating = false;
            }
        }
    }
}

void terminate_display() {
    /* Always release resources when you're done */
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}
