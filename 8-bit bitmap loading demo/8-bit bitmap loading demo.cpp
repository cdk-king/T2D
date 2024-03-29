// DEMO7_10.CPP 8-bit bitmap loading demo

//和在工程设置里写上链入wpcap.lib的效果一样（两种方式等价，或说一个隐式一个显式调用）
#pragma comment(lib,"ddraw.lib") 

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  // just say no to MFC

#define INITGUID

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

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// container structure for bitmaps .BMP file
typedef struct BITMAP_FILE_TAG
{
	BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
	BITMAPINFOHEADER bitmapinfoheader;  // this is all the info including the palette
	PALETTEENTRY     palette[256];      // we will store the palette here
	UCHAR           *buffer;           // this is a pointer to the data

} BITMAP_FILE, *BITMAP_FILE_PTR;

// PROTOTYPES  //////////////////////////////////////////////

int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height);

int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename);

int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap);

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, int color);

// MACROS /////////////////////////////////////////////////

// tests if a key is up or down
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// initializes a direct draw struct
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// this builds a 16 bit color value in 5.5.5 format (1-bit alpha mode)
#define _RGB16BIT555(r,g,b) ((b & 31) + ((g & 31) << 5) + ((r & 31) << 10))

// this builds a 16 bit color value in 5.6.5 format (green dominate mode)
#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 5) + ((r & 31) << 11))

// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))


// GLOBALS ////////////////////////////////////////////////

HWND      main_window_handle = NULL; // globally track main window
int       window_closed = 0;    // tracks if window is closed
HINSTANCE hinstance_app = NULL; // globally track hinstance

// directdraw stuff

LPDIRECTDRAW7         lpdd = NULL;   // dd object
LPDIRECTDRAWSURFACE7  lpddsprimary = NULL;   // dd primary surface
LPDIRECTDRAWSURFACE7  lpddsback = NULL;   // dd back surface
LPDIRECTDRAWPALETTE   lpddpal = NULL;   // a pointer to the created dd palette
LPDIRECTDRAWCLIPPER   lpddclipper = NULL;   // dd clipper
PALETTEENTRY          palette[256];          // color palette
PALETTEENTRY          save_palette[256];     // used to save palettes
DDSURFACEDESC2        ddsd;                  // a direct draw surface description struct
DDBLTFX               ddbltfx;               // used to fill
DDSCAPS2              ddscaps;               // a direct draw surface capabilities struct
HRESULT               ddrval;                // result back from dd calls
DWORD                 start_clock_count = 0; // used for timing

BITMAP_FILE           bitmap;                // holds the bitmap

char buffer[80];                             // general printing buffer

// FUNCTIONS ////////////////////////////////////////////////

int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename)
{
	// this function opens a bitmap file and loads the data into bitmap

	int file_handle,  // the file handle
		index;        // looping index

	UCHAR   *temp_buffer = NULL; // used to convert 24 bit images to 16 bit-用于将24位图像转换为16位图像？
	OFSTRUCT file_data;          // the file data information

	// 打开文件（如果存在）
	if ((file_handle = OpenFile(filename, &file_data, OF_READ)) == -1)
		return(0);

	// now load the bitmap file header
	//现在加载位图文件头
	/*WINBASEAPI
	UINT
		WINAPI
		_lread(
			_In_ HFILE hFile,
			_Out_writes_bytes_to_(uBytes, return) LPVOID lpBuffer,
			_In_ UINT uBytes
		);*/

	_lread(file_handle, &bitmap->bitmapfileheader, sizeof(BITMAPFILEHEADER));

	// test if this is a bitmap file
	if (bitmap->bitmapfileheader.bfType != BITMAP_ID)
	{
		// close the file
		_lclose(file_handle);

		// return error
		return(0);
	} // end if

 // now we know this is a bitmap, so read in all the sections

 // first the bitmap infoheader

 // now load the bitmap file header
	_lread(file_handle, &bitmap->bitmapinfoheader, sizeof(BITMAPINFOHEADER));

	// now load the color palette if there is one
	if (bitmap->bitmapinfoheader.biBitCount == 8)
	{
		_lread(file_handle, &bitmap->palette, MAX_COLORS_PALETTE * sizeof(PALETTEENTRY));

		// now set all the flags in the palette correctly and fix the reversed 
		//现在正确设置调色板中的所有标志并修复反转的
		// BGR RGBQUAD data format
		for (index = 0; index < MAX_COLORS_PALETTE; index++)
		{
			// reverse the red and green fields
			int temp_color = bitmap->palette[index].peRed;
			bitmap->palette[index].peRed = bitmap->palette[index].peBlue;
			bitmap->palette[index].peBlue = temp_color;

			// always set the flags word to this
			//总是将标志字设置为
			bitmap->palette[index].peFlags = PC_NOCOLLAPSE;
		} // end for index

	} // end if

// finally the image data itself
	//重新定位文件的读/写位置
	//_lseek(file_handle, -(int)(bitmap->bitmapinfoheader.biSizeImage), SEEK_END);

	// now read in the image, if the image is 8 or 16 bit then simply read it
	// but if its 24 bit then read it into a temporary area and then convert
	// it to a 16 bit image
	//现在读取图像，如果图像是8或16位，那么只需读取它
	//但如果是24位，则将其读取到临时区域，然后转换
	//转换为16位图像

	//|| bitmap->bitmapinfoheader.biBitCount == 16 || bitmap->bitmapinfoheader.biBitCount == 24 
	if (bitmap->bitmapinfoheader.biBitCount == 8 || bitmap->bitmapinfoheader.biBitCount == 16 || bitmap->bitmapinfoheader.biBitCount == 24)
	{
		//删除最后一个图像（如果有）上一个
		// delete the last image if there was one
		//if (bitmap->buffer)
			//free(bitmap->buffer);

		// allocate the memory for the image
		//为图像分配内存
		if (!(bitmap->buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
		{
			// close the file
			_lclose(file_handle);

			// return error
			return(0);
		} // end if

	 // now read it in
		_lread(file_handle, bitmap->buffer, bitmap->bitmapinfoheader.biSizeImage);

	} // end if
	else
	{
		// serious problem
		//return(0);

	} // end else

//如果给定条件为真，则编译下面代码
//#if 0
#if 1
// write the file info out 
	printf("\nfilename:%s \nsize=%d \nwidth=%d \nheight=%d \nbitsperpixel=%d \ncolors=%d \nimpcolors=%d",
		filename,
		bitmap->bitmapinfoheader.biSizeImage,
		bitmap->bitmapinfoheader.biWidth,
		bitmap->bitmapinfoheader.biHeight,
		bitmap->bitmapinfoheader.biBitCount,
		bitmap->bitmapinfoheader.biClrUsed,
		bitmap->bitmapinfoheader.biClrImportant);
#endif

	// close the file
	_lclose(file_handle);

	// flip the bitmap
	//大多数的bmp文件是上下颠倒的，需要翻转
	Flip_Bitmap(bitmap->buffer,
		bitmap->bitmapinfoheader.biWidth*(bitmap->bitmapinfoheader.biBitCount / 8),
		bitmap->bitmapinfoheader.biHeight);

	// return success
	return(1);

} // end Load_Bitmap_File

///////////////////////////////////////////////////////////

int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap)
{
	// this function releases all memory associated with "bitmap"
	if (bitmap->buffer)
	{
		// release memory
		free(bitmap->buffer);

		// reset pointer
		bitmap->buffer = NULL;

	} // end if

 // return success
	return(1);

} // end Unload_Bitmap_File

///////////////////////////////////////////////////////////

int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height)
{
	// this function is used to flip bottom-up .BMP images

	UCHAR *buffer; // used to perform the image processing
	int index;     // looping index

	// allocate the temporary buffer
	if (!(buffer = (UCHAR *)malloc(bytes_per_line*height)))
		return(0);

	// copy image to work area
	memcpy(buffer, image, bytes_per_line*height);

	// flip vertically
	for (index = 0; index < height; index++)
		memcpy(&image[((height - 1) - index)*bytes_per_line],
			&buffer[index*bytes_per_line], bytes_per_line);

	// release the memory
	free(buffer);

	// return success
	return(1);

} // end Flip_Bitmap

///////////////////////////////////////////////////////////////

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, int color)
{
	DDBLTFX ddbltfx; // this contains the DDBLTFX structure

	// clear out the structure and set the size field 
	DDRAW_INIT_STRUCT(ddbltfx);

	// set the dwfillcolor field to the desired color
	ddbltfx.dwFillColor = color;

	// ready to blt to surface
	lpdds->Blt(NULL,       // ptr to dest rectangle
		NULL,       // ptr to source surface, NA            
		NULL,       // ptr to source rectangle, NA
		DDBLT_COLORFILL | DDBLT_WAIT,   // fill and wait                   
		&ddbltfx);  // ptr to DDBLTFX structure

// return success
	return(1);
} // end DDraw_Fill_Surface


///////////////////////////////////////////////////////////////

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

	// make sure this isn't executed again
	if (window_closed)
		return(0);

	// for now test if user is hitting ESC and send WM_CLOSE
	if (KEYDOWN(VK_ESCAPE))
	{
		PostMessage(main_window_handle, WM_CLOSE, 0, 0);
		window_closed = 1;
	} // end if

	RECT source_rect, // used to hold the destination RECT
		dest_rect;  // used to hold the destination RECT

 // get a random rectangle for source
	int x1 = rand() % SCREEN_WIDTH;
	int y1 = rand() % SCREEN_HEIGHT;
	int x2 = rand() % SCREEN_WIDTH;
	int y2 = rand() % SCREEN_HEIGHT;

	// get a random rectangle for destination
	int x3 = rand() % SCREEN_WIDTH;
	int y3 = rand() % SCREEN_HEIGHT;
	int x4 = rand() % SCREEN_WIDTH;
	int y4 = rand() % SCREEN_HEIGHT;

	// now set up the RECT structure to fill the region from
	// (x1,y1) to (x2,y2) on the source surface
	source_rect.left = x1;
	source_rect.top = y1;
	source_rect.right = x2;
	source_rect.bottom = y2;

	// now set up the RECT structure to fill the region from
	// (x3,y3) to (x4,y4) on the destination surface
	dest_rect.left = x3;
	dest_rect.top = y3;
	dest_rect.right = x4;
	dest_rect.bottom = y4;

	// make the blitter call
	if (FAILED(lpddsprimary->Blt(NULL,  // pointer to dest RECT &dest_rect
		lpddsback,   // pointer to source surface
		NULL,// pointer to source RECT &source_rect
		DDBLT_WAIT,  // control flags
		NULL)))      // pointer to DDBLTFX holding info
		return(0);

	//Sleep(30);

 // copy the bitmap image to the primary buffer line by line
 // note this is a good candidate operation to make into a function - hint!

 // lock the primary surface
	lpddsback->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

	// get video pointer to primary surfce
	UINT  *primary_buffer = (UINT *)ddsd.lpSurface;

	// test if memory is linear
	if (ddsd.lPitch == SCREEN_WIDTH*4)
	{
		// copy memory from double buffer to primary buffer
		//memcpy((void *)primary_buffer, (void *)bitmap.buffer, SCREEN_WIDTH*SCREEN_HEIGHT);

		// process each line and copy it into the primary buffer
		for (int index_y = 0; index_y < SCREEN_HEIGHT; index_y++)
		{
			for (int index_x = 0; index_x < SCREEN_WIDTH; index_x++)
			{
				// get BGR values
				UCHAR blue = (bitmap.buffer[index_y*SCREEN_WIDTH * 3 + index_x * 3 + 0]),
					green = (bitmap.buffer[index_y*SCREEN_WIDTH * 3 + index_x * 3 + 1]),
					red = (bitmap.buffer[index_y*SCREEN_WIDTH * 3 + index_x * 3 + 2]);

				// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
				int pixel = _RGB32BIT(128, red, green, blue);

				// write the pixel
				primary_buffer[index_x + (index_y*(ddsd.lPitch >> 2))] = pixel;

			} // end for index_x
			
		} // end for index_y


	} // end if
	else
	{ // non-linear

	// make copy of source and destination addresses
		UINT  *dest_ptr = primary_buffer;
		UCHAR *src_ptr = bitmap.buffer;

		// memory is non-linear, copy line by line
		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			// copy line
			memcpy((void *)dest_ptr, (void *)src_ptr, SCREEN_WIDTH*4);

			// advance pointers to next line
			dest_ptr += ddsd.lPitch/4;
			src_ptr += SCREEN_WIDTH;
		} // end for

	} // end else

 // now unlock the primary surface
	if (FAILED(lpddsback->Unlock(NULL)))
		return(0);


	// do nothing -- look at pretty picture

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
		return(0);

	// set display mode to 640x480x8
	if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, 0, 0)))
		return(0);

	// clear ddsd and set size
	DDRAW_INIT_STRUCT(ddsd);

	// enable valid fields
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

	// set the backbuffer count field to 1, use 2 for triple buffering
	ddsd.dwBackBufferCount = 1;

	// request a complex, flippable
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;

	// create the primary surface
	if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
		return(0);

	// this line is needed by the call
	ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

	// get the attached back buffer surface
	if (FAILED(lpddsprimary->GetAttachedSurface(&ddsd.ddsCaps, &lpddsback)))
		return(0);

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

	// create the palette object
	if (FAILED(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256 |
		DDPCAPS_INITIALIZE,
		palette, &lpddpal, NULL)))
		return(0);

	// finally attach the palette to the primary surface
	//if (FAILED(lpddsprimary->SetPalette(lpddpal)))
		//return(0);




	// draw a color gradient in back buffer
	DDRAW_INIT_STRUCT(ddsd);

	// lock the back buffer
	if (FAILED(lpddsback->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL)))
		return(0);

	// get alias to start of surface memory for fast addressing
	UINT *video_buffer = (UINT *)ddsd.lpSurface;

	// draw the gradient
	for (int index_y = 0; index_y < SCREEN_HEIGHT; index_y++)
	{
		// fill next line with color
		memset((void *)video_buffer, index_y * 255 / SCREEN_HEIGHT , SCREEN_WIDTH * 4);

		// advance pointer
		video_buffer += ddsd.lPitch/4;

	} // end for index_y

	// unlock the back buffer
	if (FAILED(lpddsback->Unlock(NULL)))
		return(0);


	// load the 8-bit image
	if (!Load_Bitmap_File(&bitmap, "bitmap24.bmp"))
		return(0);

	// load it's palette into directdraw
	//if (FAILED(lpddpal->SetEntries(0, 0, MAX_COLORS_PALETTE, bitmap.palette)))
		//return(0);

	// clean the surface
	DDraw_Fill_Surface(lpddsprimary, 0);


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

 // unload the bitmap file, we no longer need it
	Unload_Bitmap_File(&bitmap);


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
		"DirectDraw 8-Bit Bitmap Loading", // title
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

