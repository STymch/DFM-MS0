// ConsoleApplication.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#include <algorithm>
#include <iostream>
#include <vector>

int main() {
	std::vector<int> vect = { 1, 2, 3 };
	std::transform(vect.begin(), vect.end(), vect.begin(),
		[](int value) { return value + 1; });
	for (int value : vect) {
		std::cout << value << std::endl;
	}
	
	getchar();
	
	return 0;
}
