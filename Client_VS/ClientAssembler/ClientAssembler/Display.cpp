#include "Display.h"

// http://kipirvine.com/asm/gettingStartedVS2019/index.htm

extern "C" {
	void drawGridPoint( int , int , int );
}

Display::Display(int x, int y) {
	width = x;
	height = y;
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			colorGrid[i].push_back(15);
		}
	}
}

void Display::DrawGrid()
{
	// Windows console
	// https://www.dreamincode.net/forums/topic/228811-microsoft-using-console-functions-to-achieve-blinking-text/





	// iterate through entire grid
	for (int i = 0; i < width; i++) 
	{
		for (int j = 0; j < height; j++) 
		{

			// in line assembly to pass grid data to necessary registers
			//__asm 
			//{
			//	// set the x coordinate to the current column
			//	mov dh,BYTE PTR i
			//	// set the y coordinate to the current row
			//	mov dl,BYTE PTR j

			//	// passes color to eax
			//	mov eax, DWORD PTR colorGrid[i][j]

			//}

			// call predefined 
			//drawGridPoint(1, 2, 3);
			//drawGridPoint(i,j, colorGrid[i][j]);
		}
	}
}
