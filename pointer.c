#include <stdlib.h>
#include <stdio.h>

#include <bluetooth/bluetooth.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <cwiid.h>

// Constants
#define S_WIDTH  800
#define S_HEIGHT 600
#define S_BPP    32
#define S_FLAGS  SDL_HWSURFACE | SDL_DOUBLEBUF
#define X_MAX    1000
#define Y_MAX    750

// Macros
#define toggle_bit(bf,b) \
  (bf) = ((bf) & b) ? ((bf) & ~(b)) : ((bf) | (b))

// Structures definition
typedef enum AppState AppState;
enum AppState
  {
    PAIRING, DISPLAY, EXIT
  };

// Prototypes


int main( int argc, char** argv )
{
  // Variables definition
  int wiimote_paired = 0;
  int x = -1;
  int y = -1;
  int batt = 0;
  char big_msg[256];
  char small_msg[256];
  unsigned char rumble = 0;
  unsigned char rpt_mode = 0;
  SDL_Surface* screen = NULL;
  SDL_Event event;
  SDL_Rect big_position, small_position;
  SDL_Surface* big_text = NULL;
  SDL_Surface* small_text = NULL;
  TTF_Font* big_font = NULL;
  TTF_Font* small_font = NULL;
  AppState appstate = PAIRING;
  cwiid_wiimote_t* wiimote;
  bdaddr_t bdaddr = *BDADDR_ANY;
  struct cwiid_state state;
  SDL_Color white = {0,0,0};
  SDL_Rect dot;

  // Variables initialisation
  big_position.x = 180;
  big_position.y = 250;
  small_position.x = 300;
  small_position.y = 50;

  // SDL and SDL_TTF initialization
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) )
    {
      fprintf( stderr, "Error while initializing SDL: %s\n", SDL_GetError() );
      exit( EXIT_FAILURE );
    }
  if( TTF_Init() )
    {
     fprintf( stderr, "Error while initializing SDL_TTF: %s\n", TTF_GetError() );
     exit( EXIT_FAILURE );
    }

  // Resources loading
  big_font = TTF_OpenFont( "gratis.ttf", 35 );
  if( big_font == NULL )
    {
      fprintf( stderr, "Error while loading font: %s\n", TTF_GetError() );
      exit( EXIT_FAILURE );
    }
  small_font = TTF_OpenFont( "gratis.ttf", 15 );
  if( small_font == NULL )
    {
      fprintf( stderr, "Error while loading font: %s\n", TTF_GetError() );
      exit( EXIT_FAILURE );
    }

  // Main window creation
  screen = SDL_SetVideoMode( S_WIDTH, S_HEIGHT, S_BPP, S_FLAGS );
  SDL_WM_SetCaption( "Wiimote pointer display", NULL );

  // Pair with the wiimote and rumble to notify
  printf( "Trying to pair with the wiimote now.\nPut the wiimote in search mode (press 1+2) ...\n" );
  if( wiimote = cwiid_open( &bdaddr, 0 ) )
    {
      wiimote_paired = 1;
      appstate = DISPLAY;
      toggle_bit( rumble, 0x01 );
      cwiid_set_rumble( wiimote, rumble );
      usleep( 300000 );
      toggle_bit( rumble, 0x01 );
      cwiid_set_rumble( wiimote, rumble );
      // toggle IR reporting ON
      toggle_bit( rpt_mode, 0x08 );
      cwiid_set_rpt_mode( wiimote, rpt_mode );
    }

  // Main loop
  while( appstate != EXIT )
    {
      // Take care of events if there is any
      if( SDL_PollEvent( &event ) )
	{
	  switch( event.type )
	    {
	    case SDL_QUIT:
	      appstate = EXIT;
	      break;
	    
	    case SDL_KEYDOWN:
	      if( event.key.keysym.sym == SDLK_ESCAPE )
		appstate = EXIT;
	      break;
		 
	    }
	}

      // Query the wiimote
      if( appstate == DISPLAY )
	{
	  cwiid_get_state( wiimote, &state );
	  if( state.ir_src[0].valid )
	    {
	      SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, 0, 255, 0 ) );
	      x = state.ir_src[0].pos[0];
	      y = state.ir_src[0].pos[1];
	    }
	  else
	    {
	      SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, 255, 0, 0 ) );
	    }
	  batt = (int)(100.0 * state.battery / 0xD0);
	  //	  sprintf( big_msg, "IR source position: (%d,%d)", x, y );
	  dot.x = (int) (x * S_WIDTH) / X_MAX;
	  dot.y = (int) (y * S_HEIGHT) / Y_MAX;
	  dot.w = 10;
	  dot.h = 10;
	  SDL_FillRect( screen, &dot, SDL_MapRGB( screen->format, 255, 255, 255) );
	  sprintf( small_msg, "Battery remaining: %d%%", batt );
	  //big_text = TTF_RenderText_Solid( big_font, big_msg, white );
	  small_text = TTF_RenderText_Solid( small_font, small_msg, white );
	  //SDL_BlitSurface( big_text, NULL, screen, &big_position );
	  SDL_BlitSurface( small_text, NULL, screen, &small_position );
	}

      // Render the screen
      SDL_Flip( screen );
    }

  // Free resources and exits sub-systems
  TTF_CloseFont( big_font );
  TTF_CloseFont( small_font );
  TTF_Quit();
  if( wiimote_paired )
    cwiid_close( wiimote );
  else
    fprintf( stderr, "No wiimotes could be found\n" );
  SDL_Quit();
  
  return EXIT_SUCCESS;
}
