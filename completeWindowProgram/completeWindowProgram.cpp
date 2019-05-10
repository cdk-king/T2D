//includes
#define WIN32_LEAN_AND_MEAN

#include <windows.h>   // include all the windows headers
#include <windowsx.h>  // include useful macros
#include <mmsystem.h>  // very important and include WINMM.LIB too!
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//defines
#define WINDOW_CLASS_NAME "WINCLASS1"

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

//globals
HWND      main_window_handle = NULL; // globally track main window
HINSTANCE hinstance_app = NULL; // globally track hinstance
char buffer[80];

//functions
//WinProc
LRESULT CALLBACK WindowProc(
HWND hwnd,
UINT msg,
WPARAM wparam,
LPARAM lparam
) {
	PAINTSTRUCT ps;
	HDC hdc;

	switch (msg)
	{
	case WM_CREATE:
	{
		return(0);
	} break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		return(0);
	} break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return(0);
	} break;

	default:
		break;
	} //end switch

	return (DefWindowProc(hwnd,msg,wparam,lparam));

} //end WinProc

//WinMain
int WINAPI WinMain(
HINSTANCE hinstance,
HINSTANCE hprevinstance,
LPSTR lpcmdline,
int ncmdshow
) 
{
	WNDCLASSEX winclass;
	HWND hwnd;
	MSG msg;

	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	winclass.hbrBackground = GetStockBrush(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	
	//注册winclass
	if (!RegisterClassEx(&winclass)) {
		return(0);
	}

	//create the window
	if (!(hwnd = CreateWindowEx(NULL, 
		WINDOW_CLASS_NAME,
		"Your Basic Window",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0,0,
		400,400,
		NULL,
		NULL,
		hinstance,
		NULL
		))) {
		return(0);
	}

	//非实时循环
	/*while (GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/

	//实时事件循环
	while (TRUE){
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Sleep(30);

		// main game processing goes here
		if (KEYDOWN(VK_ESCAPE))
		SendMessage(hwnd, WM_CLOSE, 0, 0);		

		//Game_Main();
	}


	return (msg.wParam);

}	//end WinMain