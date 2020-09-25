#include "Matrix.h"


matrix::matrix(int psize,float pdata[])
{
	_size = psize;
	_data =(float *) malloc(psize * psize * sizeof(float));
	float * xtemp = _data;
	for (int x = 0;x < _size * _size;x++)
	{
		*xtemp = pdata[x];
		xtemp++;
	}

}

matrix::~matrix()
{
	if (_data != NULL) 
	{
		free(_data);
		std::cout << "Free data:" << _tag;
	}
}

matrix matrix::operator+(matrix &b)
{
	size_t xlen = (b._size * b._size);
	float* xtemp1 = _data;
	float* xtemp2 = b._data;
	for (size_t i = 0; i < xlen; i++)
	{
		*xtemp1 =*xtemp1 + *xtemp2;
		xtemp1++;
		xtemp2++;
	}
	return matrix(_size, _data);
}

matrix matrix::operator-(matrix &b)
{
	size_t xlen = (b._size * b._size);
	float* xtemp1 = _data;
	float* xtemp2 = b._data;
	for (size_t i = 0; i < xlen; i++)
	{
		*xtemp1 = *xtemp1 - *xtemp2;
		xtemp1++;
		xtemp2++;
	}
	return matrix(_size, _data);
}

matrix matrix::operator*(matrix &b)
{
	size_t xlen = (b._size * b._size);
	float* xtemp1 = _data;
	float* xtemp2 = b._data;
	for (size_t i = 0; i < xlen; i++)
	{
		*xtemp1 = *xtemp1 - *xtemp2;
		xtemp1++;
		xtemp2++;
	}
	return matrix(_size, _data);
}

matrix matrix::operator=(matrix &b)
{
	matrix temp = matrix(b._size, b._data);
	return temp;
}

matrix matrix::dot(matrix &a, matrix &b)
{
	matrix temp = matrix(b._size, b._data);
	return temp;
}

matrix matrix::scalar(const float a, matrix& b)
{
	matrix temp = matrix(b._size, b._data);
	return temp;
}

void matrix::print()
{
	size_t xlen = (_size * _size);
	std::cout << _tag << ":";
	for (size_t i = 0; i < xlen; i++)
	{
		std::cout << *(_data+i);
	}
}

void matrix::set_tag(std::string _ptag)
{
	_tag = _ptag;
}

