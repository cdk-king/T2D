// DEMO6_3.CPP basic full-screen pixel plotting DirectDraw demo


//和在工程设置里写上链入wpcap.lib的效果一样（两种方式等价，或说一个隐式一个显式调用）
#pragma comment(lib,"ddraw.lib") 

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  // just say no to MFC

#define INITGUID // make sure directX guids are included

#include <windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <iostream> // include important C/C++ stuff
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h> 
#include <math.h>
#include <io.h>
#include <fcntl.h>

#include <ddraw.h> // include directdraw

// DEFINES ////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define SCREEN_BPP      32    // bits per pixel
#define MAX_COLORS      256  // maximum colors

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// initializes a direct draw struct
#define DD_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// GLOBALS ////////////////////////////////////////////////
HWND      main_window_handle = NULL; // globally track main window
HINSTANCE hinstance_app = NULL; // globally track hinstance

// directdraw stuff

LPDIRECTDRAW7         lpdd = NULL;   // dd object
LPDIRECTDRAWSURFACE7  lpddsprimary = NULL;   // dd primary surface主面
LPDIRECTDRAWSURFACE7  lpddsback = NULL;   // dd back surface幅面
LPDIRECTDRAWPALETTE   lpddpal = NULL;   // a pointer to the created dd palette调色板
LPDIRECTDRAWCLIPPER   lpddclipper = NULL;   // dd clipper剪切
PALETTEENTRY          palette[256];          // color palette
PALETTEENTRY          save_palette[256];     // used to save palettes
DDSURFACEDESC2        ddsd;                  // a direct draw surface description struct
DDBLTFX               ddbltfx;               // used to fill
DDSCAPS2              ddscaps;               // a direct draw surface capabilities struct
HRESULT               ddrval;                // result back from dd calls
DWORD                 start_clock_count = 0; // used for timing

// these defined the general clipping rectangle
int min_clip_x = 0,                          // clipping rectangle 
max_clip_x = SCREEN_WIDTH - 1,
min_clip_y = 0,
max_clip_y = SCREEN_HEIGHT - 1;

// these are overwritten globally by DD_Init()
int screen_width = SCREEN_WIDTH,            // width of screen
screen_height = SCREEN_HEIGHT,           // height of screen
screen_bpp = SCREEN_BPP;              // bits per pixel


char buffer[80];                     // general printing buffer

// FUNCTIONS //////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	// this is the main message handler of the system
	PAINTSTRUCT		ps;		// used in WM_PAINT
	HDC				hdc;	// handle to a device context
	char buffer[80];        // used to print strings

	// what is the message 
	switch (msg)
	{
	case WM_CREATE:
	{
		// do initialization stuff here
		// return success
		return(0);
	} break;

	case WM_PAINT:
	{
		// simply validate the window 
		hdc = BeginPaint(hwnd, &ps);

		// end painting
		EndPaint(hwnd, &ps);

		// return success
		return(0);
	} break;

	case WM_DESTROY:
	{

		// kill the application, this sends a WM_QUIT message 
		PostQuitMessage(0);

		// return success
		return(0);
	} break;

	default:break;

	} // end switch

// process any messages that we didn't take care of 
	return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

///////////////////////////////////////////////////////////

int Game_Main(void *parms = NULL, int num_parms = 0)
{
	// this is the main loop of the game, do all your processing
	// here

	// for now test if user is hitting ESC and send WM_CLOSE
	if (KEYDOWN(VK_ESCAPE))
		SendMessage(main_window_handle, WM_CLOSE, 0, 0);


	// plot 1000 random pixels to the primary surface and return
	// clear ddsd and set size, never assume it's clean
	//内存空间初始化
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	if (FAILED(lpddsprimary->Lock(NULL, &ddsd,
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
		NULL)))
	{
		// error
		return(0);
	} // end if

 // now ddsd.lPitch is valid and so is ddsd.lpSurface

 // make a couple aliases to make code cleaner, so we don't
 // have to cast
	int mempitch = (int)ddsd.lPitch;
	UCHAR *video_buffer = (UCHAR *)ddsd.lpSurface;

	// plot 1000 random pixels with random colors on the
	// primary surface, they will be instantly visible
	for (int index = 0; index < 1000; index++)
	{
		// select random position and color for 640x480x8
		UCHAR color1 = rand() % 255;
		UCHAR color2 = rand() % 255;
		UCHAR color3 = rand() % 255;
		int x = rand() % 640;
		int y = rand() % 480;

		// plot the pixel
		video_buffer[x*4 + y * mempitch] = color1;
		video_buffer[x*4 + y * mempitch + 1] = color2;
		video_buffer[x*4 + y * mempitch + 2] = color3;
		video_buffer[x*4 + y * mempitch + 3] = 128;

	} // end for index

// now unlock the primary surface
	if (FAILED(lpddsprimary->Unlock(NULL)))
		return(0);

	// sleep a bit
	Sleep(30);

	// return success or failure or your own return code here
	return(1);

} // end Game_Main

////////////////////////////////////////////////////////////

int Game_Init(void *parms = NULL, int num_parms = 0)
{
	// this is called once after the initial window is created and
	// before the main event loop is entered, do all your initialization
	// here

	// create IDirectDraw interface 7.0 object and test for error
	if (FAILED(DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL)))
		return(0);

	// set cooperation to full screen
	if (FAILED(lpdd->SetCooperativeLevel(main_window_handle,
		DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX |
		DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)))
	{
		// error
		return(0);
	} // end if

 // set display mode to 640x480x8
	if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, 0, 0)))
	{
		// error
		return(0);
	} // end if


 // clear ddsd and set size
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	// enable valid fields
	ddsd.dwFlags = DDSD_CAPS;

	// request primary surface主显示表面
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// create the primary surface
	if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
	{
		// error
		return(0);
	} // end if

 // build up the palette data array
	for (int color = 1; color < 255; color++)
	{
		// fill with random RGB values
		palette[color].peRed = rand() % 256;
		palette[color].peGreen = rand() % 256;
		palette[color].peBlue = rand() % 256;

		// set flags field to PC_NOCOLLAPSE
		palette[color].peFlags = PC_NOCOLLAPSE;
	} // end for color

// now fill in entry 0 and 255 with black and white
	palette[0].peRed = 0;
	palette[0].peGreen = 0;
	palette[0].peBlue = 0;
	palette[0].peFlags = PC_NOCOLLAPSE;

	palette[255].peRed = 255;
	palette[255].peGreen = 255;
	palette[255].peBlue = 255;
	palette[255].peFlags = PC_NOCOLLAPSE;

	// create the palette object创建调色板
	if (FAILED(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256 |
		DDPCAPS_INITIALIZE,
		palette, &lpddpal, NULL)))
	{
		// error
		return(0);
	} // end if

	// finally attach the palette to the primary surface主面设置调色板
	if (FAILED(lpddsprimary->SetPalette(lpddpal)))
	{
		// error
		return(0);
	} // end if

// return success or failure or your own return code here
	return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
	// this is called after the game is exited and the main event
	// loop while is exited, do all you cleanup and shutdown here

	// first the palette
	if (lpddpal)
	{
		lpddpal->Release();
		lpddpal = NULL;
	} // end if

 // now the primary surface
	if (lpddsprimary)
	{
		lpddsprimary->Release();
		lpddsprimary = NULL;
	} // end if

 // now blow away the IDirectDraw4 interface
	if (lpdd)
	{
		lpdd->Release();
		lpdd = NULL;
	} // end if

 // return success or failure or your own return code here
	return(1);

} // end Game_Shutdown

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE hprevinstance,
	LPSTR lpcmdline,
	int ncmdshow)
{

	WNDCLASSEX winclass; // this will hold the class we create
	HWND	   hwnd;	 // generic window handle
	MSG		   msg;		 // generic message
	HDC        hdc;      // graphics device context

	// first fill in the window class stucture
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC |
		CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// save hinstance in global
	hinstance_app = hinstance;

	// register the window class
	if (!RegisterClassEx(&winclass))
		return(0);

	// create the window
	if (!(hwnd = CreateWindowEx(NULL,                  // extended style
		WINDOW_CLASS_NAME,     // class
		"DirectDraw Full-Screen Demo", // title
		WS_POPUP | WS_VISIBLE,
		0, 0,	  // initial x,y
		SCREEN_WIDTH, SCREEN_HEIGHT,  // initial width, height
		NULL,	  // handle to parent 
		NULL,	  // handle to menu
		hinstance,// instance of this application
		NULL)))	// extra creation parms
		return(0);

	// save main window handle
	main_window_handle = hwnd;

	// initialize game here
	Game_Init();

	// enter main event loop
	while (TRUE)
	{
		// test if there is a message in queue, if so get it
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// test if this is a quit
			if (msg.message == WM_QUIT)
				break;

			// translate any accelerator keys
			TranslateMessage(&msg);

			// send the message to the window proc
			DispatchMessage(&msg);
		} // end if

		// main game processing goes here
		Game_Main();

	} // end while

// closedown game here
	Game_Shutdown();

	// return to Windows like this
	return(msg.wParam);

} // end WinMain

///////////////////////////////////////////////////////////

