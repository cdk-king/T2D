// DEMO2_1.CPP - standard version
#include <stdio.h>
#include <iostream>
//#include <cstring>

//当使用<iostream>的时候，该头文件没有定义全局命名空间，必须使用namespace std；这样才能正确使用cout。
using namespace std;
// main entry point for all standard DOS/console programs
void main(void)
{
	cout << "Hello World" << endl;
	printf("\nTHERE CAN BE ONLY ONE!!!\n");
	
	//程序末尾增加输入语句，这样程序运行结束前会要求用户输入，控制台就会保持存在
	//char a;
	//cin >> a;
	//printf("%c",a);

	//在程序末尾添加语句：system("pause"); 加上这句后，控制台显示运行结果后会显示“请按任意键继续”
	system("pause");
} // end main        

