#pragma once
#include"Platform.h"
#include<iostream>
#include "Renderer.h"

class Window
{
private:
	HWND _window_handle;
	HINSTANCE _app_instance;
	int _width=0;
	int _height=0;
	Renderer *_r = NULL;
	
public:
	Window(Renderer *pr,const char * pname,int pwidth, int pheight);
	~Window();
	void InitOsSurface();
	void DestroyOsSurface();
};

