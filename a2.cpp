#include "a2.hpp"

using namespace std;

const double UCTK = 1.2;

const double DFS_EXPLORATION_RATE = 0.3;

static random_device rd;
static mt19937 gen(rd());
static uniform_real_distribution<double> uni_d(0.0, 1.0);

struct MOVE
{
    int arr[3]; // index_from, index_to, flag
};

struct TREE_NODE
{
	TREE_NODE* child;
	TREE_NODE* sibling;
	TREE_NODE* best_child;
	int visits;
	int rewards;
	MOVE tmove;
};

struct STATE
{
	int* board;
	vector<MOVE> moves;
	int curr_color;
	unsigned int curr_key;
};

MOVE stringToMove(string move_string);
string moveToString(MOVE move);



// General Globals
int n = 0; // size of board
int* board = NULL; //board
int** neighbors = NULL;
int color_to_play = BLACK;
double total_time = 10.0;
double time_left = 10.0;
bool timer_on = false;
int maxpoint = 0;

//Globals for UCT
STATE clonestate;
// int random_result = 0;
// int min_stone;
// int best_stone_couldbe;
// bool best_reached = false;
// vector<MOVE> best_sequence;
// vector<MOVE> curr_sequence;

void UCTSearch(TREE_NODE& root){
    // int** board = init_state.board;
	int* curr_board = new int[maxpoint];
	for(int i = 0; i < maxpoint; i++){
		curr_board[i] = board[i];
	}

    clonestate.board = curr_board;
    // clonestate.moves = init_state.moves;
    clonestate.curr_color = color_to_play;
    // clonestate.curr_key = init_state.curr_key;

	// curr_sequence.clear();

	// playSimulation(root, h, w);

	delete [] curr_board;

} 

//MOVE genMove(int* board, int color_to_play){
//
//}

void playMove(int* curr_board, MOVE move, int& color_to_play){
	int index_from = move.arr[0];
	int index_to = move.arr[1];

	assert(curr_board[index_from] == color_to_play);
	assert(curr_board[index_to] == oppositeColor(color_to_play));

	curr_board[index_to] = curr_board[index_from];
	curr_board[index_from] = EMPTY;

	color_to_play = oppositeColor(color_to_play);
}

MOVE stringToMove(string move_string){
	char col_from, col_to;
	int row_from, row_to;

	istringstream iss(move_string);
	iss >> col_from;
	iss >> row_from;
	iss >> col_to;
	iss >> row_to;

	assert(board != NULL && n > 0);
	assert(row_from >= 1 && row_from <= n);
	assert(row_to >= 1 && row_to <= n);
	assert(col_from >= 'a' && col_from <= ('a'+n-1));
	assert(col_to >= 'a' && col_to <= ('a'+n-1));

	// cerr << "stringtomove " << col_from << row_from << col_to << row_to << endl;

	int index_from = row_from * (n+1) + (int)(col_from - 'a');
	int index_to = row_to * (n+1) + (int)(col_to - 'a');

	MOVE move;
	move.arr[0] = index_from;
	move.arr[1] = index_to;
	move.arr[2] = false;

	return move;
}

string moveToString(MOVE move){
	int index_from = move.arr[0];
	int index_to = move.arr[1];

	char col_from, col_to;
	int row_from, row_to;

	assert(board != NULL && n > 0);

	row_from = index_from / (n+1);
	col_from = (char)(index_from % (n+1) + 'a');
	row_to = index_to / (n+1);
	col_to = (char)(index_to % (n+1) + 'a');

	assert(row_from >= 1 && row_from <= n);
	assert(row_to >= 1 && row_to <= n);
	assert(col_from >= 'a' && col_from <= ('a'+n-1));
	assert(col_to >= 'a' && col_to <= ('a'+n-1));

	// cerr << "movetostring " << col_from << row_from << col_to << row_to << endl;

	string s = col_from + to_string(row_from) + col_to + to_string(row_to);

	return s;
}

int main(int argc, char* argv[])
{
	string command;

	string moveToPlay;

	unsigned int*** random_table = NULL;
	unsigned int init_key = gen();

	double time_limit = 0.0;
	for (string line; getline(cin, line);){
		if (line.compare(0, 9, "boardsize") == 0){
			istringstream iss(line);
			iss >> command;
			iss >> n;
			maxpoint = n * n + 3 * (n + 1) - 1;
            // random_table = new unsigned int**[h];
            // for(int i = 0; i < h; i++){
            //     random_table[i] = new unsigned int*[w];
            //     for(int j = 0; j < w; j++){
            //         random_table[i][j] = new unsigned int[2];
            //         for(int k = 0; k < 2; k++){
            //             random_table[i][j][k] = gen();
            //         }
            //     }
            // }
			assert(n >= 3 && n <= 10);

			board = new int[maxpoint];
			neighbors = new int*[maxpoint];

    		// Generate board
			for (int i = 0; i < maxpoint; i++){
				int row_number = i / (n+1);
				int col_number = i % (n+1) + 1;

				if (col_number == n + 1){
					board[i] = OFFBOARD;
				} else if (row_number == 0 || row_number == n+1){
					board[i] = OFFBOARD;
				} else if (row_number % 2 == 1){
					if (col_number % 2 == 1){
                		// odd row odd col
						board[i] = WHITE;
					} else if (col_number % 2 == 0){
                		// odd row even col
						board[i] = BLACK;
					}
				} else if (row_number % 2 == 0){
					if (col_number % 2 == 1){
                		// even row odd col
						board[i] = BLACK;
					} else if (col_number % 2 == 0){
                		// even row even col
						board[i] = WHITE;
					}
				}
			}

            // Precompute neighbors
			for (int i = 0; i < maxpoint; i++){
				neighbors[i] = new int[4];

				int we = 1;
				int ns = n + 1;
				int row_number = i / (n+1);
				int col_number = i % (n+1) + 1;

				for (int j = 0; j < 4; j++){
					if(col_number == n+1){
						neighbors[i][j] = 0;
						continue;
					} else if(row_number == 0 || row_number == n+1){
						neighbors[i][j] = 0;
						continue;
					} 

					if(j == 0){
						if (row_number == 1){
							neighbors[i][j] = 0;
						} else {
							neighbors[i][j] = i - ns;
						}
					} else if (j == 1){
						if (col_number == n){
							neighbors[i][j] = 0;
						} else {
							neighbors[i][j] = i + we;
						}
					} else if (j == 2){
						if (row_number == n){
							neighbors[i][j] = 0;
						} else {
							neighbors[i][j] = i + ns;
						}
					} else if (j == 3){
						if (col_number == 1){
							neighbors[i][j] = 0;
						} else {
							neighbors[i][j] = i - we;
						}
					}
				}
			}

            // Sort neighbors
			for (int i = 0; i < maxpoint; i++){
				qsort (neighbors[i], 4, sizeof(int), compareInt);
			}


			for (int j = 0; j < maxpoint; j++){
				cerr << board[j] << endl;
			}
			for (int j = 0; j < maxpoint; j++){
				cerr << neighbors[j][0] << " " << neighbors[j][1] << " " << neighbors[j][2] << " " << neighbors[j][3] << endl;
			}

			cerr << "Board set. Size: " << n << endl << flush;
			cout << "=\n\n" << flush;
		} else if (line.compare(0, 7, "version") == 0){
			cout << "= 0.0.1\n\n" << flush;
		} else if (line.compare(0, 4, "name") == 0){
			cout << "= shida3\n\n" << flush;
		} else if (line.compare(0, 16, "protocol_version") == 0){
			cout << "= 2\n\n" << flush;
		} else if (line.compare(0, 9, "timelimit") == 0){
			istringstream iss(line);
			iss >> command;
			iss >> total_time;

			assert(total_time >= 10.0 && total_time <= 1000.0);
    		// setTimer(total_time);
			time_left = total_time;
			timer_on = true;

			cout << "=\n\n"<< flush;
		} else if (line.compare(0, 9, "showboard") == 0){
			if (board == NULL){
				cerr << "Board uninitialized." << endl << flush;
			} else {
				cout << "= \n" << showBoard(board, n) << flush;
			}
		} else if (line.compare(0, 7, "genmove") == 0){
			// MOVE move = genMove(board, color_to_play);
			// cout << "= " << moveToString(move) << "\n\n" << flush;
		} else if (line.compare(0, 4, "play") == 0){
			istringstream iss(line);
			iss >> command;
			iss >> moveToPlay;
			// cerr << moveToString(stringToMove(moveToPlay)) << endl;
			playMove(board, stringToMove(moveToPlay), color_to_play);

			cout << "=\n\n" << flush;
		}


	}



    // for(int i = 0; i < w; ++i) {
    //     delete [] board[i];
    // }
    // delete [] board;


	return 0;
}