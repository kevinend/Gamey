#include "windows_input.h"
#include <stdint.h>

// Currently only setup for one device -- keyboard
void 
setup_input_devices( HWND window ) 
{
	// keyboard -- 32, mouse -- 40
	#define MAX_DEVICE_MESSAGE_SIZE 40

	// 0x01 and 0x06 together specify the keyboard
	RAWINPUTDEVICE devices[1];
	devices[0].usUsagePage = 0x01;
	devices[0].usUsage     = 0x06;
	devices[0].dwFlags     = 0;
	devices[0].hwndTarget  = window;

	if ( RegisterRawInputDevices( devices, 1, sizeof devices ) == FALSE ) {
		// log this error -- using format message 
	}

	device_input_message = malloc( MAX_DEVICE_MESSAGE_SIZE );
	if ( !device_input_message ) {
		exit( EXIT_FAILURE );
	}

	return;
}

void
process_input_data() 
{
	if ( device_input_message->header.dwType == RIM_TYPEKEYBOARD ) {
		process_keyboard_commands();
	}
	
	if ( device_input_message->header.dwType == RIM_TYPEMOUSE ) {
		process_mouse_commands();
	}
}

void
process_mouse_commands()
{
	return;
}

void 
process_keyboard_commands() {
	
	int key_code;
	key_code = device_input_message->data.keyboard.VKey;

	switch ( key_code ) {
		case VK_LEFT: {
			OutputDebugString( "Moving left\n" );
			render_gradient( -10, 0 ); 
		} break;

		case VK_RIGHT: {
			OutputDebugString( "Moving right\n" );
			render_gradient( 10, 0 );
		} break;

		case VK_UP: {
			render_gradient( 0, -10 );
		} break;

		case VK_DOWN: {
			render_gradient( 0, 10 );
		} break;

		default: {

		} break;
	}
}

void free_device_input_message() 
{
	free( device_input_message );
}

