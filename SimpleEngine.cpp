// SimpleEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "matrix.h"
#include <conio.h>
#include "Renderer.h"

int main()
{
    matrix m1 = matrix(4,new float [] { 1.0,2.0,3.0,4.0,1.0,2.0,3.0,4.0,1.0,2.0,3.0,4.0,1.0,2.0,3.0,4.0 });
    m1.set_tag("M1");
    m1.print();
    matrix m2 = matrix(4, new float [] { 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0 });
    m2.set_tag("M2");
    m2.print();
    matrix m3 = m2 + m1;
    m3.set_tag("M3");
    m3.print();

    Renderer r = Renderer();
    _getch();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
