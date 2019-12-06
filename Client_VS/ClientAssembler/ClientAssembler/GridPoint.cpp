#include "GridPoint.h"
using namespace std;

GridPoint::GridPoint(bool conquered, bool path, bool playerPos, int ownerID) 
{
	isConquered = conquered;
	isPath = path;
	isPlayerPos = playerPos;
	this->ownerID = ownerID;
}

bool GridPoint::getIsConquered() { return isConquered; }
bool GridPoint::getIsPath() { return isPath; }
bool GridPoint::getIsPlayerPos() { return isPlayerPos; }
int GridPoint::getOwnerId() { return ownerID; }

void GridPoint::setIsConquered(bool conquered) { isConquered = conquered; }
void GridPoint::setIsPath(bool path) { isPath = path; }
void GridPoint::setIsPlayerPos(bool playerPos) { isPlayerPos = playerPos; }
void GridPoint::setOwnerId(int ownerID) 
{
	this->ownerID = ownerID; 
	if (ownerID == -1)
		isConquered = false;
	else
		isConquered = true;
}

//Color Determining Functions (TODO: redo in assembler if enough time)
// - colorType (0 - blue (b), 1 - green(g), 2 - light blue (b,g), 3 - red (r), 4 - purple (b,r), 5 - yellow (g,r), (-1) - white (r,g,b))
bool GridPoint::isRed()
{
	int colorID = ownerID % 6;
	if (colorID == -1 || colorID == 3 || colorID == 4 || colorID == 5)
		return true;
	else
		return false;
}

bool GridPoint::isGreen()
{
	int colorID = ownerID % 6;
	if (colorID == -1 || colorID == 1 || colorID == 2 || colorID == 5)
		return true;
	else
		return false;
}

bool GridPoint::isBlue()
{
	int colorID = ownerID % 6;
	if (colorID == -1 || colorID == 0 || colorID == 2 || colorID == 4)
		return true;
	else
		return false;
}

bool GridPoint::isBright()
{
	if (isPlayerPos || isConquered)
		return true;
	else
		return false;
}