#include "Sudoku.h"

#include <ctime>

int main(int argc, char* argv[])
{
	Sudoku sdk;
	sdk.SolveProblemSetBySearch(argv[1]);
	sdk.SolveProblemSetBySAT(argv[1]);

	return 1;
}