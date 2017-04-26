#ifndef _WIN32_INPUT_H_
#define _WIN32_INPUT_H_

#include "windows.h"

// device message sizes keyboard 32, mouse 40
extern RAWINPUT *device_input_message;
extern int input_message_size;

// Currently only setup for one device -- keyboard
void setup_input_devices( HWND );

void process_input_data();

void process_mouse_commands();

void process_keyboard_commands();

void free_device_input_message();

#endif 
