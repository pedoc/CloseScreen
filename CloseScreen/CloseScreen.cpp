#include <iostream>
#include <Windows.h>
#include <WinUser.h>
int main()
{
	PostMessage((HWND)-1,0x112,0xF170,2);
}