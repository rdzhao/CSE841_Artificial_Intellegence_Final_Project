#pragma once

#ifndef _SUDOKU_RUNDONG_
#define _SUDOKU_RUNDONG_

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>

#include "CDCLSatSolver.hpp"

using namespace std;

enum Heuristic
{
	Simple,
	MRV,
};

class Sudoku
{
public:
	Sudoku() {};
	Sudoku(std::string fileName);
	~Sudoku() {};

	int Solve(Heuristic h);
	int BuildAndSolveSAT();

	int SolveProblemSetBySearch(string fileName);
	int SolveProblemSetBySAT(string fileName);

	int GetValue(int r, int c, std::vector<std::vector<int> > sol);
	int SetValue(int r, int c, int val, std::vector<std::vector<int> >& sol);
	int GetValueByBlock(int br, int bc, int sr, int sc, std::vector<std::vector<int> > sol); // block (br, bc)
	int GetPotentialValidValue(std::vector<int>& potentials, int r, int c, std::vector<std::vector<int> > sol);
	bool SelectNextBlank(int& r, int& c, Heuristic h, std::vector<std::vector<int> > sol);
	int Recursion(std::vector<std::vector<int> > sol, Heuristic h, int execTime);
	
	int AddConstraintClausesOnEachEntry(togasat::Solver& solver);
	int AddConstraintClausesOnRow(togasat::Solver& solver);
	int AddConstraintClausesOnColumn(togasat::Solver& solver);
	int AddConstraintClausesOnBlock(togasat::Solver& solver);
	int AddAssignmentByProblem(togasat::Solver& solver);
	int ExtractSolution(togasat::Solver& solver);

	std::vector<std::vector<int> > _chart; // 9*9; 0 means empty; 
};

#endif // ! _SUDOKU_RUNDONG_
