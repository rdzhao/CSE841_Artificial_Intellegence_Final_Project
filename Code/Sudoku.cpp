#include "Sudoku.h"

Sudoku::Sudoku(std::string fileName)
{
	std::fstream file;
	file.open(fileName.c_str(), std::ios::in);

	std::string s;
	int nr, nc;
	nr = 0;
	while (getline(file, s))
	{
		std::stringstream ss(s);
		int val;
		std::vector<int> tmpv;
		nc = 0;
		while (!ss.eof())
		{
			ss >> val;
			tmpv.push_back(val);
			
			++nc;
			if (nc > 9)
			{
				std::cout << "ERROR: #column > 9" << std::endl;
				exit(0);
			}
		}
		_chart.push_back(tmpv);

		++nr;
		if (nr > 9)
		{
			std::cout << "ERROR: #row > 9" << std::endl;
			exit(0);
		}
	}

	file.close();
}

int Sudoku::GetValue(int r, int c, std::vector<std::vector<int> > sol)
{
	return sol[r][c];
}

int Sudoku::SetValue(int r, int c, int val, std::vector<std::vector<int> >& sol)
{
	return sol[r][c] = val;
}

int Sudoku::GetValueByBlock(int br, int bc, int sr, int sc, std::vector<std::vector<int> > sol)
{
	return sol[br * 3 + sr][bc * 3 + sc];
}

int Sudoku::SolveProblemSetBySearch(string fileName)
{
	fstream file;
	file.open(fileName.c_str(), std::ios::in);
	fstream statFile;
	statFile.open("TimeStat.txt", std::ios::out);

	int count = 0;
	std::string s;
	while (getline(file, s))
	{
		// assemble _chart.
		std::vector<std::vector<int> > sol;
		for (int i = 0; i < 9; ++i)
		{
			std::vector<int> tmp;
			tmp.resize(9);
			sol.push_back(tmp);
		}

		for (int i = 0; i < s.size(); ++i)
			sol[i / 9][i % 9] = static_cast<int>(s[i]) - 48;
		
		//cout << "Execution time: " << (end - start) / double(CLOCKS_PER_SEC)*1000 << endl;

		//cout << "###########################################"<< endl;
		//cout << "Problem: "<< endl;
		//for (int i = 0; i < sol.size(); ++i)
		//{
		//	for (int j = 0; j < sol[0].size(); ++j)
		//		std::cout << sol[i][j] << " ";
		//	std::cout << std::endl;
		//}

		int start = clock();
		Recursion(sol, Simple, 0);
		int end = clock();
		//cout << "Solution: "<< endl;
		//for (int i = 0; i < _chart.size(); ++i)
		//{
		//	for (int j = 0; j < _chart[0].size(); ++j)
		//		std::cout << _chart[i][j] << " ";
		//	std::cout << std::endl;
		//}
		cout << "Execution time: " << (end - start) / double(CLOCKS_PER_SEC) * 1000 << " " << count << endl;
		//cout << endl;
		statFile << (end - start) / double(CLOCKS_PER_SEC) * 1000 << endl;

		++count;
	}

	file.close();
	statFile.close();

	return 1;
}

int Sudoku::SolveProblemSetBySAT(string fileName)
{
	// init chart
	for (int i = 0; i < 9; ++i)
	{
		std::vector<int> tmp;
		tmp.resize(9);
		_chart.push_back(tmp);
	}
	

	fstream file;
	file.open(fileName.c_str(), std::ios::in);
	fstream statFile;
	statFile.open("TimeStat.txt", std::ios::out);

	std::string s;
	while (getline(file, s))
	{
		// assemble _chart.
		for (int i = 0; i < s.size(); ++i)
			_chart[i / 9][i % 9] = static_cast<int>(s[i]) - 48;
	
		//cout << "********************************************"<< endl;
		//cout << "Problem: "<< endl;
		//for (int i = 0; i < _chart.size(); ++i)
		//{
		//	for (int j = 0; j < _chart[0].size(); ++j)
		//		std::cout << _chart[i][j] << " ";
		//	std::cout << std::endl;
		//}

		togasat::Solver solver;

		AddConstraintClausesOnEachEntry(solver);
		AddConstraintClausesOnRow(solver);
		AddConstraintClausesOnColumn(solver);
		AddConstraintClausesOnBlock(solver);

		AddAssignmentByProblem(solver);

		int start = clock();
		togasat::lbool status = solver.solve();
		int end = clock();

		//cout << "Satisifiable? : " << status << endl;
		cout << "Execution time: " << (end - start) / double(CLOCKS_PER_SEC) * 1000 << endl;
		statFile << (end - start) / double(CLOCKS_PER_SEC) * 1000 << endl;

		ExtractSolution(solver);
	
		//cout << "Solution: "<< endl;
		//for (int i = 0; i < _chart.size(); ++i)
		//{
		//	for (int j = 0; j < _chart[0].size(); ++j)
		//		std::cout << _chart[i][j] << " ";
		//	std::cout << std::endl;
		//}
	}

	file.close();
	statFile.close();

	return 1;
}

int Sudoku::Solve(Heuristic h)
{
	std::vector<std::vector<int> > sol;
	sol = _chart;

	Recursion(sol, h, 0);

	return 1;
}

int Sudoku::Recursion(std::vector<std::vector<int> > sol, Heuristic h, int execTime)
{
	if (execTime > 100000)
	{
		return 0;
	}

	int start = clock();

	int r, c;
	std::vector<int> potentials;
	bool success = SelectNextBlank(r, c, h, sol); // if already filled, return false

	if (!success) //solution is ready
	{
		_chart.clear();
		_chart = sol;
	}
	else
	{
		std::vector<int> potentials;
		GetPotentialValidValue(potentials, r, c, sol);
		//cout << potentials.size() << endl;

		for (int i = 0; i < potentials.size(); ++i)
		{
			SetValue(r, c, potentials[i], sol);
			int end = clock();
			int time = (end - start) / double(CLOCKS_PER_SEC) * 1000;
			Recursion(sol, h, execTime + time);
			SetValue(r, c, 0, sol);
		}
	}

	return 1;
}

int Sudoku::GetPotentialValidValue(std::vector<int>& potentials, int r, int c, std::vector<std::vector<int> > sol)
{
	potentials.clear();

	std::map<int, bool> valid;
	for (int i = 0; i < 9; ++i)
		valid[i + 1] = true;
	
	// horizontal
	for (int i = 0; i < 9; ++i)
	{
		valid[sol[r][i]] = false;
		valid[sol[i][c]] = false;
	}

	// block
	int br, bc;
	br = r / 3;
	bc = c / 3;
	
	for (int i = 0; i < 9; ++i)
	{
		int sr, sc;
		sr = i / 3;
		sc = i % 3;

		int val;
		val = GetValueByBlock(br, bc, sr, sc, sol);
		valid[val] = false;
	}

	for (int i = 0; i < 9; ++i)
		if (valid[i + 1])
			potentials.push_back(i + 1);

	return 1;
}

bool Sudoku::SelectNextBlank(int& r, int& c, Heuristic h, std::vector<std::vector<int> > sol)
{
	bool found = false;

	if (h == Simple)
	{
		for (int i = 0; i < 9; ++i)
		{
			for (int j = 0; j < 9; ++j)
			{
				if (sol[i][j] == 0)
				{
					r = i;
					c = j;
					found = true;

					break;
				}
			}

			if (found)
				break;
		}
	}

	if (h == MRV)
	{
		int tr, tc;
		std::vector<int> tp;
		for (int i = 0; i < 9; ++i)
			for (int j = 0; j < 9; ++j)
			{
				if (sol[i][j] == 0)
				{
					std::vector<int> p;
					GetPotentialValidValue(p, i, j, sol);

					if (!found)
					{
						tr = i;
						tc = j;
						tp = p;

						found = true;
					}
					else if(tp.size() > p.size())
					{
						tr = i;
						tc = j;
						tp = p;
					}
				}
			}

		r = tr;
		c = tc;
	}

	return found;
}

int Sudoku::BuildAndSolveSAT()
{
	//for (int i = 0; i < _chart.size(); ++i)
	//{
	//	for (int j = 0; j < _chart[0].size(); ++j)
	//		std::cout << _chart[i][j] << " ";
	//	std::cout << std::endl;
	//}

	// there are 729 variables and 11644 clauses.
	togasat::Solver solver;
	
	AddConstraintClausesOnEachEntry(solver);
	AddConstraintClausesOnRow(solver);
	AddConstraintClausesOnColumn(solver);
	AddConstraintClausesOnBlock(solver);

	AddAssignmentByProblem(solver);

	int start = clock();
	togasat::lbool status = solver.solve();
	int end = clock();

	cout << "Satisifiable? : " << status << endl;
	cout << "Execution time: " << (end - start) / double(CLOCKS_PER_SEC) * 1000 << endl;

	ExtractSolution(solver);

	//for (int i = 0; i < _chart.size(); ++i)
	//{
	//	for (int j = 0; j < _chart[0].size(); ++j)
	//		std::cout << _chart[i][j] << " ";
	//	std::cout << std::endl;
	//}

	return 1;
}

int Sudoku::AddConstraintClausesOnEachEntry(togasat::Solver& solver)
{
	std::vector<int> clause;

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
		{
			clause.clear();

			for (int k = 0; k < 9; ++k)
				clause.push_back(81 * i + 9 * j + k + 1);

			solver.addClause(clause);
		}

	//for(int i = 0; i < 9; ++i)
	//	for (int j = 0; j < 9; ++j)
	//		for (int k = 0; k < 9; ++k)
	//			for (int l = k + 1; l < 9; ++l)
	//			{
	//				clause.clear();

	//				clause.push_back(-(81 * i + 9 * j + k + 1));
	//				clause.push_back(-(81 * i + 9 * j + l + 1));
	//				
	//				solver.addClause(clause);
	//			}

	return 1;
}

int Sudoku::AddConstraintClausesOnRow(togasat::Solver& solver)
{
	std::vector<int> clause;

	for(int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			for (int k = 0; k < 9; ++k)
				for (int l = k + 1; l < 9; ++l)
				{
					clause.clear();

					clause.push_back(-(81 * k + 9 * i + j + 1));
					clause.push_back(-(81 * l + 9 * i + j + 1));
					
					solver.addClause(clause);
				}

	//for (int i = 0; i < 9; ++i)
	//	for (int j = 0; j < 9; ++j)
	//	{
	//		clause.clear();

	//		for (int k = 0; k < 9; ++k)
	//			clause.push_back(81 * k + 9 * i + j + 1);

	//		solver.addClause(clause);
	//	}

	return 1;
}

int Sudoku::AddConstraintClausesOnColumn(togasat::Solver& solver)
{
	std::vector<int> clause;

	for (int i = 0; i < 9; ++i)
		for (int j = 0; j < 9; ++j)
			for (int k = 0; k < 9; ++k)
				for (int l = k + 1; l < 9; ++l)
				{
					clause.clear();

					clause.push_back(-(81 * i + 9 * k + j + 1));
					clause.push_back(-(81 * i + 9 * l + j + 1));

					solver.addClause(clause);
				}

	//for (int i = 0; i < 9; ++i)
	//	for (int j = 0; j < 9; ++j)
	//	{
	//		clause.clear();

	//		for (int k = 0; k < 9; ++k)
	//			clause.push_back(81 * i + 9 * k + j + 1);

	//		solver.addClause(clause);
	//	}

	return 1;
}

int Sudoku::AddConstraintClausesOnBlock(togasat::Solver& solver)
{
	std::vector<int> clause;

	for (int k = 0; k < 9; ++k)
	{
		for(int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
			{
				for(int p = 0; p < 3; ++p)
					for (int q = 0; q < 3; ++q)
						for (int r = q + 1; r < 3; ++r)
						{
							clause.clear();

							clause.push_back(-(81 * (3 * i + p) + 9 * (3 * j + q) + k + 1));
							clause.push_back(-(81 * (3 * i + p) + 9 * (3 * j + r) + k + 1));

							solver.addClause(clause);
						}
			}
	}

	for (int k = 0; k < 9; ++k)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
			{
				for (int p = 0; p < 3; ++p)
					for (int q = 0; q < 3; ++q)
						for (int r = p + 1; r < 3; ++r)
							for (int s = 0; s < 3; ++s)
							{
								clause.clear();

								clause.push_back(-(81 * (3 * i + p) + 9 * (3 * j + q) + k + 1));
								clause.push_back(-(81 * (3 * i + r) + 9 * (3 * j + s) + k + 1));

								solver.addClause(clause);
							}
			}
	}

	//for(int i = 0; i < 3; ++i)
	//	for (int j = 0; j < 3; ++j)
	//	{
	//		for (int p = 0; p < 3; ++p)
	//			for (int q = 0; q < 3; ++q)
	//			{
	//				clause.clear();

	//				for (int k = 0; k < 9; ++k)
	//					clause.push_back(81 * (3 * i + p) + 9 * (3 * j + q) + k + 1);

	//				solver.addClause(clause);
	//			}
	//	}

	return 1;
}

int Sudoku::AddAssignmentByProblem(togasat::Solver& solver)
{
	for(int i = 0; i < _chart.size(); ++i)
		for (int j = 0; j < _chart[0].size(); ++j)
		{
			if (_chart[i][j] != 0)
			{
				int val = _chart[i][j];

				solver.assigns[81 * i + 9 * j + val - 1] = 0;
			}
		}

	return 1;
}

int Sudoku::ExtractSolution(togasat::Solver& solver)
{
	for (int i = 0; i < solver.assigns.size(); i+=9)
	{
		int r, c;
		r = i / 81;
		c = (i - 81 * r) / 9;

		for (int k = 0; k < 9; ++k)
		{
			//cout << solver.assigns[i + k] << " ";

			if (solver.assigns[i + k] == 0)
			{
				_chart[r][c] = k + 1;
			}
		}
		//cout << endl;

	}

	return 1;
}