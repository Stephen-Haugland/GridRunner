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
			colorGrid[i].push_back(7);
		}
	}

	//TODO:
	//GridPoint class
	// - isPath
	// - isConquered
	// - isPlayerPos
	// - colorType (1 - blue (b), 2 - green(g), 3 - light blue (b,g), 4 - red (r), 5 - purple (b,r), 6 - yellow (g,r), 7 - white (r,g,b))


}

void Display::DrawGrid()
{
	// Windows console
	// https://www.dreamincode.net/forums/topic/228811-microsoft-using-console-functions-to-achieve-blinking-text/

	//Clear the console

	int curX = 0;
	int curY = 0;
	bool isDrawing = true;
	while (isDrawing)
	{
		//Assembly for next grid point and isDrawing state
		__asm 
		{
			// check if at the end of the grid
				mov eax, width
				cmp eax, curX
				jne nextGridPointCalc

				mov eax, height
				cmp eax, curY
				jne nextGridPointCalc

				//if at end set to stop drawing
				mov isDrawing, 0

			//check if at the end of row
			nextGridPointCalc:
				mov eax, width
				cmp eax, curX
				je endOfRowCalc
				jmp nextCellCalc

				//set x = 0 and add to increment if at end
				endOfRowCalc:
					inc curY
					mov curX, 0

				//add to x if not at end
				nextCellCalc:
					inc curX
		}

		//C++ goto function (x multiplied by two since our grid point is two space charecters
		SetCursorPosition(curX * 2, curY);

		//C++ set color functions (TODO: get proper color from grid point variable)
		//SetDrawColor();

		//C++ write text (a singular grid point)
		std::cout << "  " << std::endl;
	}
}

//[UTILITY DRAWING FUNCTIONS C++]
void Display::SetDrawColor(bool r, bool g, bool b, bool isBright)
{
	int finalOption = 0;

	//Assembly to figure out final option number:
	// - if need to include blue add 16 (since 0x0010 in hex is 16 in decimal)
	// - if need to include green add 32 (since 0x0020 in hex is 32 in decimal)
	// - if need to include red add 64 (since 0x0040 in hex is 64 in decimal)
	// - if needs to be bright also add 128 (since 0x0080 in hex is 128 in decimal)
	// - combinations of colors and brightness allow for differnt looks (all colors includesd results in white)

	__asm
	{
		//check if need to add red to the mix 
		mov eax, r
		cmp eax, 1
		jne addGreenCalc
		add finalOption, 64d

		//check if need to add green to the mix 
		addGreenCalc:
			mov eax, g
			cmp eax, 1
			jne addBlueCalc
			add finalOption, 32d
			
		//check if need to add blue to the mix 
		addBlueCalc:
			mov eax, b
			cmp eax, 1
			jne addIntensityCalc
			add finalOption, 16d

		//check if need to add intensity to the mix 
		addIntensityCalc:
			mov eax, isBright
			cmp eax, 1
			jne endOfSelection
			add finalOption, 128d

		endOfSelection:
	}
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, finalOption);
}

//our function to set the cursor position.
void Display::SetCursorPosition(short CoordX, short CoordY)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD position = { CoordX,CoordY };

	SetConsoleCursorPosition(hStdout, position);
}
