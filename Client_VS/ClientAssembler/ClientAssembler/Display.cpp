#include "Display.h"

// http://kipirvine.com/asm/gettingStartedVS2019/index.htm


Display::Display(int x, int y) {
	width = x;
	height = y;
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			colorGrid[i].push_back(15);
		}
	}
}

extern "C" void drawGridPoint();

void Display::DrawGrid()
{
	// iterate through entire grid
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {

			// in line assembly to pass grid data to necessary registers
			__asm {
				// set the x coordinate to the current column
				mov dh,BYTE PTR i
				// set the y coordinate to the current row
				mov dl,BYTE PTR j

				// passes color to eax
				mov eax, DWORD PTR colorGrid[i][j]

			}

			// call predefined 
			drawGridPoint();

		}
	}

}
