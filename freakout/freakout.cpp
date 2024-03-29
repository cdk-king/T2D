// FREAKOUT.CPP - break game demo

// INCLUDES ///////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN // include all macros
#define INITGUID            // include all GUIDs 

#include <windows.h>        // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>

#include <iostream>       // include important C/C++ stuff
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

#include <ddraw.h>          // directX includes
#include "blackbox.h"       // game library includes

// DEFINES ////////////////////////////////////////////////////

// defines for windows 
#define WINDOW_CLASS_NAME "WIN3DCLASS"  // class name

#define WINDOW_WIDTH            640     // size of window
#define WINDOW_HEIGHT           480

// states for game loop
#define GAME_STATE_INIT         0
#define GAME_STATE_START_LEVEL  1
#define GAME_STATE_RUN          2
#define GAME_STATE_SHUTDOWN     3
#define GAME_STATE_EXIT         4 

// block defines
#define NUM_BLOCK_ROWS          6
#define NUM_BLOCK_COLUMNS       8

#define BLOCK_WIDTH             64
#define BLOCK_HEIGHT            16
#define BLOCK_ORIGIN_X          8
#define BLOCK_ORIGIN_Y          8
#define BLOCK_X_GAP             80
#define BLOCK_Y_GAP             32

// paddle defines
#define PADDLE_START_X          (SCREEN_WIDTH/2 - 16)
#define PADDLE_START_Y          (SCREEN_HEIGHT - 32);
#define PADDLE_WIDTH            64
#define PADDLE_HEIGHT           8
#define PADDLE_COLOR            191

// ball defines
#define BALL_START_Y            (SCREEN_HEIGHT/2)
#define BALL_SIZE                4

// PROTOTYPES /////////////////////////////////////////////////

// game console
int Game_Init(void *parms=NULL);
int Game_Shutdown(void *parms=NULL);
int Game_Main(void *parms=NULL);

// GLOBALS ////////////////////////////////////////////////////

HWND main_window_handle  = NULL; // save the window handle
HINSTANCE main_instance  = NULL; // save the instance
int game_state           = GAME_STATE_INIT; // starting state

int paddle_x = 0, paddle_y = 0; // tracks position of paddle
int ball_x   = 0, ball_y   = 0; // tracks position of ball
int ball_dx  = 0, ball_dy  = 0; // velocity of ball
int score    = 0;               // the score
int level    = 1;               // the current level
int blocks_hit = 0;             // tracks number of blocks hit
char info[80];
int lastTime = 0;
int fps = 0;
HDC			hdc;
HWND	   hwnd;

int Draw_Rectangle(int x1, int y1, int x2, int y2, int r,int g,int b,
	LPDIRECTDRAWSURFACE7 lpdds)
{
	// this function uses directdraw to draw a filled rectangle
	DDBLTFX ddbltfx; // this contains the DDBLTFX structure
	RECT fill_area;  // this contains the destination rectangle

	// clear out the structure and set the size field 
	DD_INIT_STRUCT(ddbltfx);

	// set the dwfillcolor field to the desired color
	ddbltfx.dwFillColor = RGB(r,g, b);

	// fill in the destination rectangle data (your data)
	fill_area.top = y1;
	fill_area.left = x1;
	fill_area.bottom = y2 + 1;
	fill_area.right = x2 + 1;

	// ready to blt to surface, in this case blt to primary
	lpdds->Blt(&fill_area, // ptr to dest rectangle
	           NULL,       // ptr to source surface, NA            
	           NULL,       // ptr to source rectangle, NA
	           DDBLT_COLORFILL | DDBLT_WAIT | DDBLT_ASYNC,   // fill and wait                   
	           &ddbltfx);  // ptr to DDBLTFX structure
	
	//HBRUSH hbrush = CreateSolidBrush(RGB(color, 0, 0));

	// draw either a filled rect or a wireframe rect
	//FillRect(hdc, &fill_area, CreateSolidBrush(RGB(color, 0, 0)));

	/*HPEN pen = CreatePen(PS_SOLID,1,color);
	HBRUSH hbrush = CreateSolidBrush(color);
	SelectObject(hdc,pen);
	SelectObject(hdc,hbrush);
	Rectangle(hdc, x1,y1,x2+1,y2+1);

	DeleteObject(pen);
	DeleteObject(hbrush);*/

	// return success
	return(1);

} // end Draw_Rectangle

// this contains the game grid data   

UCHAR blocks[NUM_BLOCK_ROWS][NUM_BLOCK_COLUMNS][4];     

// FUNCTIONS //////////////////////////////////////////////////

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
// this is the main message handler of the system
PAINTSTRUCT	ps;		   // used in WM_PAINT
//HDC			hdc;	   // handle to a device context

// what is the message 
switch(msg)
	{	
	case WM_CREATE: 
        {
		// do initialization stuff here
		return(0);
		} break;

    case WM_PAINT:
         {
         // start painting
         hdc = BeginPaint(hwnd,&ps);

         // the window is now validated 

         // end painting
         EndPaint(hwnd,&ps);
         return(0);
        } break;

	case WM_DESTROY: 
		{
		// kill the application			
		PostQuitMessage(0);
		return(0);
		} break;

	default:break;

    } // end switch

// process any messages that we didn't take care of 
return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

// WINMAIN ////////////////////////////////////////////////////

int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{
// this is the winmain function

WNDCLASS winclass;	// this will hold the class we create
HWND	 hwnd;		// generic window handle
MSG		 msg;		// generic message
//HDC      hdc;       // generic dc
PAINTSTRUCT ps;     // generic paintstruct

// first fill in the window class stucture
winclass.style			= CS_DBLCLKS | CS_OWNDC | 
                          CS_HREDRAW | CS_VREDRAW;
winclass.lpfnWndProc	= WindowProc;
winclass.cbClsExtra		= 0;
winclass.cbWndExtra		= 0;
winclass.hInstance		= hinstance;
winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= NULL; 
winclass.lpszClassName	= WINDOW_CLASS_NAME;

// register the window class
if (!RegisterClass(&winclass))
	return(0);

// create the window, note the use of WS_POPUP
if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME,    // class
             "freakout",	// title
	//(WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION),
			 WS_POPUP | WS_VISIBLE,
			 0,0,	                // initial x,y
   		     GetSystemMetrics(SM_CXSCREEN),  // intial width
             GetSystemMetrics(SM_CYSCREEN),  // initial height
			 //640,480,
			 NULL,	    // handle to parent 
			 NULL,	    // handle to menu
			 hinstance,// instance
			 NULL)))	// creation parms
return(0);

// hide mouse
//隐藏鼠标
ShowCursor(FALSE);

// save the window handle and instance in a global
main_window_handle = hwnd;
main_instance      = hinstance;

// perform all game console specific initialization
Game_Init();

hdc = GetDC(hwnd);

// enter main event loop
while(1)
	{
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
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

ReleaseDC(hwnd, hdc);
// shutdown game and release all resources
Game_Shutdown();

// show mouse
ShowCursor(TRUE);

// return to Windows like this
return(msg.wParam);

} // end WinMain

// T3DX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////////

int Game_Init(void *parms)
{
// this function is where you do all the initialization 
// for your game


// return success
return(1);

} // end Game_Init

///////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
// this function is where you shutdown your game and
// release all resources that you allocated
	
	// first the palette
	//先清除最后创建的，最后在清除最先创建的lpdd
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

///////////////////////////////////////////////////////////////

void Init_Blocks(void)
{
// initialize the block field
	for (int row = 0; row < NUM_BLOCK_ROWS; row++) {
		for (int col = 0; col < NUM_BLOCK_COLUMNS; col++)
		{
			blocks[row][col][0] = row * 8 + col * 8+50;
			blocks[row][col][1] = row * 8 + col * 8+50;
			blocks[row][col][2] = 0;
			blocks[row][col][3] = 128;
		}
	}

} // end Init_Blocks

///////////////////////////////////////////////////////////////

void Draw_Blocks(void)
{
// this function draws all the blocks in row major form
int x1 = BLOCK_ORIGIN_X, // used to track current position
    y1 = BLOCK_ORIGIN_Y; 

// draw all the blocks
for (int row=0; row < NUM_BLOCK_ROWS; row++)
    {    
    // reset column position
    x1 = BLOCK_ORIGIN_X;

    // draw this row of blocks
    for (int col=0; col < NUM_BLOCK_COLUMNS; col++)
        {
        // draw next block (if there is one)
        if (blocks[row][col][3]!=0)
            {
            // draw block   
			//阴影
            Draw_Rectangle(x1-2,y1+2,
                 x1+BLOCK_WIDTH-2,y1+BLOCK_HEIGHT+2,0,0,0);
			//实体
            Draw_Rectangle(x1,y1,x1+BLOCK_WIDTH,
                 y1+BLOCK_HEIGHT,blocks[row][col][0], blocks[row][col][1], blocks[row][col][2]);
            } // end if

        // advance column position
		//实体宽度家间隙宽度
        x1+=BLOCK_X_GAP;
        } // end for col

    // advance to next row position
    y1+=BLOCK_Y_GAP;

    } // end for row

} // end Draw_Blocks

///////////////////////////////////////////////////////////////

void Process_Ball(void)
{
// this function tests if the ball has hit a block or the paddle
// if so, the ball is bounced and the block is removed from 
// the playfield note: very cheesy collision algorithm :)

// first test for ball block collisions

// the algorithm basically tests the ball against each 
// block's bounding box this is inefficient, but easy to 
// implement, later we'll see a better way

int x1 = BLOCK_ORIGIN_X, // current rendering position（当前位置）
    y1 = BLOCK_ORIGIN_Y; 

int ball_cx = ball_x+(BALL_SIZE/2),  // 计算球的中心
    ball_cy = ball_y+(BALL_SIZE/2);

// test of the ball has hit the paddle
//ball_dy > 0（方向向下）
if (ball_y > (SCREEN_HEIGHT/2) && ball_dy > 0)
   {
   // extract leading edge of ball
   int x = ball_x+(BALL_SIZE/2);
   int y = ball_y+(BALL_SIZE/2);

   // test for collision with paddle
   //碰撞检测
   if ((x >= paddle_x && x <= paddle_x+PADDLE_WIDTH) &&
       (y >= paddle_y && y <= paddle_y+PADDLE_HEIGHT))
       {
       // reflect ball（反射）
       ball_dy=-ball_dy;

       // push ball out of paddle since it made contact
	   //提前弹起
       ball_y+=ball_dy;

       // add a little english to ball based on motion of paddle
	   //添加摩檫力？
       if (KEY_DOWN(VK_RIGHT))
          ball_dx-=(rand()%3);
       else
       if (KEY_DOWN(VK_LEFT))
          ball_dx+=(rand()%3);
       else
          ball_dx+=(-1+rand()%3);
	   
	   // draw the info
	   //sprintf(info, "(x-paddle_x) %d", (x - paddle_x));
	   

	   if ((x-paddle_x)>PADDLE_WIDTH/2) {
		   ball_dx += (rand() % 3 * ((x - paddle_x- (PADDLE_WIDTH / 2)) / (PADDLE_WIDTH / 2)));
	   }
	   else {
		   ball_dx -= (rand() % 3 * (((PADDLE_WIDTH / 2)-(x - paddle_x)) / (PADDLE_WIDTH / 2)));
	   }
       
       // test if there are no blocks, if so send a message
       // to game loop to start another level
	   //检测是否通关
       if (blocks_hit >= (NUM_BLOCK_ROWS*NUM_BLOCK_COLUMNS))
          {
          game_state = GAME_STATE_START_LEVEL;
          level++;
          } // end if

       // make a little noise
       MessageBeep(MB_OK);

       // return
       return; 

       } // end if

   } // end if

// now scan thru all the blocks and see of ball hit blocks
//现在扫视所有的障碍物，看球击中障碍物
for (int row=0; row < NUM_BLOCK_ROWS; row++)
    {    
    // reset column position
    x1 = BLOCK_ORIGIN_X;

    // scan this row of blocks
    for (int col=0; col < NUM_BLOCK_COLUMNS; col++)
        {
        // if there is a block here then test it against ball
		//如果这里有障碍物，那么用球来测试它。
        if (blocks[row][col][3]!=0)
           {
            // test ball against bounding box of block
			//在讲道理这个碰撞检测不精确
           if ((ball_cx > x1) && (ball_cx < x1+BLOCK_WIDTH) &&     
               (ball_cy > y1) && (ball_cy < y1+BLOCK_HEIGHT))
               {
               // remove the block
			   //移除障碍物
               blocks[row][col][3] = 0;
			   

               // increment global block counter, so we know 
               // when to start another level up
			   //增加击中次数
               blocks_hit++;

               // bounce the ball
			   //反弹
               ball_dy=-ball_dy;

               // add a little 摩檫力？
               ball_dx+=(-1+rand()%3);

               // make a little noise
			   //添加音效
               MessageBeep(MB_OK);

               // add some points
			   //加分数
               score+=5*(level+(abs(ball_dx)));

               // that's it -- no more block
               return;

               } // end if  

           } // end if

        // advance column position
        x1+=BLOCK_X_GAP;
        } // end for col

    // advance to next row position
    y1+=BLOCK_Y_GAP;

    } // end for row

} // end Process_Ball

///////////////////////////////////////////////////////////////

//cdk书签

int Game_Main(void *parms)
{
	
// this is the workhorse of your game it will be called
// continuously in real-time this is like main() in C
// all the calls for you game go here!

char buffer[80]; // used to print text

// what state is the game in? 
if (game_state == GAME_STATE_INIT)
    {
    // initialize everything here graphics
	//初始化此处的所有内容图形
    DD_Init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

    // seed the random number generator
    // so game is different each play
    srand(Start_Clock());

    // set the paddle position here to the middle bottom
    paddle_x = PADDLE_START_X;
    paddle_y = PADDLE_START_Y;

    // 初始化球的位置和速度
    ball_x = 8+rand()%(SCREEN_WIDTH-16);
    ball_y = BALL_START_Y;
    ball_dx = -4 + rand()%(8+1);
    ball_dy = 6 + rand()%2;

    // transition to start level state
	//进入开始游戏关卡阶段
    game_state = GAME_STATE_START_LEVEL;

    } // end if 
////////////////////////////////////////////////////////////////
else
if (game_state == GAME_STATE_START_LEVEL)
    {
    // get a new level ready to run

    // initialize the blocks
	// 初始化障碍物
    Init_Blocks();

    // reset block counter
	// 重新设置击打数量
    blocks_hit = 0;

    // transition to run state
    game_state = GAME_STATE_RUN;

    } // end if
///////////////////////////////////////////////////////////////
else
if (game_state == GAME_STATE_RUN)
    {
    // start the timing clock
	// 开始游戏时间
    Start_Clock();

    // clear drawing surface for the next frame of animation
	// 清空画面
    Draw_Rectangle(0,0,SCREEN_WIDTH-1, SCREEN_HEIGHT-1,100,100,100);

    // move the paddle
    if (KEY_DOWN(VK_RIGHT))
       {
       // move paddle to right
	   //右移
       paddle_x+=8;
 
       // make sure paddle doesn't go off screen
	   //边缘检测
       if (paddle_x > (SCREEN_WIDTH-PADDLE_WIDTH))
          paddle_x = SCREEN_WIDTH-PADDLE_WIDTH;

       } // end if
    else
    if (KEY_DOWN(VK_LEFT))
       {
       // move paddle to right
		//左移
       paddle_x-=8;
 
       // make sure paddle doesn't go off screen
	   //边缘检测
       if (paddle_x < 0)
          paddle_x = 0;

       } // end if

    // draw blocks
	// 绘制障碍物
    Draw_Blocks();

    // move the ball
	//移动球
    ball_x+=ball_dx;
    ball_y+=ball_dy;

    // keep ball on screen, if the ball hits the edge of 
    // screen then bounce it by reflecting its velocity
	//边缘检测
    if (ball_x > (SCREEN_WIDTH - BALL_SIZE) || ball_x < 0) 
       {
       // reflect x-axis velocity
       ball_dx=-ball_dx;

       // update position 
       ball_x+=ball_dx;
       } // end if

    // now y-axis
    if (ball_y < 0) 
       {
       // reflect y-axis velocity
       ball_dy=-ball_dy;

       // update position 
       ball_y+=ball_dy;
       } // end if
   else 
   // penalize player for missing the ball
   if (ball_y > (SCREEN_HEIGHT - BALL_SIZE))
       {
       // reflect y-axis velocity
       ball_dy=-ball_dy;

       // update position 
       ball_y+=ball_dy;

       // minus the score
	   //扣分
       score-=50;

       } // end if

    // next watch out for ball velocity getting out of hand
	//限制速度
    if (ball_dx > 8) ball_dx = 8;
    else
    if (ball_dx < -8) ball_dx = -8;    

    // test if ball hit any blocks or the paddle
	//碰撞检测
    Process_Ball();

    // draw the paddle and shadow
	//绘制平面阴影
    Draw_Rectangle(paddle_x-4, paddle_y+4, 
                   paddle_x+PADDLE_WIDTH-4, 
                   paddle_y+PADDLE_HEIGHT+4,0,0,0);
	//绘制平面
    Draw_Rectangle(paddle_x, paddle_y, 
                   paddle_x+PADDLE_WIDTH, 
                   paddle_y+PADDLE_HEIGHT,255,0,0);

    // 绘制球体阴影
    Draw_Rectangle(ball_x-4, ball_y+4, ball_x+BALL_SIZE-4, 
                   ball_y+BALL_SIZE+4, 0,0,0);
	// 绘制球体
    Draw_Rectangle(ball_x, ball_y, ball_x+BALL_SIZE, 
                   ball_y+BALL_SIZE, 255,255,255);

    // draw the info
    sprintf(buffer,"F R E A K O U T           分数 %d             等级 %d",score,level);
	
    Draw_Text_GDI(buffer, 8,SCREEN_HEIGHT-16, 127);

	// sync to 33ish fps
	Wait_Clock(30);

	if (lastTime != 0) {
		if (Get_Clock()-lastTime > 1000) {
			fps = Get_Clock() - start_clock_count;
			lastTime = Get_Clock();
		}
	}
	else {
		lastTime = Get_Clock();
	}

	sprintf(info, "fps %d  ", fps);
	Draw_Text_GDI(info, 8, SCREEN_HEIGHT - 32, 127);

	// flip the surfaces
	DD_Flip();

    // check of user is trying to exit
    if (KEY_DOWN(VK_ESCAPE))
       {
       // send message to windows to exit
       PostMessage(main_window_handle, WM_DESTROY,0,0);

       // set exit state
       game_state = GAME_STATE_SHUTDOWN;

       } // end if

		

		//Sleep(300);

		//InvalidateRect(hwnd, NULL, TRUE);
	

    } // end if
///////////////////////////////////////////////////////////////
else
if (game_state == GAME_STATE_SHUTDOWN)
   {
   // in this state shut everything down and release resources
   DD_Shutdown();

   // switch to exit state
   game_state = GAME_STATE_EXIT;

   } // end if

// return success
return(1);

} // end Game_Main

///////////////////////////////////////////////////////////////