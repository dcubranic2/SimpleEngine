#pragma once
#include <malloc.h>
#include <string>
#include <iostream>

class matrix
{
private:
	int _size;
	float* _data = NULL;
	std::string _tag;
public:
	matrix(int size, float pdata[]);
	~matrix();
	matrix operator  +(matrix &b);
	matrix operator  -(matrix &b);
	matrix operator  *(matrix &b);
	matrix operator  =(matrix &b);
	matrix dot(matrix &a,matrix &b);
	matrix scalar(const float a, matrix &b);
	void print();
	void set_tag(std::string _ptag);
};
