#include "DancingLinks.h"
#include "globals.h"
#include <vector>

using namespace std;

static constexpr int constexpr_sqrt_helper(int x, int low, int high) {
	return low > high ? high : ((low + high) / 2) * ((low + high) / 2) > x
		? constexpr_sqrt_helper(x, low, (low + high) / 2 - 1)
		: constexpr_sqrt_helper(x, (low + high) / 2 + 1, high);
}

static constexpr int constexpr_sqrt(int x) {
	return x < 0 ? -1 : constexpr_sqrt_helper(x, 0, x);
}

constexpr auto GRID_SQRT = constexpr_sqrt(GRID_SIZE);

Node::Node() {
	//Intentionally blank
}

DancingLinks::DancingLinks() : cover(GRID_CUBED, vector<bool>(MAX_COLS, false)), // Initialize cover
HeadNode(nullptr)                                      // Initialize HeadNode 
{
	initializeCoverMatrix();
	initializeNodeLinks();
}

DancingLinks::~DancingLinks() {
	//Intentionally blank
}

void DancingLinks::initializeCoverMatrix() {
	int counter = 0;
	int j = 0;
	for (int i = 0; i < GRID_CUBED; i++) {
		cover[i][j] = 1; //1 value per cell
		counter++;
		if (counter >= GRID_SIZE) {
			j++;
			counter = 0;
		}
	}

	counter = 1;
	int x = 0;
	for (j = GRID_SQUARED; j < 2 * GRID_SQUARED; j++) {
		for (int i = x; i < counter * GRID_SQUARED; i += GRID_SIZE) {
			cover[i][j] = 1; //only 1 instance in a row
		}
		if ((j + 1) % GRID_SIZE == 0) {
			x = counter * GRID_SQUARED;
			counter++;
		}
		else {
			x++;
		}
	}

	j = 2 * GRID_SQUARED;
	for (int i = 0; i < GRID_CUBED; i++) {
		cover[i][j] = 1; //only 1 instance per column
		j++;
		if (j == 3 * GRID_SQUARED) {
			j -= GRID_SQUARED;
		}
	}

	x = 0;
	for (j = 3 * GRID_SQUARED; j < MAX_COLS; j++) {
		for (int k = 0; k * k < GRID_SIZE; k++) {
			for (int l = 0; l * l < GRID_SIZE; l++) {
				cover[x + k * GRID_SIZE + l * GRID_SQUARED][j] = 1; //one per box
			}
		}
		x++;
		if ((j + 1) % GRID_SIZE == 0) {
			x += (GRID_SQRT - 1) * GRID_SIZE;
			if ((j + 1) % (GRID_SQRT * GRID_SIZE) == 0) {
				x += (GRID_SQRT - 1) * GRID_SQUARED;
			}
		}
	}
}

void DancingLinks::initializeNodeLinks() {
	sptr header = make_shared<Node>();
	header->l = header->r = header->u = header->d = header;
	header->size = -1;
	header->head = header;
	sptr prev = header;

	for (int i = 0; i < MAX_COLS; i++) {
		auto newCol = make_shared<Node>();
		newCol->size = 0;
		newCol->u = newCol->d = newCol;
		newCol->l = prev;
		newCol->r = header;
		newCol->head = newCol;
		prev->r = newCol;
		prev = newCol;

	}

	sptr colHead = make_shared<Node>();
	int rowEl[3] = { 0,0,0 };

	for (int i = 0; i < GRID_CUBED; i++) {
		colHead = header->r;
		prev = NULL;

		rowEl[0] = (rowEl[0]) % GRID_SIZE + 1;
		if (i % GRID_SIZE == 0) {
			rowEl[2] = (rowEl[2]) % GRID_SIZE + 1;
			if (i % (GRID_SQUARED) == 0)
				rowEl[1] = (rowEl[1]) % GRID_SIZE + 1;
		}

		for (int j = 0; j < MAX_COLS; j++, colHead = colHead->r) {
			if (cover[i][j]) {
				auto newNode = make_shared<Node>();

				newNode->rowEl[0] = rowEl[0];
				newNode->rowEl[1] = rowEl[1];
				newNode->rowEl[2] = rowEl[2];

				colHead->size++;
				newNode->head = colHead;

				if (prev == NULL) {
					prev = newNode;
					prev->r = newNode;
				}

				newNode->l = prev;
				newNode->r = prev->r;

				newNode->d = colHead;
				newNode->u = colHead->u;

				newNode->r->l = newNode;
				prev->r = newNode;

				colHead->u->d = newNode;
				colHead->u = newNode;

				prev = newNode;
			}
		}
	}
	HeadNode = header.get();
}

void DancingLinks::coverCol(const sptr& col) {
	col->l->r = col->r;
	col->r->l = col->l;//hide the column

	for (sptr nv = col->d; nv != col; nv = nv->d) {//vertical traversal
		for (sptr nh = nv->r; nh != nv; nh = nh->r) {//horizontal traversal
			nh->d->u = nh->u;
			nh->u->d = nh->d;
			nh->head->size--;
		}
	}
}
void DancingLinks::uncoverCol(const sptr& col) {
	col->l->r = col;
	col->r->l = col;//unhide the column
	for (sptr nv = col->u; nv != col; nv = nv->u) {//reverse vertical traversal
		for (sptr nh = nv->l; nh != nv; nh = nh->l) {//reverse horizontal traversal
			nh->d->u = nh;
			nh->u->d = nh;
			nh->head->size++;
		}
	}
}

bool DancingLinks::search(int source, PUZZLE_GRID& sudoku) {
	if (HeadNode->r.get() == HeadNode) {
		return true;
	}

	// Choose the column with the smallest size
	sptr col = HeadNode->r;
	for (sptr nh = col->r; nh.get() != HeadNode; nh = nh->r) {
		if (nh->size < col->size) {
			col = nh;
		}
	}

	coverCol(col);

	for (sptr nv = col->d; nv != col; nv = nv->d) {
		for (sptr nh = nv->r; nh != nv; nh = nh->r) {
			coverCol(nh->head);
		}

		if (search(source + 1, sudoku)) {
			sudoku[nv->rowEl[1] - 1][nv->rowEl[2] - 1] = nv->rowEl[0];
			return true;
		}

		for (sptr nh = nv->l; nh != nv; nh = nh->l) {
			uncoverCol(nh->head);
		}
	}

	uncoverCol(col);
	return false;
}

void DancingLinks::coverGivenNumber(int i, int j, PUZZLE_GRID& sudoku) {

	for (sptr nh = HeadNode->r; nh.get() != HeadNode; nh = nh->r) {
		for (sptr nv = nh->d; nv != nh; nv = nv->d) {
			if (nv->rowEl[0] == sudoku[i][j] && nv->rowEl[1] == i + 1 && nv->rowEl[2] == j + 1) {
				sptr nr = nv;
				do {
					coverCol(nr->head);
					nr = nr->r;
				} while (nr != nv);

				return;
			}
		}
	}
}

bool DancingLinks::Solve(PUZZLE_GRID& sudoku) {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			if (sudoku[i][j]) {
				coverGivenNumber(i, j, sudoku);
			}
		}
	}
	return search(0, sudoku);
}
