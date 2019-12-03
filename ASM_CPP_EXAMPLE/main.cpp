#include <iostream>

using namespace std;

extern "C" {
	int gcd( int, int );   // Return the greatest common divisor
	int negate( int );     // Return the negative of the argument
	int mask_bits( int );  // Return argument with certain bits masked
	void mania();
}



int main()
{
	mania();
	cout << negate( 5 ) << endl;
	cout << mask_bits( 52323402 ) << endl;
	cout << gcd( 31*27*10, 17*31 ) << endl;
	// drawGridPoint(1, 2, 1);
	return 0;
}
