#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
using namespace std;
/*
 * Constraints:
 *  - white dot btwn adjacent cells require abs(cell_1 - cell_2) = 1
 *  - black dot btwn adjacent cells require one to be double of the other
 *  Notes:
 *  horizontal constraints are between the current cell & the immediate right
 *  vertical constraints are between the current cell & directly below it
 */

struct Cell {
    int val;
    set<int> domain = {1, 2, 3, 4, 5, 6, 7, 8, 9};
};
const int NUM_ROWS_COLS = 9,
          NUM_CELL_GROUPS = 3;
          
const int BLANK = 0,
          WHITE = 1,
          BLACK = 2;        
const set<int> DOMAIN = {1, 2, 3, 4, 5, 6, 7, 8, 9};
vector<vector<int>> g_board;
vector<vector<int>> g_horizontal_dots;
vector<vector<int>> g_vertical_dots;
map<int, set<int>> g_row_to_values; // map row # to vals in row
map<int, set<int>> g_col_to_values; // map col # to vals in col
map<pair<int, int>, set<int>> g_subgrid_to_values; // map subgrid coordinates to vals in subgrid


//Prototypes
void parse_file(const string& filepath); // Populate board and constraint matrices
void print_matrix(const vector<vector<int>>& matrix); // Print a matrix
void initialize(); // Initialize the board and auxiliary data structures
void backtrack(); // Perform backtracking to solve the puzzle
pair<int,int> select_unassigned_variable(const vector<vector<int>>& state); /*Uses 
MRV and DH to select an unassigned variable*/
vector<pair<int,int>> mrv(const vector<vector<int>>& state);// calculates Minimum remaining value
pair<int, int> degree_heuristic(const vector<pair<int,int>>& mrv_array); // uses degree heuristic to select
                                                                        //variable 
bool horizontal_dots(int row, int col, int val); // verify if horizontal constraint is satisfied
bool vertical_dots(int row, int col, int val); // verify if vertical constraint is satisfied

bool consistent(const vector<vector<int>>& state);
void parse_file(const string& filepath) { // populate board & constraint matrices
    ifstream file_obj(filepath);
    string line;
    for (int i = 0; i < NUM_CELL_GROUPS; i++) {
        for (int j = 0; j < NUM_ROWS_COLS; j++) {
            getline(file_obj, line);
            vector<int> row;
            istringstream ss(line);
            int number;
            while (ss >> number && row.size() < NUM_ROWS_COLS) {
                row.push_back(number);
            }
            if (i == 0) {
                g_board.push_back(row);
            } else if (i == 1) {
                g_horizontal_dots.push_back(row);
            } else if (i == 2 && j < (NUM_ROWS_COLS - 1)) {
                g_vertical_dots.push_back(row);
            }
            if ((i == 0 || i == 1) && (j == (NUM_ROWS_COLS - 1))) {
                getline(file_obj, line);
            }
        }
    }

    file_obj.close();
}


void print_matrix(const vector<vector<int>>& matrix) {
    for (vector<int> row : matrix) {
        for (int val : row) { cout << val << " "; }
        cout << endl;
    }
}

pair<int,int> select_unassigned_variable(const vector<vector<int>>& state){
    
    return degree_heuristic(mrv(state));

}
bool is_unique(const vector<vector<int>>& state, 
               int row, int col, int val) {
    for (int x : state[row]){
        if (x == val) return false;
    }
    for (int r = 0; r < 9; ++r){
        if (state[r][col] == val){return false;}
    }
    int row_start = (row / 3);
    int col_start = (col / 3);
    for (int r : g_subgrid_to_values[pair<int, int>{row_start, col_start}]) {
        if (r == val) { return false; }
    }

}
bool horizontal_dots(int row, int col, int val) {
    if (g_horizontal_dots[row][col] == WHITE) { // white 
        if (col + 1 < NUM_ROWS_COLS) { 
            if (g_board[row][col + 1] != 0) {
                return abs(val - g_board[row][col + 1] == 1); 
            }
            return true; // tile was unpopulated
        }
    } 
    else if (g_horizontal_dots[row][col] == BLACK) { // black
        if (col + 1 < NUM_ROWS_COLS) { 
            if (g_board[row][col + 1] != 0) {
                float quotient = (float)g_board[row][col + 1] / (float)val;
                return (quotient == 2 || quotient == 0.5);
            }
            return true; 
        }
    }
}
bool vertical_dots(int row, int col, int val) {
    if (g_vertical_dots[row][col] == WHITE) { // white 
        if (row + 1 < NUM_ROWS_COLS) {
            if (g_board[row + 1][col] != 0) {
                return abs(val - g_board[row + 1][col] == 1); 
            }
            return true;
        }
    } 
    else if ((g_vertical_dots[row][col] == BLACK)) { // black
        if (row + 1 < NUM_ROWS_COLS) {
            if (g_board[row + 1][col] != 0) {
                float quotient = (float)g_board[row][col + 1] / (float)val;
                return (quotient == 2 || quotient == 0.5);
            }
        }
    }
}

bool check_dot_constraints(int row, int col, int val) { 
    return horizontal_dots(row, col, val) && vertical_dots(row, col, val);
}
bool consistent(const vector<vector<int>>& state, int row, int col, int val) {
    return is_unique(state, row, col, val) && check_dot_constraints(row, col, val);
}
pair<int, int> degree_heuristic(const vector<pair<int,int>>& mrv_array) {
    int constraints = 1; // all diff
    int max_degree = 0;
    size_t max_idx = 0; // track idx of element with highest degree heuristic
    for (size_t i = 0; i < mrv_array.size(); i++) {
        pair<int, int> coord = mrv_array[i];
        if (g_horizontal_dots[coord.first][coord.second] != BLANK) {
            constraints++;
        }
        if (g_vertical_dots[coord.first][coord.second] != BLANK) {
            constraints++;
        }
        if (constraints > max_idx) {
            max_idx = i;
        }
    }
    
}
vector<pair<int,int>> mrv(const vector<vector<int>>& state) {
    int min_count = 10;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int legal_states = 0;
            for (int val: DOMAIN){
                if (consistent(state, i, j, val)){
                    ++legal_states;
            }
            if (legal_states < min_count) {}
            }
            
        }
        
    }
}



void initialize() {
    for (int i = 0; i < NUM_ROWS_COLS; i++) {
        for (int j = 0; j < NUM_ROWS_COLS; j++) {
            pair<int, int> subgrid_coords = {i / 3, j / 3};
            if (g_board[i][j] != 0) { // add val to set, row & cols
                g_subgrid_to_values[subgrid_coords].insert(g_board[i][j]);
                g_row_to_values[i].insert(g_board[i][j]);
                g_col_to_values[j].insert(g_board[i][j]);
            }
        }
    }
}

int main(int argc, char* argv[]) {
//    if (argc != 2) {cerr << "Requires input file\n"; return -1;}
//    string file_name = argv[1];

    string file_name = "Sample_Input.txt";
    parse_file(file_name);

    initialize();


}

void backtrack() {
    for (int i = 0; i < NUM_ROWS_COLS; i++) {
        for (int j = 0; j < NUM_ROWS_COLS; j++) {
            set<int> domain = {1, 2, 3, 4, 5, 6, 7, 8, 9};
            if (g_board[i][j] == 0) { // select unassigned variable (?)

                for (int val : g_row_to_values[i]) {
                    domain.erase(val);
                }
                for (int val : g_col_to_values[j]) {
                    domain.erase(val);
                }
                for (int val : g_subgrid_to_values[pair<int, int>{i / 3, j / 3}]) {
                    domain.erase(val);
                }
                if (g_horizontal_dots[i][j] == 1) { // white horizontal dot
                    if (g_horizontal_dots[i][j + 1] != 0) {}
                } else if (g_horizontal_dots[i][j] == 2) { // black horizontal dot

                }
                if (!domain.empty()) { // domain has values to pick from
                    g_board[i][j] = *domain.begin(); // assign to first value
                } else {
                    cerr << "Failed at (i, j) = " << i << " " << j << endl;
                    return; // failed if there are no values to pick from
                }
            }
        }
    }
}
 