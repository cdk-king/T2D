//includes
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <math.h>
#include "resource.h"

//defines
#define WINDOW_CLASS_NAME "WINCLASS1"

//globals

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

	return (DefWindowProc(hwnd, msg, wparam, lparam));

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
	winclass.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(ICON_T3DX));
	winclass.hCursor = LoadCursor(hinstance, MAKEINTRESOURCE(CURSOR_CROSSHAIR));
	winclass.hbrBackground = GetStockBrush(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(hinstance, MAKEINTRESOURCE(ICON_T3DX));

	//注册winclass
	if (!RegisterClassEx(&winclass)) {
		return(0);
	}

	//create the window
	if (!(hwnd = CreateWindowEx(NULL,
		WINDOW_CLASS_NAME,
		"Your Basic Window",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,
		400, 400,
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
	while (TRUE) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Game_Main();
	}

	return (msg.wParam);

}	//end WinMain