// ConsoleApplication.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#include <algorithm>
#include <iostream>
#include <vector>

extern	void TestPrintf();
int nMainCalls = 0;

int main() {
	++nMainCalls;

	std::vector<int> vect = { 1, 2, 3 };
	std::transform(vect.begin(), vect.end(), vect.begin(),
		[](int value) { return value + 1; });
	for (int value : vect) {
		std::cout << value << std::endl;
	}
	
	getchar();

	TestPrintf();

	if (nMainCalls < 2) main();

	return 0;
}

//#include <stdio.h>
//#include <conio.h>

void TestPrintf() {
	int a = 0x77, b = -20;
	char c = 'F';
	double f = 12.2341524;
	double d = 2e2;
	char* string = "Hello, World!";

	printf("%.3f\n", f);
	printf("%.*f\n", 2, f);
	printf("%10.3f\n", d);
	printf("%*d\n", 6, a);
	printf("%+d\n", b);
	printf("%6d\n", a);
	printf("%.f\n", d);
	printf("%.4s", string);

	getchar();
}