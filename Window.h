#pragma once
#include"Platform.h"
#include<iostream>

class Window
{
private:
	HWND _window_handle;
	HINSTANCE _app_instance;
	
public:
	Window(const char * pname);
	~Window();
};

