#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <cwiid.h>

#define toggle_bit(bf,b) \
  (bf) = ((bf) & b) ? ((bf) & ~(b)) : ((bf) | (b))

cwiid_mesg_callback_t cwiid_callback ;

void display_menu() ;
void get_wiimote_id( cwiid_wiimote_t* );
void set_led_state( cwiid_wiimote_t *, unsigned char );
void set_rpt_mode( cwiid_wiimote_t*, unsigned char );
void print_state( struct cwiid_state*);
void make_it_beep( cwiid_wiimote_t* );

int main( int argc, char** argv )
{
  cwiid_wiimote_t *wiimote; // wiimote handle
  bdaddr_t bdaddr = *BDADDR_ANY ; // bluetooth address
  int exit = 0;
  unsigned char led_state = 0;
  unsigned char rumble = 0;
  unsigned char rpt_mode = 0;
  struct cwiid_state state;
  FILE* h = NULL ;


  printf( "Trying to connect now.\nPut the Wiimote in search mode (press 1+2) ...\n" );


  // Connecting to the wiimote
  if( !( wiimote = cwiid_open(&bdaddr, 0) ) )
    {
      printf( "Could not connect to a wiimote...\n" );
      return -1 ;
    }

  while( !exit )
    {
      display_menu() ;

      switch( getchar() )
	{
	case '1':
	  toggle_bit( led_state, 0x01 );
	  set_led_state( wiimote, led_state );
	  break;

	case '2':
	  toggle_bit( led_state, 0x02 );
	  set_led_state( wiimote, led_state );
	  break;

	case '3':
	  toggle_bit( led_state, 0x04 );
	  set_led_state( wiimote, led_state );
	  break;

	case '4':
	  toggle_bit( led_state, 0x08 );
	  set_led_state( wiimote, led_state );
	  break;

	case '5':
	  toggle_bit( rumble, 1 );
	  if( cwiid_set_rumble( wiimote, rumble ) )
	    printf( "Error setting rumble...\n" );
	  break;

	case 'a':
	  toggle_bit( rpt_mode, 0x04 );
	  set_rpt_mode( wiimote, rpt_mode );
	  break;

	case 'A':
	  while (1)
	    {
	      cwiid_get_state( wiimote, &state );
	      fprintf(stdout, "\nX : %d\nY : %d\nZ : %d\n", state.acc[0], state.acc[1], state.acc[2] );
	      fputs( "\033[A\033[2K\033[A\033[2K", stdout );
	      rewind( stdout );
	      ftruncate(1,0);
	    }
	  break;

	case 'b':
	  toggle_bit( rpt_mode, 0x02 );
	  set_rpt_mode( wiimote, rpt_mode );
	  break;

	case 'd':
	  get_wiimote_id( wiimote );
	  break;

	case 'i':
	  toggle_bit( rpt_mode, 0x08 );
	  set_rpt_mode( wiimote, rpt_mode );
	  break;
	  
	case 's':
	  if( cwiid_get_state( wiimote, &state ) )
	    printf( "Error while getting state...\n" );

	  print_state( &state );
	  break;

	case 'w':
	  make_it_beep( wiimote );
	  break;

	case 'x':
	  exit = 1 ;
	  break;
	  
	case '\n':
	  break;

	default:
	  printf( "Invalid option...\n\n" );
	}
    }

  if( cwiid_close( wiimote ) )
    printf( "Error while disconnecting the wiimote\n" );
  else
    printf( "Wiimote disconnected sucessfully.\nProgram terminated.\n\n" );
}


void display_menu()
{
  printf( "=============\n");
  printf( "Choose an option between the following ones:\n\n" );
  printf( "1.\tToggle LED 1\n" );
  printf( "2.\tToggle LED 2\n" );
  printf( "3.\tToggle LED 3\n" );
  printf( "4.\tToggle LED 4\n" );
  printf( "5.\tToggle rumble\n" );
  printf( "a.\tToggle Acc reporting\n" );
  printf( "A.\tGet acc outputs continuously\n" );
  printf( "b.\tToggle Button reporting\n" );
  printf( "i.\tToggle IR reporting\n" );
  printf( "d.\tGet Wiimote ID\n" );
  printf( "s.\tPrint current state\n" );
  printf( "x.\tExit program\n" );
  printf( "w.\tMake it beep\n" );

  printf( "=============\n\n");
}

void get_wiimote_id( cwiid_wiimote_t *wiimote )
{
  int id = cwiid_get_id( wiimote );

  printf( "Wiimote ID is: %d\n", id );
}

void set_led_state( cwiid_wiimote_t *wiimote, unsigned char led_state )
{
  if( cwiid_set_led( wiimote, led_state ) )
    {
      printf( "Error setting LEDs ...\n" );
    }
}

void set_rpt_mode( cwiid_wiimote_t* wiimote, unsigned char rpt_mode )
{
  if( cwiid_set_rpt_mode( wiimote, rpt_mode ) )
    {
      printf( "Error setting RPT mode...\n" );
    }
}

void print_state( struct cwiid_state *state )
{
  int i;
  int valid_source = 0;

  printf( "*****************************************\n" );
  printf( "\t\tSTATE DESCRIPTION\n" );
  printf( "*****************************************\n" );
  printf( "Report mode:\t" );
  
  if (state->rpt_mode & 0x08) printf("IR");
  if (state->rpt_mode & 0x02) printf("Button");

  printf( "\n\n" );

  printf( "Battery: %d%%\n", (int)(100.0 * state->battery / 0xD0) );

  printf( "\n\n" );

  printf("Accelerometer: x=%d y=%d z=%d\n", state->acc[0], state->acc[1], state->acc[2] );

  printf( "IR\n" );
  for( i = 0; i <4; i++)
    {
      if( state->ir_src[i].valid )
	{
	  valid_source = 1;
	  printf( "(%d,%d) ", state->ir_src[i].pos[0], state->ir_src[i].pos[1] );
	}
    }
  if( !valid_source )
    printf( "No valid sources detected\n" );
  printf( "\n" );
}

void make_it_beep( cwiid_wiimote_t* wiimote )
{
  if( cwiid_beep( wiimote ) )
    printf( "Problem while trying ot make it beep...\n" );
}
