#pragma once
#include "globals.h"
#include <array>
#include <vector>
#include <bitset>
#include <memory>
#include <algorithm>

constexpr auto MAX_COLS = GRID_SIZE * GRID_SIZE * 4;
constexpr auto GRID_SQUARED = GRID_SIZE * GRID_SIZE;
constexpr auto GRID_CUBED = GRID_SIZE * GRID_SIZE * GRID_SIZE;

using namespace std;

class Node {
public:
	Node();
	shared_ptr<Node> head, u, d, l, r;
	int size;
	array<int, 3> rowEl;
};


typedef std::shared_ptr<Node> sptr;

class DancingLinks
{
public:
	DancingLinks();
	~DancingLinks();
	bool Solve(PUZZLE_GRID& sudoku);
private:
	vector<vector<bool>> cover;
	Node* HeadNode;

	void initializeCoverMatrix();
	void initializeNodeLinks();
	void coverCol(const sptr& col);
	void uncoverCol(const sptr& col);
	void coverGivenNumber(int i, int j, PUZZLE_GRID&);
	bool search(int, PUZZLE_GRID&);
};