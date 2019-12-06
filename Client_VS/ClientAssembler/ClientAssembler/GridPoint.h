#pragma once
using namespace std;

//GridPoint class
// - isPath
// - isConquered
// - isPlayerPos
// - colorType (1 - blue (b), 2 - green(g), 3 - light blue (b,g), 4 - red (r), 5 - purple (b,r), 6 - yellow (g,r), 7 - white (r,g,b))

class GridPoint
{
private:
	bool isPath;		//Keeps track of whether grid point has been conquered
	bool isConquered;	//Keeps track of whether grid point is currently part of an unfinalized path
	bool isPlayerPos;	//Keeps track wether current gridpoint stores a player's position
	int ownerID;

public:

	GridPoint(bool conquered, bool path, bool playerPos, int ownerID);
	bool getIsConquered();
	bool getIsPath();
	bool getIsPlayerPos();
	int getOwnerId();

	void setIsConquered(bool conquered);
	void setIsPath(bool path);
	void setIsPlayerPos(bool playerPos);
	void setOwnerId(int ownerID);

	//Color Determining Functions
	bool isRed();
	bool isGreen();
	bool isBlue();
	bool isBright();
};