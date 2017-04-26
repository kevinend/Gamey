/* NEED FOLLOWING LIBS -- kernel32.lib user32.lib gdi32.lib */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <windows.h>

static void *bitmap_data;

// NOTE(KEVIN): MOVE TO HEADER FILE
void draw( HWND );

LRESULT CALLBACK 
Win32MainCallback( HWND window,
				   UINT window_message,
				   WPARAM w_param,
				   LPARAM l_param )
{
	LRESULT result = 0;
	switch (window_message) {

		case WM_PAINT: {
			draw( window );
		} break;

		case WM_CLOSE: {
			DestroyWindow( window );
		} break;

		default: {
			result = DefWindowProc( window, window_message, w_param, l_param );
		} break;
	}
	
	return result;

}

void
render_weird_gradient1( int window_width, int window_height )
{
	int *bitmap_rgba = (int *)bitmap_data;
	for ( int x = 0; x < window_width; ++x ) {
		for ( int y = 0; y < window_height; ++y ) {
			*bitmap_rgba = (255 << 16) | (0 << 8) | (0 << 0);
			++bitmap_rgba;
		}
	}
	return;
}

void
render_weird_gradient2( int window_width, int window_height )
{
	int *bitmap_rgba = (int *)bitmap_data;
	for ( int x = 0; x < window_width; ++x ) {
		for ( int y = 0; y < window_height; ++y ) {
			*bitmap_rgba = (0 << 16) | (255 << 8) | (0 << 0);
			++bitmap_rgba;
		}
	}
	return;
}

void
render_weird_gradient3( int window_width, int window_height )
{
	int *bitmap_rgba = (int *)bitmap_data;
	for ( int x = 0; x < window_width; ++x ) {
		for ( int y = 0; y < window_height; ++y ) {
			*bitmap_rgba = (0 << 16) | (0 << 8) | (255 << 0);
			++bitmap_rgba;
		}
	}
	return;
}

void
draw( HWND window ) 
{
	#define BYTES_PER_PIXEL 4
	
	HDC device_context;	
	RECT client_area;
	int client_area_width;
	int client_area_height;
	BITMAPINFO bitmap_info = {0};

	device_context = GetDC( window );

	GetClientRect( window, &client_area );
	client_area_width = client_area.right - client_area.left;
	client_area_height = client_area.bottom - client_area.top;
	
	// .biHeight *-1 to draw top down, from top left to bottom right
	bitmap_info.bmiHeader.biSize = sizeof bitmap_info;
	bitmap_info.bmiHeader.biWidth = client_area_width;
	bitmap_info.bmiHeader.biHeight = -client_area_height; 
	bitmap_info.bmiHeader.biPlanes = 1;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_info.bmiHeader.biSizeImage = 0;

	if ( bitmap_data ) {
		VirtualFree( bitmap_data, 0, MEM_RELEASE );
	}

	int allocation_size = client_area_height * client_area_width * BYTES_PER_PIXEL;
	bitmap_data = VirtualAlloc( 0, allocation_size, MEM_COMMIT, PAGE_READWRITE );

	if ( !bitmap_data ) {
		fprintf( stderr, "draw()...unable to allocate a virtual page for bitmap...shit's fucked up\n" );
		exit( EXIT_FAILURE );
	}

	// now we can put the rendering function in here 
	static int flag = 1;

	switch (flag) {
		case 1:  {
			render_weird_gradient1( client_area_width, client_area_height );	
			Sleep( 2000 );
			flag = 2;
		} break;

		case 2: {
			render_weird_gradient2( client_area_width, client_area_height );
			Sleep( 2000 );
			flag = 3;
		} break;

		case 3: {
			render_weird_gradient3( client_area_width, client_area_height );
			Sleep( 2000 );
			flag = 1;
		} break;
	}

	// here is where we would call StretchDIBits 
	StretchDIBits( device_context,
  				   client_area.left, client_area.top,
  				   client_area_width, client_area_height,
  				   client_area.left, client_area.top,
  				   client_area_width, client_area_height,
  				   bitmap_data,
  				   &bitmap_info,
  				   DIB_RGB_COLORS,
                   SRCCOPY );
	
	ReleaseDC( window, device_context );

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

	if ( RegisterClass( &window_class ) ) {
		
		HWND window = 
			CreateWindow( window_class.lpszClassName,
  						  "Depth",
  						  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
  						  CW_USEDEFAULT,
  						  CW_USEDEFAULT,
  						  CW_USEDEFAULT,
  						  CW_USEDEFAULT,
  						  0,
  						  0,
  						  instance,
 						  0 );

 		if ( window ) {	
			
			bool keep_running = true;
			MSG window_messages;
			
			while ( keep_running ) {
		
				while ( PeekMessage( &window_messages, window, 0, 0, PM_REMOVE ) ) {
					if ( window_messages.message == WM_QUIT ) {
						// NOTE(kevin): Doesn't free memory associated with backbuffer, handled by the OS when the program exits
						keep_running = false;
					}
					else {
						TranslateMessage( &window_messages );
						DispatchMessage( &window_messages );
					}
				}
				draw( window );
				// once we clear all the window_messages we can draw
				// update
				// draw
				// render 
			}

 		}
 		else {
			// logging here, need to put this somewhere
			// unable to open a window 
 		}
	}
	else {
		// logging here, need to put this somewhere
		// unable to register the window class 
	}

	return 0;	
}
