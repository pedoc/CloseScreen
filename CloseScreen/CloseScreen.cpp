// CloseScreen.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <WinUser.h>

int main()
{
    PostMessage((HWND)-1,0x112,0xF170,2);
}
