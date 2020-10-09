#pragma once
#include"Platform.h"
#include<iostream>

class Window
{
private:
	HWND _window_handle;
	HINSTANCE _app_instance;
	int width=0;
	int height=0;
	
public:
	Window(const char * pname,int pwidth, int pheight);
	~Window();
};

