#include "a2.hpp"

using namespace std;


const double UCTK = 1.2;

static random_device rd;
static mt19937 gen(rd());
static uniform_int_distribution<int> uni_i(0, 10*10*4);


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
    int rewards; // wins
    MOVE tmove;
};


TREE_NODE& UCTSelect(TREE_NODE& n, int curr_color);
int playRandomGame(int* cloneboard, int clonecolor, unsigned int curr_key);
bool createChildren(TREE_NODE& n);
void updateReward(TREE_NODE& n, int random_result, int curr_color);
void setBest(TREE_NODE& n);
void setWorst(TREE_NODE& n);
void playSimulation(TREE_NODE& n);
void UCTSearch(TREE_NODE& root);
void pushPossibleMovesAt(int* board, int curr_color, int i, vector<MOVE>& moves);
vector<MOVE> findPossibleMoves(int* board, int curr_color);
MOVE& randomSelectAMove(vector<MOVE>& moves);
void moveRootToChild(MOVE& move);
MOVE genMove(int* board, int color_to_play);
MOVE genMoveRandom(int* board, int color_to_play);
void setTimer(double time_read);
void playMove(int* curr_board, MOVE& move, int& colortp, unsigned int& curr_key);
void unPlayMove(int* curr_board, MOVE& move, int& colortp, unsigned int& curr_key);
MOVE stringToMove(string move_string);
bool stringMoveIsLegal(string move_string);
bool moveIsLegal(MOVE move);
string moveToString(MOVE move);
void setBoardAndNeighbors();



// Globals
int n = 4; // size of board
int* board = NULL; //board
int** neighbors = NULL;
int color_to_play = BLACK;
double total_time = 10.0;
double time_left = 10.0;
bool timer_on = false;
int maxpoint = 0;
int moves_after_start = 0;
int my_color = EMPTY;

int* cloneboard = NULL;
int clonecolor = EMPTY;
int random_result = 0;
TREE_NODE* root = NULL;

unsigned int** random_table = NULL;
unsigned int init_key = gen();
unsigned int clonekey = init_key;
unordered_map<unsigned int, TREE_NODE> hash_table;



TREE_NODE& UCTSelect(TREE_NODE& n, int curr_color){
    
    double bestuct = 0.0;
    
    TREE_NODE& next = *(n.child);
    TREE_NODE& result = next;
    
    int epsilon = 4000;
    while (1){
        double uctvalue;
        if (next.visits > 0){
            double reward_rate = (double)next.rewards / (double)next.visits;
            double uct = UCTK * sqrt(log(n.visits) / (5*next.visits));
            uctvalue = reward_rate + uct;
        } else {
            uctvalue = epsilon + uni_i(gen);
        }
        
        if (uctvalue > bestuct){
            bestuct = uctvalue;
            result = next;
        }
        
        if(next.sibling == NULL){
            break;
        } else {
            next = *(next.sibling);
        }
    }
    return result;
}

int playRandomGame(int* cloneboard, int clonecolor, unsigned int curr_key){
    // play random game
    // return loser
    
    int* curr_board = cloneboard;
    int curr_color = clonecolor;
    while(1){
        vector<MOVE> curr_moves = findPossibleMoves(curr_board, curr_color);
        if(curr_moves.empty()){
            return curr_color;
        }
        MOVE move_todo = randomSelectAMove(curr_moves);
        playMove(curr_board, move_todo, curr_color, curr_key);
    }
    
} // TO IMPROVE

bool createChildren(TREE_NODE& n){
    
    int* curr_board = cloneboard;
    int curr_color = clonecolor;
    unsigned int curr_key = clonekey;
    
    vector<MOVE> n_moves = findPossibleMoves(curr_board, curr_color);
    if(n_moves.empty()){
        return true;
    }

    TREE_NODE* child_1 = new TREE_NODE;
    child_1->tmove = n_moves[0];
    child_1->child = NULL;
    child_1->sibling = NULL;
    child_1->best_child = NULL;
    child_1->visits = 0;
    child_1->rewards = 0;
    
    playMove(curr_board, n_moves[0], curr_color, curr_key);
    auto test_result = hash_table.insert(make_pair(curr_key, *child_1));
    if (test_result.second == false){
        child_1->visits = test_result.first->second.visits;
        child_1->rewards = test_result.first->second.rewards;
        child_1->child = test_result.first->second.child;
        child_1->best_child = test_result.first->second.best_child;
    }
    unPlayMove(curr_board, n_moves[0], curr_color, curr_key);
    
    n.child = child_1;
    
    TREE_NODE* child_prev = child_1;
    for (int i = 1; i < n_moves.size(); i++){
        TREE_NODE* child_next = new TREE_NODE;
        child_next->tmove = n_moves[i];
        child_next->child = NULL;
        child_next->sibling = NULL;
        child_next->best_child = NULL;
        child_next->visits = 0;
        child_next->rewards = 0;
        
        playMove(curr_board, n_moves[i], curr_color, curr_key);
        auto test_result = hash_table.insert(make_pair(curr_key,*child_next));
        if (test_result.second == false){
            child_next->visits = test_result.first->second.visits;
            child_next->rewards = test_result.first->second.rewards;
            child_next->child = test_result.first->second.child;
            child_next->best_child = test_result.first->second.best_child;
        }
        unPlayMove(curr_board, n_moves[i], curr_color, curr_key);
        
        child_prev->sibling = child_next;
        child_prev = child_next;
    }
    
    return false;
}

void updateReward(TREE_NODE& n, int random_result, int curr_color){
    if (random_result == oppositeColor(curr_color)){
        n.rewards += 1;
    }
}

void setBest(TREE_NODE& n){
    TREE_NODE& next = *(n.child);
    TREE_NODE& result = next;
    double best_reward_rate = 0.0;
    while (1){
        double reward_rate;
        if (next.visits > 0){
            reward_rate = (double)next.rewards / (double)next.visits;
        } else {
            reward_rate = 0.0;
        }
        
        if (reward_rate > best_reward_rate){
            best_reward_rate = reward_rate;
            result = next;
        }
        
        if(next.sibling == NULL){
            break;
        }
        next = *(next.sibling);
    }
    n.best_child = &result;
}

void setWorst(TREE_NODE& n){
    TREE_NODE& next = *(n.child);
    TREE_NODE& result = next;
    double best_reward_rate = 1.0;
    while (1){
        double reward_rate;
        if (next.visits > 0){
            reward_rate = (double)next.rewards / (double)next.visits;
        } else {
            reward_rate = 1.0;
        }
        
        if (reward_rate < best_reward_rate){
            best_reward_rate = reward_rate;
            result = next;
        }
        
        if(next.sibling == NULL){
            break;
        }
        next = *(next.sibling);
    }
    n.best_child = &result;
}

void playSimulation(TREE_NODE& n){
    bool terminal;
    int curr_color = clonecolor;
    if (n.visits == 0){
        random_result = playRandomGame(cloneboard, clonecolor, clonekey);
    } else {
        if (n.child == NULL){
            terminal = createChildren(n);
        }
        if(!terminal){
            TREE_NODE& next = UCTSelect(n, curr_color);
            
            playMove(cloneboard, next.tmove, clonecolor, clonekey);
            playSimulation(next);
        }
    }
    n.visits++;
    if (!terminal)
    updateReward(n, random_result, curr_color);
    
    if(n.child != NULL){
        if (curr_color == oppositeColor(my_color))
            setBest(n); // set n.best_child to the child with highest rewardrate
        else
            setWorst(n);
    }
}

void UCTSearch(TREE_NODE& root){
    if (cloneboard == NULL){
        cloneboard = new int[maxpoint];
    }
    
    for(int i = 0; i < maxpoint; i++){
        cloneboard[i] = board[i];
    }
    clonecolor = color_to_play;
    clonekey = init_key;

    playSimulation(root);
}

void pushPossibleMovesAt(int* board, int curr_color, int i, vector<MOVE>& moves){
    int curr = board[i];
    if (curr != curr_color){
        return;
    }
    int index_to;
    for (int d = 0; d < 4; d++){
        
        index_to = neighbors[i][d];
        
        if (index_to == 0){
            break;
        }
        if (board[index_to] != oppositeColor(curr)){
            continue;
        } else {
            MOVE m;
            m.arr[0] = i;
            m.arr[1] = index_to;
            m.arr[2] = false;
            moves.push_back(m);
        }
    }
}

vector<MOVE> findPossibleMoves(int* board, int curr_color){
    vector<MOVE> moves;
    for (int i = 0; i < maxpoint; i++){
        pushPossibleMovesAt(board, curr_color, i, moves);
    }
    return moves;
}

MOVE& randomSelectAMove(vector<MOVE>& moves){
    int i = uni_i(gen) % (moves.size());
    return moves.at(i);
}

MOVE genMove(int* board, int color_to_play){
    
    if (my_color == EMPTY){
        my_color = color_to_play;
    }
    
    // timer MACOS or LINUX
    chrono::high_resolution_clock::time_point start, end;
    start = chrono::high_resolution_clock::now();
    
    
    double p_inc = 1.0 / double(n * n);
    double p = 0.0 + p_inc * (moves_after_start + 1);
    
    double time_l = time_left * p;
    
    int iteration_count = 0;
    
    if (root == NULL){
        root = new TREE_NODE;
        root->child = NULL;
        root->sibling = NULL;
        root->best_child = NULL;
        root->visits = 0;
        root->rewards = 0;
    }
    
    double duration_double;
    
    while(1){
        
        UCTSearch(*root);
        
        cerr << "iteration " << iteration_count << endl;
        
        iteration_count++;
        
        // timer MACOS or LINUX
        end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration_seconds = end-start;
        duration_double = duration_seconds.count();
        if (duration_double >= time_l){
            time_left -= duration_double;
            assert(time_left >= 0.0);
            break;
        }
    }
    
    MOVE move;
    if (root->best_child == NULL){
        move.arr[0] = 0;
        move.arr[1] = 0;
        move.arr[2] = false;
    } else {
        // TO COMMENT
        int wins = root->rewards;
        int visits = root->visits;
        cerr << "Move after start: " << moves_after_start << "  Iterations: " << iteration_count << endl;
        cerr << "Time used: " << duration_double << "  Time left: " << time_left << endl;
        cerr << "Winrate: " << (double)wins/(double)visits << "  Wins: " << wins << "  Visits: " << visits << endl;
        // TO COMMENT
        
        move = root->best_child->tmove;
        root = root->best_child;

    }
    
    return move;
}

void moveRootToChild(MOVE& move){
    for(int i = 0; i < maxpoint; i++){
        cloneboard[i] = board[i];
    }
    clonecolor = color_to_play;
    clonekey = init_key;
    
    createChildren(*root);
    
    TREE_NODE* next = root->child;

    while (1){
        if (next->tmove.arr[0] == move.arr[0] && next->tmove.arr[1] == move.arr[1]){
            break;
        }
        if (next->sibling == NULL){
            cerr << "No suitable child to move root" << endl;
            break;
        }

        next = next->sibling;
    }
    
    root = next;
}

MOVE genMoveRandom(int* board, int color_to_play){
    vector<MOVE> moves = findPossibleMoves(board, color_to_play);
    MOVE move;
    if (moves.empty()){
        move.arr[0] = 0;
        move.arr[1] = 0;
        move.arr[2] = false;
    } else {
        move = randomSelectAMove(moves);
    }
    return move;
}

void setTimer(double time_read){
    total_time = time_read;
    time_left = total_time;
    timer_on = true;
}

void playMove(int* curr_board, MOVE& move, int& colortp, unsigned int& curr_key){
    int index_from = move.arr[0];
    int index_to = move.arr[1];
    
//    cerr << "index_from " << index_from << " index_to " << index_to << " colortp " << colortp << endl;
    assert(curr_board[index_from] == colortp);
    assert(curr_board[index_to] == oppositeColor(colortp));
    
    int sfrom = curr_board[index_from];
    int sto = curr_board[index_to];
    
    curr_key ^= random_table[index_from][sfrom];
    curr_key ^= random_table[index_from][EMPTY];
    curr_key ^= random_table[index_to][sto];
    curr_key ^= random_table[index_to][sfrom];
    
    curr_board[index_to] = sfrom;
    curr_board[index_from] = EMPTY;
    
    colortp = oppositeColor(colortp);
    
//    cerr << "Current color: " << color_to_play << endl;
}

void unPlayMove(int* curr_board, MOVE& move, int& colortp, unsigned int& curr_key){
    int index_from = move.arr[0];
    int index_to = move.arr[1];
    
    colortp = oppositeColor(colortp);
    
    assert(curr_board[index_from] == EMPTY);
    assert(curr_board[index_to] == colortp);
    
    int sfrom = colortp;
    int sto = oppositeColor(colortp);
    
    curr_key ^= random_table[index_from][sfrom];
    curr_key ^= random_table[index_from][EMPTY];
    curr_key ^= random_table[index_to][sto];
    curr_key ^= random_table[index_to][sfrom];
    
    curr_board[index_to] = sto;
    curr_board[index_from] = sfrom;
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
    
//    cerr << "stringtomove " << col_from << row_from << col_to << row_to << endl;
    
    int index_from = row_from * (n+1) + (int)(col_from - 'a');
    int index_to = row_to * (n+1) + (int)(col_to - 'a');
    
    MOVE move;
    move.arr[0] = index_from;
    move.arr[1] = index_to;
    move.arr[2] = false;
    
    return move;
}

bool stringMoveIsLegal(string move_string){
    char col_from, col_to;
    int row_from, row_to;
    
    istringstream iss(move_string);
    iss >> col_from;
    iss >> row_from;
    iss >> col_to;
    iss >> row_to;
    
    if (!(board != NULL && n > 0)){
        return false;
    }
    if(!(row_from >= 1 && row_from <= n)){
        return false;
    }
    if(!(row_to >= 1 && row_to <= n)){
        return false;
    }
    if(!(col_from >= 'a' && col_from <= ('a'+n-1))){
        return false;
    }
    if(!(col_to >= 'a' && col_to <= ('a'+n-1))){
        return false;
    }
    
    int index_from = row_from * (n+1) + (int)(col_from - 'a');
    int index_to = row_to * (n+1) + (int)(col_to - 'a');
    int we = 1;
    int ns = n + 1;
    
    if(!(index_from == index_to - we || index_from == index_to + we ||
         index_from == index_to + ns || index_from == index_to - ns)){
        return false;
    }
    if(!(board[index_from] == color_to_play && board[index_to] == oppositeColor(color_to_play))){
        return false;
    }
    
    return true;
}

bool moveIsLegal(MOVE move){
    int index_from = move.arr[0];
    int index_to = move.arr[1];
    
    int we = 1;
    int ns = n + 1;
    
    char col_from, col_to;
    int row_from, row_to;
    
    if (!(board != NULL && n > 0)){
        return false;
    }
    
    if(!(index_from == index_to - we || index_from == index_to + we ||
         index_from == index_to + ns || index_from == index_to - ns)){
        return false;
    }
    
    
    row_from = index_from / (n+1);
    col_from = (char)(index_from % (n+1) + 'a');
    row_to = index_to / (n+1);
    col_to = (char)(index_to % (n+1) + 'a');
    
    if(!(row_from >= 1 && row_from <= n)){
        return false;
    }
    if(!(row_to >= 1 && row_to <= n)){
        return false;
    }
    if(!(col_from >= 'a' && col_from <= ('a'+n-1))){
        return false;
    }
    if(!(col_to >= 'a' && col_to <= ('a'+n-1))){
        return false;
    }
    
    return true;
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
    
//    cerr << "movetostring " << col_from << row_from << col_to << row_to << endl;
    
    string s = col_from + to_string(row_from) + col_to + to_string(row_to);
    
    return s;
}

void setBoardAndNeighbors(){
    
    if(board != NULL){
        delete[] board;
        board = NULL;
    }
    if(cloneboard != NULL){
        delete[] cloneboard;
        cloneboard = NULL;
    }
    if(neighbors != NULL){
        for(int i = 0; i < maxpoint; i++){
            delete [] neighbors[i];
        }
        delete [] neighbors;
        neighbors = NULL;
    }
    if(random_table != NULL){
        for(int i = 0; i < maxpoint; i++){
            delete [] random_table[i];
        }
        delete [] random_table;
        random_table = NULL;
    }
    
    maxpoint = n * n + 3 * (n + 1) - 1;
    
    board = new int[maxpoint];
    neighbors = new int*[maxpoint];
    random_table = new unsigned int*[maxpoint];
    
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
    
    // Build random table
    for(int i = 0; i < maxpoint; i++){
        random_table[i] = new unsigned int[4];
        for(int j = 0; j < 3; j++){
            random_table[i][j] = gen();
        }
    }
    
    // Compute init_key
    for(int i = 0; i < maxpoint; i++){
        init_key ^= random_table[i][(board[i])];
    }
    
    if (root == NULL){
        root = new TREE_NODE;
        root->child = NULL;
        root->sibling = NULL;
        root->best_child = NULL;
        root->visits = 0;
        root->rewards = 0;
    }
    
    hash_table.clear();
    hash_table.insert(make_pair(init_key, *root));
    clonekey = init_key;
    clonecolor = color_to_play;
    
    if (cloneboard == NULL){
        cloneboard = new int[maxpoint];
    }
    
    for(int i = 0; i < maxpoint; i++){
        cloneboard[i] = board[i];
    }
    
    timer_on = false;
}

int main(int argc, char* argv[]){
    string command;
    
    string moveToPlay;
    
    
    setBoardAndNeighbors();
    
    for (string line; getline(cin, line);){
        if (line.compare(0, 9, "boardsize") == 0){
            int board_size_read = 0;
            istringstream iss(line);
            iss >> command;
            iss >> board_size_read;
            
            if (!(board_size_read >= 3 && board_size_read <= 10)){
                cout << "? Invalid Board Size (valid for 3 to 10)\n\n" << flush;
                continue;
            }
            
            n = board_size_read;
            color_to_play = BLACK;
            
            setBoardAndNeighbors();
            
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
            double time_read = 0.0;
            istringstream iss(line);
            iss >> command;
            iss >> time_read;
            
            if (!(time_read >= 10.0 && time_read <= 1000.0)){
                cout << "? Invalid Time Limit (valid for 10.0 to 1000.0)\n\n" << flush;
                continue;
            }
            setTimer(time_read);
            cout << "=\n\n"<< flush;
        } else if (line.compare(0, 9, "showboard") == 0){
            if (board == NULL){
                cerr << "Board uninitialized." << endl << flush;
            } else {
                cout << "= \n" << showBoard(board, n) << flush;
            }
        } else if (line.compare(0, 7, "genmove") == 0){
            MOVE move;
            bool random = line.compare(7,6,"random") == 0;
            if (timer_on == false || random){
                move = genMoveRandom(board, color_to_play);
            } else {
                move = genMove(board, color_to_play);
            }
            
            if (move.arr[0] == 0 && move.arr[1] == 0){
                cout << "= resign\n\n" << flush;
            } else {
                if (timer_on == true && random){
                    moveRootToChild(move);
                }
                playMove(board, move, color_to_play, init_key);
                moves_after_start++;
                cout << "= " << moveToString(move) << "\n\n" << flush;
            }
        } else if (line.compare(0, 4, "play") == 0){
            istringstream iss(line);
            iss >> command;
            iss >> moveToPlay;
            
            if (!stringMoveIsLegal(moveToPlay)){
                cout << "? Illegal Move\n\n" << flush;
            } else {
                MOVE move = stringToMove(moveToPlay);
                if (timer_on == true){
                    moveRootToChild(move);
                }
                playMove(board, move, color_to_play, init_key);
                moves_after_start++;
                cout << "=\n\n" << flush;
            }
        } else if (line.compare(0, 4, "quit") == 0){
            cout << "=\n\n" << flush;
            break;
        } else {
            string unknowncommand;
            istringstream iss(line);
            iss >> unknowncommand;
            
            if (!unknowncommand.empty()){
                cout << "? Unknown Command: " << line << "\n\n" << flush;
            }
        }
        
    }
    
    
    if(board != NULL){
        delete[] board;
        board = NULL;
    }
    if(neighbors != NULL){
        for(int i = 0; i < maxpoint; i++){
            delete [] neighbors[i];
        }
        delete [] neighbors;
        neighbors = NULL;
    }
    
    return 0;
}