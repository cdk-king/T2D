
#pragma comment(lib, "winmm.lib")

//includes
#define WIN32_LEAN_AND_MEAN

//使用PlaySound函数时要这样 :
//#include<windows.h>
//#include<Mmsystem.h>
//#pragma comment(lib,"winmm.lib")
//using namespace std;/*不要忘了写这句话*/

#include <windows.h>
#include <windowsx.h>
#include<Mmsystem.h>			//顺序不能错，“windows.h”要在“Mmsystem.h”
#include <stdio.h>
#include <math.h>
#include "resource.h"

//defines
#define WINDOW_CLASS_NAME "WINCLASS1"

//using namespace std;

//globals
HWND      main_window_handle = NULL; // globally track main window
HINSTANCE hinstance_app = NULL; // globally track hinstance

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
	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case MENU_FILE_ID_EXIT:
		{
			PostQuitMessage(0);
		}break;
		case MENU_HELP_ABOUT:
		{
			MessageBox(hwnd,"Menu Sound Demo","About Sound Menu",MB_OK | MB_ICONEXCLAMATION);
		}break;
		case MENU_PLAY_ID_ENERGIZE:
		{
			PlaySound(MAKEINTRESOURCE(SOUND_ID_ENERGIZE),hinstance_app,SND_RESOURCE | SND_ASYNC);
		}break;
		case MENU_PLAY_ID_BEAM:
		{
			PlaySound(MAKEINTRESOURCE(SOUND_ID_BEAM), hinstance_app, SND_RESOURCE | SND_ASYNC);
		}break;
		case MENU_PLAY_ID_TELEPORT:
		{
			PlaySound(MAKEINTRESOURCE(SOUND_ID_TELEPORT), hinstance_app, SND_RESOURCE | SND_ASYNC);
		}break;
		case MENU_PLAY_ID_WARP:
		{
			PlaySound(MAKEINTRESOURCE(SOUND_ID_WARP), hinstance_app, SND_RESOURCE | SND_ASYNC);
		}break;

		default:
			break;
		}
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		return(0);
	} break;
	case WM_DESTROY:
	{
		//PlaySound(MAKEINTRESOURCE(SOUND_ID_CREATE), hinstance_app, SND_RESOURCE | SND_SYNC);
		//PlaySound(NULL, hinstance_app, SND_PURGE);

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
	//winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(ICON_T3DX));
	//winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hCursor = LoadCursor(hinstance, MAKEINTRESOURCE(CURSOR_CROSSHAIR));
	winclass.hbrBackground = GetStockBrush(BLACK_BRUSH);
	//winclass.lpszMenuName = "SoundMenu";
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	//winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hIconSm = LoadIcon(hinstance, MAKEINTRESOURCE(ICON_T3DX));

	// save hinstance in global
	hinstance_app = hinstance;

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

	HMENU hmenuhandle = LoadMenu(hinstance, "SoundMenu");

	SetMenu(hwnd, hmenuhandle);

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