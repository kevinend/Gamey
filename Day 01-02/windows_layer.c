/* NEED FOLLOWING LIBS -- kernel32.lib user32.lib gdi32.lib */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <windows.h>

#include "windows_input.h"

typedef struct {
	void *bitmap_data;
	int width;
	int height;
	int bytes_per_pixel;	
	HDC device_context;
} Frame_Buffer;

bool running = true;

Frame_Buffer frame_buffer;
BITMAPINFO bitmap_info;

RAWINPUT *device_input_message;
int input_message_size;

// NOTE(KEVIN): MOVE TO HEADER FILE
void draw();
void setup_frame_buffer( HWND );

LRESULT CALLBACK 
Win32MainCallback( HWND window,
				   UINT window_message,
				   WPARAM w_param,
				   LPARAM l_param )
{
	LRESULT result = 0;
	switch (window_message) {
		case WM_INPUT: {
			// l_param for the WM_INPUT message contains a pointer to a RAWINPUT structure 
			// need both calls to GetRawInputData -- doesn't work without the first, preallocated buffer but expects first call, allocation, second call
				GetRawInputData( (HRAWINPUT)l_param, RID_INPUT, NULL, &input_message_size, sizeof (RAWINPUTHEADER) );
				GetRawInputData( (HRAWINPUT)l_param, RID_INPUT, device_input_message, &input_message_size, sizeof (RAWINPUTHEADER) );
				process_input_data();
		} break;

		case WM_PAINT: {
			draw( window );
		} break;

		case WM_SIZE: {
			setup_frame_buffer( window );
		} break;

		case WM_QUIT:
		case WM_CLOSE: {
			DestroyWindow( window );	
			running = false;
		} break;

		default: {
			result = DefWindowProc( window, window_message, w_param, l_param );
		} break;
	}
	
	return result;
}

// If i can animate a simple scanline then I want to read input here
// and we can then adjust the scanline value and animate that way
void
render_gradient( int vscan_inc, int hscan_inc )
{
	static int vscan = 0;
	static int hscan = 0;
	int *bitmap_rgba = (int *)frame_buffer.bitmap_data;
	for ( int row = 0; row < frame_buffer.height; ++row ) {
		for ( int col = 0; col < frame_buffer.width; ++col ) {
			if ( col == vscan ) {
				*bitmap_rgba = (255 << 16) | (255 << 8) | (255 << 0);
			}
			else if ( row == hscan ) {
				*bitmap_rgba = (255 << 16) | (255 << 8) | (255 << 0);
			}
			else {
				*bitmap_rgba = (0 << 16) | (0 << 8) | (0 << 0 );
			}
			++bitmap_rgba;
		}
	}
	if ( vscan == frame_buffer.width ) {
		vscan = 0;
	}
	if ( hscan == frame_buffer.height ) {
		hscan = 0;	
	}
	vscan += vscan_inc;
	hscan += hscan_inc;
	
	return;
}

void
setup_frame_buffer( HWND window ) 
{
	if ( frame_buffer.device_context == NULL ) {
		frame_buffer.device_context = GetDC( window );
	}

	RECT client_area;
	GetClientRect( window, &client_area );

	frame_buffer.bytes_per_pixel = 4;
	frame_buffer.width = client_area.right - client_area.left;
	frame_buffer.height = client_area.bottom - client_area.top;

	// .biHeight *-1 to draw top down, from top left to bottom right
	bitmap_info.bmiHeader.biSize = sizeof bitmap_info;
	bitmap_info.bmiHeader.biWidth = frame_buffer.width;
	bitmap_info.bmiHeader.biHeight = -frame_buffer.height;
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_info.bmiHeader.biSizeImage = 0;

	if ( frame_buffer.bitmap_data ) {
		VirtualFree( frame_buffer.bitmap_data, 0, MEM_RELEASE );
	}

	int allocation_size = frame_buffer.width * frame_buffer.height * frame_buffer.bytes_per_pixel;
	frame_buffer.bitmap_data = VirtualAlloc( 0, allocation_size, MEM_COMMIT, PAGE_READWRITE );

	if ( !frame_buffer.bitmap_data ) {
		fprintf( stderr, "draw()...unable to allocate a virtual page for bitmap...shit's fucked up\n" );
		exit( EXIT_FAILURE );
	}

	return;
}

void
draw() 
{
	// here is where we would call StretchDIBits 
	StretchDIBits( frame_buffer.device_context,
  				   0, 0, frame_buffer.width, frame_buffer.height,
  				   0, 0, frame_buffer.width, frame_buffer.height,
  				   frame_buffer.bitmap_data,
  				   &bitmap_info,
  				   DIB_RGB_COLORS,
                   SRCCOPY );
	return;	
}


// Set up a window class
// Register the window class
// Start Message Loop
int CALLBACK 
WinMain( HINSTANCE instance,
		 HINSTANCE previous_instance,
		 LPSTR	   command_line_args,
		 int	   show_window_options )
{
	WNDCLASS window_class = {0}; // initializes all members to zero

	// NOTE(Kevin): If we want to add a cursor need to add the hCursor class
	window_class.style  = CS_HREDRAW | CS_OWNDC | CS_VREDRAW;
	window_class.lpfnWndProc = Win32MainCallback;
	window_class.hInstance = instance;
	window_class.hCursor = NULL;                 	
	window_class.lpszClassName = "Depth_Main_Window";

	if ( !RegisterClass( &window_class ) ) {	
		// logging -- unable to register class
		exit( EXIT_FAILURE );
	}

	HWND window = 
		CreateWindow( window_class.lpszClassName,
  					  "Depth",
  					  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
  					  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				      0, 0, instance, 0 );

 	if ( !window ) {	
 		// logging -- unable to create a window
 		exit( EXIT_FAILURE );
 	}
		
	setup_frame_buffer( window );
	setup_input_devices( window );
			
	MSG window_messages;
	while ( running ) {	
		while ( PeekMessage( &window_messages, window, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &window_messages );
			DispatchMessage( &window_messages );
		}

		draw( window );
		// once we clear all the window messages we can draw
		// sequence should be as follows: 
		//	updata
		// 	draw
		//	render
	}
	ReleaseDC( window, frame_buffer.device_context );
	free_device_input_message();
	
	return 0;	
}

