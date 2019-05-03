//includes
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
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
	case WM_MOUSEMOVE:
	{
		int mouse_x = (int)LOWORD(lparam);
		int mouse_y = (int)HIWORD(lparam);

		int buttons = (int)wparam;

		// get a graphics context
		hdc = GetDC(hwnd);

		// set the foreground color to green
		SetTextColor(hdc, RGB(0, 255, 0));

		// set the background color to black
		SetBkColor(hdc, RGB(0, 0, 0));

		// set the transparency mode to OPAQUE
		SetBkMode(hdc, OPAQUE);

		// print the ascii code and key state
		sprintf(buffer, "Mouse (X,Y) = (%d,%d)      ", mouse_x, mouse_y);
		TextOut(hdc, 0, 150, buffer, strlen(buffer));

		sprintf(buffer, "Right Button = %d  ", ((buttons & MK_RBUTTON) ? 1 : 0));
		TextOut(hdc, 0, 166, buffer, strlen(buffer));

		sprintf(buffer, "Left Button = %d  ", ((buttons & MK_LBUTTON) ? 1 : 0));
		TextOut(hdc, 0, 182, buffer, strlen(buffer));

		// release the dc back
		ReleaseDC(hwnd, hdc);

		if (buttons & MK_LBUTTON) {

		}

		if (buttons & MK_RBUTTON) {

		}

	}break;
	case WM_CHAR:
	{
		char buffer[30];
		char ascii_code = wparam;
		unsigned int key_state = lparam;

		hdc = GetDC(hwnd);

		SetTextColor(hdc, RGB(0, 255, 0));

		SetBkColor(hdc, RGB(0, 0, 0));

		SetBkMode(hdc, OPAQUE);

		sprintf(buffer, "WM_CHAR: Character = %c   ", ascii_code);
		TextOut(hdc, 0, 0, buffer, strlen(buffer));

		sprintf(buffer, "Key State = 0X%X", key_state);
		TextOut(hdc, 0, 16, buffer, strlen(buffer));

		ReleaseDC(hwnd, hdc);



	}break;
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

//HINSTANCE hinstance_app;
//HWND main_window_handle;

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
	HDC hdc;

	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = GetStockBrush(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	hinstance_app = hinstance;

	//注册winclass
	if (!RegisterClassEx(&winclass)) {
		return(0);
	}

	//create the window
	if (!(hwnd = CreateWindowEx(NULL,
		WINDOW_CLASS_NAME,
		"SendMessage Demo",
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

	main_window_handle = hwnd;

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

		hdc = GetDC(hwnd);

		SetTextColor(hdc, RGB(0, 255, 0));

		SetBkColor(hdc, RGB(0, 0, 0));

		SetBkMode(hdc, OPAQUE);

		sprintf(buffer, "Up Arrow = %d   ", KEYDOWN(VK_UP));
		TextOut(hdc, 0, 48, buffer, strlen(buffer));

		sprintf(buffer, "Down Arrow = %d   ", KEYDOWN(VK_DOWN));
		TextOut(hdc, 0, 64, buffer, strlen(buffer));

		sprintf(buffer, "Rignt Arrow = %d   ", KEYDOWN(VK_RIGHT));
		TextOut(hdc, 0, 80, buffer, strlen(buffer));

		sprintf(buffer, "Left Arrow = %d   ", KEYDOWN(VK_LEFT));
		TextOut(hdc, 0, 96, buffer, strlen(buffer));

		ReleaseDC(hwnd, hdc);

		if (KEYDOWN(VK_ESCAPE)) {
			SendMessage(hwnd,WM_CLOSE,0,0);
		}

		//Game_Main();
	}

	return (msg.wParam);

}	//end WinMain