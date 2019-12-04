#include "Display.h"

Display::Display(int x, int y) {
	width = x;
	height = y;
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			colorGrid[i].push_back(new GridPoint(false, false, false, -1));
		}
	}
}

Display::~Display()
{
	for (int i = 0; i < width; i++) 
	{
		for (int j = 0; j < height; j++) 
		{
			delete(colorGrid[i][j]);
		}
	}
}

void Display::DrawGrid()
{
	// Windows console
	// https://www.dreamincode.net/forums/topic/228811-microsoft-using-console-functions-to-achieve-blinking-text/

	//Clear the console
	Clear();

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

		//C++ set color functions
		SetDrawColor((*colorGrid[curX][curY]).isRed(),
					 (*colorGrid[curX][curY]).isGreen(),
					 (*colorGrid[curX][curY]).isBlue(),
					 (*colorGrid[curX][curY]).isBright());

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
		mov eax, DWORD PTR r
		cmp eax, 1
		jne addGreenCalc
		add finalOption, 64d

		//check if need to add green to the mix 
		addGreenCalc:
			mov eax, DWORD PTR g
			cmp eax, 1
			jne addBlueCalc
			add finalOption, 32d
			
		//check if need to add blue to the mix 
		addBlueCalc:
			mov eax, DWORD PTR b
			cmp eax, 1
			jne addIntensityCalc
			add finalOption, 16d

		//check if need to add intensity to the mix 
		addIntensityCalc:
			mov eax, DWORD PTR isBright
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

//Reference: following function is completely from the folloing source
//https://stackoverflow.com/questions/6486289/how-can-i-clear-console
void Display::Clear()
{
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}