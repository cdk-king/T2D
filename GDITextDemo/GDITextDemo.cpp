
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
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <string>
#include "resource.h"

//defines
#define WINDOW_CLASS_NAME "WINCLASS1"

using namespace std;

//globals
HWND      main_window_handle = NULL; // globally track main window
HINSTANCE hinstance_app = NULL; // globally track hinstance

//functions

int cts_count = 0;
int max_count = 20;

class cText {
public:
	void drawCText(HDC hdc) {
		//this->
		SetTextColor(hdc, RGB(this->r, this->g, this->b));

		SetBkColor(hdc, RGB(0, 0, 0));

		SetBkMode(hdc, TRANSPARENT);

		TextOut(hdc, this->x, this->y, "CDK国王万岁!", strlen("CDK国王万岁!"));
	};
public:
	int x;
	int y;
	int r;
	int g;
	int b;
};

cText cts[20];


void Init_Texts(void)
{

}

void Create_Text(void)
{
	cText ct;
	ct.x = rand() % 400;
	ct.y = rand() % 400;
	ct.r = rand() % 256;
	ct.g = rand() % 256;
	ct.b = rand() % 256;
	int count = cts_count;
	cts[count] = ct;
	cts_count++;
}

void Delete_Text(void)
{
	//cts
	for (int i = 0;i < max_count;i++) {
		if ((i + 1)< max_count) {
			cText ct = cts[i + 1];
			cts[i] = ct;
		}
	}
	cts_count--;
}
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
		/*switch (LOWORD(wparam))
		{
		
		}*/
	}
	case WM_PAINT:
	{
		InvalidateRect(hwnd,NULL,FALSE);
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
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = GetStockBrush(BLACK_BRUSH);
	//winclass.lpszMenuName = "SoundMenu";
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	
	// save hinstance in global
	hinstance_app = hinstance;

	//注册winclass
	if (!RegisterClassEx(&winclass)) {
		return(0);
	}

	//create the window
	if (!(hwnd = CreateWindowEx(NULL,
		WINDOW_CLASS_NAME,
		"CDK",
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

	//HMENU hmenuhandle = LoadMenu(hinstance, "SoundMenu");

	//SetMenu(hwnd, hmenuhandle);

	//非实时循环
	/*while (GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/

	HDC hdc = GetDC(hwnd);

	//实时事件循环
	while (TRUE) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//rand() stdlib.h
		
		if (cts_count<max_count) {
			Create_Text();
		}
		else {
			Delete_Text();
		}

		char chInput[512];
		sprintf(chInput, "int:%d\n", cts_count);
		OutputDebugString(chInput);

		for (int i = 0;i < cts_count;i++) {
			cts[i].drawCText(hdc);
		}
		
		//等待50毫秒
		Sleep(50);
		
		//手动无效化窗口
		InvalidateRect(hwnd, NULL, TRUE);

		// sync to 33ish fps
		//GetTickCount()

		//Game_Main();
	}

	// release the dc
	ReleaseDC(hwnd, hdc);

	return (msg.wParam);

}	//end WinMain




