#ifndef DLX_H
#define DLX_H

#include <set>
#include <vector>
#include <chrono>
#include <iostream>
#include <stack>
#include <cmath>

/*
 * These classes implement Donald Knuth's Algorithm X with dancing links and applies it to three
 * different problems: sudoku, the n-queens problem, and polyomino tiling (a generalization of pentomino tiling). 
 */

using namespace std;

struct dlx_matrix_node;

// Holds the info for each column header, with the column number, number of remaining 1's, and top and bottom pointers.
struct dlx_header_node {
    int count;
    int matrix_column;
    dlx_matrix_node* top;
    dlx_matrix_node* bottom;
};

// Comparator for placing the header nodes into a set for doing the column selection heuristic.
struct header_compare {
    inline bool operator() (const dlx_header_node* l, const dlx_header_node* r) const {
        return l->count < r->count || 
            (l->matrix_column < r->matrix_column&&l->count==r->count); // keep the same order
    }
};

// Hold the info for a matrix node, with pointers up, down, left, and right. Also a pointer to the header and the row.
struct dlx_matrix_node {
    dlx_matrix_node* left;
    dlx_matrix_node* right;
    dlx_matrix_node* up;
    dlx_matrix_node* down;
    dlx_header_node* header;
    int matrix_row;
};

// Generic templated class which can be used to create solvers for various exact cover problems.
template <class input_format, class output_format>
class dlx_matrix {
public:
    // This enum allows the save setting of the solver to be set.
    // Solutions: save only solutions to the problem.
    // Correct moves: save correct moves approaching the solution (possibly slow).
    // All moves: save all moves made by the solver (very slow).
    // None: saves nothing, useful for enumerating the number of solutions
    //       to a problem as quickly as possible.
    enum class dlx_save_setting {
        solutions,
        correct_moves,
        all_moves,
        none
    };
    // Restores a removed column by linking it back to nodes it is attached to and reinserting it to the set.
    inline void restore_column(dlx_matrix_node* given_node, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) {
        for (dlx_matrix_node* curr_node = given_node->up; curr_node!=given_node; curr_node=curr_node->up) {// this loop visits all nodes remaining in a column EXCEPT the given one, downward
            // this loop visits all nodes remaining in a row EXCEPT the one it shares with the column, rightward
            for (dlx_matrix_node* curr_row_node = curr_node->left; curr_row_node!=curr_node; curr_row_node=curr_row_node->left) {
#ifdef DEBUG
                if (curr_row_node->header->count < 0) {
                    cout << "Negative count.\n";
                    exit(1);
                }
                else {
#endif
                if (curr_row_node->header->count == 0) {
                    curr_row_node->header->top = curr_row_node;
                    curr_row_node->header->bottom = curr_row_node;
                }
                else {
                    curr_row_node->up->down = curr_row_node;
                    curr_row_node->down->up = curr_row_node;
                    if (curr_row_node->matrix_row>curr_row_node->header->bottom->matrix_row/*&&curr_row_node->down==curr_row_node->header->top*/) {
                        curr_row_node->header->bottom = curr_row_node;
                    }
                    else if (curr_row_node->matrix_row<curr_row_node->header->top->matrix_row/*&&curr_row_node->up==curr_row_node->header->bottom*/) {
                        curr_row_node->header->top = curr_row_node;
                    }
                }
#ifdef DEBUG
                }
#endif
                
                //the safety checks on these are almost certainly unnecessary
                //if (header_tree.erase(curr_row_node->header)>0) {
                if (curr_row_node->header->matrix_column<optional_constraint_start_column) {
                    header_tree.erase(curr_row_node->header);
                    curr_row_node->header->count+=1;
                    header_tree.insert(curr_row_node->header);
                }
                else {
                    optional_header_tree.erase(curr_row_node->header);
                    curr_row_node->header->count+=1;
                    optional_header_tree.insert(curr_row_node->header);
                }
            }
        }
    }
    // Removes a column by unlinking its neighbors from it and removing it from the set.
    inline void remove_column(dlx_matrix_node* given_node, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) {
        for (dlx_matrix_node* curr_node = given_node->down; curr_node!=given_node; curr_node=curr_node->down) {// this loop visits all nodes remaining in a column EXCEPT the given one, downward
            // this loop visits all nodes remaining in a row EXCEPT the one it shares with the column, rightward
            for (dlx_matrix_node* curr_row_node = curr_node->right; curr_row_node!=curr_node; curr_row_node=curr_row_node->right) {
                //if (header_tree.erase(curr_row_node->header)>0) {
                if (curr_row_node->header->matrix_column<optional_constraint_start_column) {
                    header_tree.erase(curr_row_node->header);
                    curr_row_node->header->count-=1;
                    header_tree.insert(curr_row_node->header);
                }
                else {
                    optional_header_tree.erase(curr_row_node->header);
                    curr_row_node->header->count-=1;
                    optional_header_tree.insert(curr_row_node->header);
                }
#ifdef DEBUG
                if (curr_row_node->header->top==NULL||curr_row_node->header->bottom==NULL) {
                    cout << "Null top and bottom nodes on header, count " << curr_row_node->header->count << endl;
                    exit(1);
                }
#endif
                if (curr_row_node->header->count==0) {
                    curr_row_node->header->top = NULL;
                    curr_row_node->header->bottom = NULL;
                }
#ifdef DEBUG
                else if (curr_row_node->header->count<0) {
                    cout << "Negative count in remove.\n";
                    exit(1);
                }
#endif
                else {
                    if (curr_row_node==curr_row_node->header->top) {
                        curr_row_node->header->top = curr_row_node->down;
                    }
                    else if (curr_row_node==curr_row_node->header->bottom) {
                        curr_row_node->header->bottom = curr_row_node->up;
                    }
                    curr_row_node->up->down = curr_row_node->down;
                    curr_row_node->down->up = curr_row_node->up;
                }
            }
        }
    }
    // Restores a removed (previously selected) row by restoring all attached columns.
    inline void restore_row(dlx_matrix_node* given_row_node, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) {
        // visits all columns except the one attached to the given node
        for (dlx_matrix_node* base_row_node = given_row_node->left; base_row_node!=given_row_node; base_row_node=base_row_node->left) {
            restore_column(base_row_node, header_tree, optional_header_tree);
            if (base_row_node->header->matrix_column<optional_constraint_start_column) {
                header_tree.insert(base_row_node->header);
            }
            else {
                optional_header_tree.insert(base_row_node->header);
            }
        }
    }
    // Removes a row by removing all attached columns.
    inline void remove_row(dlx_matrix_node* given_row_node, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) {
        // visits all columns except the one attached to the given node
        for (dlx_matrix_node* base_row_node = given_row_node->right; base_row_node!=given_row_node; base_row_node=base_row_node->right) {
            if (base_row_node->header->matrix_column<optional_constraint_start_column) {
                header_tree.erase(base_row_node->header);
            }
            else {
                optional_header_tree.erase(base_row_node->header);
            }
            remove_column(base_row_node, header_tree, optional_header_tree);
        }
    }
    // This wrapper may be used to automatically fetch only the first solution to a problem (if one exists).
    inline output_format solve(input_format data_in) {
        return solve(data_in,1)[0];
    }
    // This function implements Knuth's Algorithm X with dancing links.
    // The algorithm is complex, but essentially it just iterates over the exact cover matrix and 
    // selects columns to satisfy, then iterates over the possible solutions.
    // If it cannot find a solution, it undoes a choice and selects the next row. 
    // If it runs out of choices to undo, there is no solution.
    // If it runs out of columns to satisfy, a solution has been found.
    // Finally, the number of solutions can be configured, and the problem states that get saved can also be configured.
    vector<output_format> solve(input_format data_in, int max_solutions, dlx_save_setting save_setting=dlx_save_setting::solutions) {
        chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
        chrono::high_resolution_clock::time_point prev_time;
        int attempts = 0;
        int solution_count = 0;
        set<dlx_header_node*,header_compare> header_tree;
        set<dlx_header_node*,header_compare> optional_header_tree;
        stack<pair<dlx_header_node*,dlx_matrix_node*> > backtrack_stack;
        vector<output_format> solutions;
        if (max_solutions==-1) {
            max_solutions = numeric_limits<int>::max();
        }
        prev_time = chrono::high_resolution_clock::now();
        for (int i = 0; i<optional_constraint_start_column; i++) {
            header_tree.insert(header_tree.end(),&matrix_header[i]);
        }
        for (int i = optional_constraint_start_column; i < array_width; i++) {
            optional_header_tree.insert(optional_header_tree.end(),&matrix_header[i]);
        }
        //verify_matrix();
        prev_time = chrono::high_resolution_clock::now();
        if (!initialize(data_in,header_tree,optional_header_tree)) {
            return vector<output_format>();
        }
        if (do_debug_output) {
            cout << "Initialization took " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
        }
        prev_time = chrono::high_resolution_clock::now();
        while (solution_count<max_solutions) {
            if (header_tree.empty()||(*header_tree.begin())->count<=0/*||(!optional_header_tree.empty()&&(*optional_header_tree.begin())->count<0)*/) {
                if (header_tree.empty()) {
                    if (save_setting == dlx_save_setting::solutions) {
                        if (do_debug_output)
                            cout << "Found solution number " << solutions.size()+1 << endl;
                        solutions.push_back(interpret_result(backtrack_stack));
                    }
                    else if (save_setting == dlx_save_setting::correct_moves) {
                        stack<pair<dlx_header_node*, dlx_matrix_node*> > temporary_stack = backtrack_stack;
                        stack<output_format> temporary_results;
                        while (!temporary_stack.empty()) {
                            temporary_results.push(interpret_result(temporary_stack));
                            temporary_stack.pop();
                        }
                        cout << temporary_results.size() << endl;
                        while (!temporary_results.empty()) {
                            solutions.push_back(temporary_results.top());
                            temporary_results.pop();
                        }
                    }
                    else {
                        if ((solution_count%100000)==0&&do_debug_output) {
                            cout << "Found solution number " << solutions.size()+1 << endl;
                        }
                    }
                    solution_count++;
                }
                while (!backtrack_stack.empty()&&backtrack_stack.top().first->bottom==backtrack_stack.top().second) {
                    restore_row(backtrack_stack.top().second, header_tree,optional_header_tree);
                    restore_column(backtrack_stack.top().second, header_tree, optional_header_tree);
                    header_tree.insert(backtrack_stack.top().first);
                    backtrack_stack.pop();
                }
                if (backtrack_stack.empty()) {
                    if (solution_count<1) {
                        cout << "Unable to solve.\n";
                        return vector<output_format>();
                    }
                    else {
                        if (do_debug_output)
                            cout << "Solve took " << attempts << " attempts and " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
                        return solutions;
                    }
                }
                attempts++;
                dlx_header_node* curr_header = backtrack_stack.top().first;
                dlx_matrix_node* curr_node = backtrack_stack.top().second->down;
                restore_row(backtrack_stack.top().second, header_tree, optional_header_tree);
                restore_column(backtrack_stack.top().second, header_tree, optional_header_tree);
                remove_column(curr_node, header_tree, optional_header_tree);
                backtrack_stack.pop();
                backtrack_stack.push({curr_header,curr_node});
                remove_row(curr_node, header_tree, optional_header_tree);
                if (save_setting == dlx_save_setting::all_moves) {
                    solutions.push_back(interpret_result(backtrack_stack));
                }
            }
            else { // get the constraint with the fewest satisfaction options remaining
            // attempt to satisfy the first option by removing the row that
            // represents it, the constraint columns that it satisfies,
            // and all rows which also satisfy those constraints
            // also remove 
                attempts++;
                dlx_header_node* curr_header = *header_tree.begin();
                dlx_matrix_node* curr_node = curr_header->top;
                backtrack_stack.push({curr_header,curr_node});
                header_tree.erase(curr_header);
                remove_column(curr_node, header_tree, optional_header_tree);
                remove_row(curr_node, header_tree, optional_header_tree);
                if (save_setting == dlx_save_setting::all_moves) {
                    solutions.push_back(interpret_result(backtrack_stack));
                }
            }
        }
        if (do_debug_output)
            cout << "Solve took " << attempts << " attempts and " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
        return solutions;
    }
protected:
    // This function generates the intial matrix for the problem. It may be overridden for more
    // efficient implementions.
    virtual void generate() {
        if (num_constraints<=1) {
            cerr << "The number of constraints must be greater than one." << endl;
        }
        for (int i = 0; i < array_width; i++) {
        // iterate over and initialize the header array
        // with the proper count and matrix column values
        // set the top and bottom nodes to NULL
            matrix_header[i].count = get_count(i);
            matrix_header[i].matrix_column = i;
            matrix_header[i].top = NULL;
            matrix_header[i].bottom = NULL;
        }
        for (int matrix_row=0; matrix_row<array_len; matrix_row++) {
        // iterate over the rows of the matrix, connecting them together with pointers
        // header gets connected to the column header, right gets connected to the
        // node to the right (or the first one if the node is the last node),
        // left gets connected to the node to the left (or the last one if the
        // node is the first node), matrix_row gets set to the current matrix row,
        // top gets connected to the next node up, and bottom gets connected to the
        // next node down
        // if the node is the only node in the column, it is connected up and down to itself
            for (int i=0; i<num_constraints; i++) {
                matrix[matrix_row][i].header = &matrix_header[get_column(matrix_row,i)];
                matrix[matrix_row][i].right = &matrix[matrix_row][(i!=num_constraints-1)?i+1:0];
                matrix[matrix_row][i].left = &matrix[matrix_row][(i!=0)?i-1:num_constraints-1];
                dlx_header_node* curr_header = matrix[matrix_row][i].header;
                matrix[matrix_row][i].matrix_row = matrix_row;
                if (curr_header->top==NULL) {
                    curr_header->top = &matrix[matrix_row][i];
                    curr_header->bottom = &matrix[matrix_row][i];
                    matrix[matrix_row][i].up = &matrix[matrix_row][i];
                    matrix[matrix_row][i].down = &matrix[matrix_row][i];
                }
                else {
                    matrix[matrix_row][i].up = curr_header->bottom;
                    matrix[matrix_row][i].down = curr_header->top;
                    curr_header->bottom->down = &matrix[matrix_row][i];
                    curr_header->top->up = &matrix[matrix_row][i];
                    curr_header->bottom = &matrix[matrix_row][i];
                }
            }
        }
    }
    // This function is used by the generic (though inefficient) version of the generate function.
    // This function returns the initial 1 count for a column given its index.
    virtual int get_count(int matrix_column) = 0;
    // This function is used by the generic (though inefficient) version of the generate function.
    // Returns the matrix column given the row number and constraint number.
    virtual int get_column(int matrix_row, int constraint_num) = 0;
    // This function sets the intitial matrix state for a given problem.
    virtual bool initialize(input_format &data_in, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) = 0;
    // This function converts the backtrack stack into a solution for the problem.
    // Returns the solution as output_format.
    virtual output_format interpret_result(stack<pair<dlx_header_node*,dlx_matrix_node*> > backtrack_stack) = 0;
    // This function prints a constraint, used for debugging.
    void virtual print_constraint(int matrix_column, int matrix_row = -1) {}
    // Saves the dimensions of the initial sparse matrix.
    // array_len: length of the matrix
    // array_width: width of the matrix
    // num_constraints: number of columns in the (non-sparse) matrix.
    int array_len, array_width, num_constraints;
    // Sparse matrix which represents the problem.
    vector<vector<dlx_matrix_node> > matrix;
    // Headers which represent a constraint to satisfy.
    vector<dlx_header_node> matrix_header;
    // Saves the initial state of the problem.
    input_format initial_data;
    // This indicates the starting position of optional columns (optional columns must all be to the right of mandatory columns)
    int optional_constraint_start_column = numeric_limits<int>::max();
    // This flags turns on debug output, including timing of some components.
    bool do_debug_output;
};

/*
 * This class implements the virtual functions of the generic solver so that it may solve sudokus.
 * Inputs must be in the form of a two dimensional vector of ints, with -1 representing empty cells.
 * Vector must be a square, and its sides lengths must be squares.
 * Large puzzles (more than 4-5) will be very slow, because sudokus are very complex to solve.
 */
class dlx_matrix_sudoku : public dlx_matrix<vector<vector<int> >, vector<vector<int> > > {
public:
    dlx_matrix_sudoku(int puzzle_width, bool do_debug=false) {
        do_debug_output = do_debug;
        sudoku_width = puzzle_width;
        sqrt_width = (int)sqrt(sudoku_width);
        num_constraints = 4;
        array_len = sudoku_width*sudoku_width*sudoku_width;
        array_width = sudoku_width*sudoku_width*num_constraints;
        optional_constraint_start_column = array_width;
        matrix.resize(array_len,vector<dlx_matrix_node>(num_constraints));
        matrix_header.resize(array_width);
        chrono::high_resolution_clock::time_point prev_time = chrono::high_resolution_clock::now();
        generate();
        if (do_debug_output)
            cout << "Generation took " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
    }
protected:
    virtual bool initialize(vector<vector<int> > &starting_puzzle, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) {
        initial_data = starting_puzzle;
        for (int row = 0; row < starting_puzzle.size(); row++) {
            for (int col = 0; col < starting_puzzle[0].size(); col++) {
                if (starting_puzzle[row][col]!=-1) {
                    dlx_matrix_node* base_row_node = &matrix[(row*sudoku_width+col)*sudoku_width+starting_puzzle[row][col]-1][0];
                    // this is a deconstructed for loop to avoid checking the condition on the first iteration (since it would be false)
                    dlx_matrix_node* curr_row_node = base_row_node;
                    do {
                        if ((*header_tree.begin())->count<=0) {
                            cout << "Input is over-constrained.\n";
                            return false;
                        }
                        header_tree.erase(curr_row_node->header);
                        remove_column(curr_row_node, header_tree, optional_header_tree);
                        curr_row_node=curr_row_node->right;
                    } while (curr_row_node!=base_row_node);
                }
            }
        }
        return true;
    }
    virtual vector<vector<int> > interpret_result(stack<pair<dlx_header_node*,dlx_matrix_node*> > backtrack_stack) {
        chrono::high_resolution_clock::time_point prev_time = chrono::high_resolution_clock::now();
        vector<vector<int> > solved_puzzle = initial_data;
        while (!backtrack_stack.empty()) {
            int matrix_column = matrix[backtrack_stack.top().second->matrix_row][0].header->matrix_column;
            int matrix_row = backtrack_stack.top().second->matrix_row;
            solved_puzzle[matrix_column%(sudoku_width*sudoku_width)/sudoku_width][matrix_column%(sudoku_width*sudoku_width)%sudoku_width] = matrix_row%sudoku_width+1;
            backtrack_stack.pop();
        }
        if (do_debug_output)
            cout << "Interpretation took " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
        return solved_puzzle;
    }
    virtual int get_count(int matrix_column) {
        return sudoku_width;
    }
    virtual int get_column(int matrix_row, int constraint_num) {
        int row_num = matrix_row/sudoku_width/sudoku_width;
        int col_num = matrix_row/sudoku_width%sudoku_width;
        int curr_num = matrix_row%sudoku_width;
        switch (constraint_num) {
            case 0:
                return row_num*sudoku_width+col_num;
            case 1:
                return row_num*sudoku_width+curr_num+sudoku_width*sudoku_width;
            case 2:
                return col_num*sudoku_width+curr_num+sudoku_width*sudoku_width*2;
            case 3:
                return (row_num/sqrt_width*sqrt_width+col_num/sqrt_width)*sudoku_width+curr_num+sudoku_width*sudoku_width*3;
            default: return -1;
        }
    }
private:
    int sudoku_width;
    int sqrt_width; 
};

/*
 * The remainder of this file is currently uncommented, though functional.
 */

class dlx_matrix_n_queens : public dlx_matrix<vector<vector<int> >, vector<vector<int> > > {
public:
    dlx_matrix_n_queens(int board_width_in) {
        board_width = board_width_in;
        num_mandatory_constraints = 2;
        num_optional_constraints = 2;
        num_constraints = num_mandatory_constraints+num_optional_constraints;
        array_len = board_width*board_width;
        array_width = board_width*num_mandatory_constraints+(board_width*2-1)*num_optional_constraints;
        optional_constraint_start_column = board_width*num_mandatory_constraints;
        matrix.resize(array_len,vector<dlx_matrix_node>(num_constraints));
        matrix_header.resize(array_width);
        chrono::high_resolution_clock::time_point prev_time = chrono::high_resolution_clock::now();
        generate();
        cout << "Generation took " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
    }
protected:
    virtual bool initialize(vector<vector<int> > &starting_puzzle, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) {
        //print_headers(header_tree);
        initial_data = starting_puzzle;
        for (int row = 0; row < starting_puzzle.size(); row++) {
            for (int col = 0; col < starting_puzzle[0].size(); col++) {
                if (starting_puzzle[row][col]!=0) {
                    dlx_matrix_node* base_row_node = &matrix[row*board_width+col][0];
                    do {
                        if ((*header_tree.begin())->count<=0) {
                            cout << "Input is over-constrained.\n";
                            return false;
                        }
                        header_tree.erase(base_row_node->header);
                        remove_column(base_row_node, header_tree,optional_header_tree);
                        base_row_node=base_row_node->right;
                    } while (base_row_node!=&matrix[row*board_width+col][0]);
                }
            }
        }
        return true;
    }
    virtual vector<vector<int> > interpret_result(stack<pair<dlx_header_node*,dlx_matrix_node*> > backtrack_stack) {
        chrono::high_resolution_clock::time_point prev_time = chrono::high_resolution_clock::now();
        vector<vector<int> > result;
        if (initial_data!=vector<vector<int> >()) {
            result = initial_data;
        }
        else {
            result = vector<vector<int> >(board_width,vector<int>(board_width));
        }
        while (!backtrack_stack.empty()) {
            int matrix_column = matrix[backtrack_stack.top().second->matrix_row][0].header->matrix_column;
            int matrix_row = backtrack_stack.top().second->matrix_row;
            result[matrix_row/board_width][matrix_row%board_width] = 1;
            backtrack_stack.pop();
        }
        cout << "Interpretation took " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
        return result;
    }
    virtual int get_count(int matrix_column) {
        if (matrix_column<board_width*2) {
            return board_width;
        }
        else if (matrix_column<board_width*4-1) {
            return board_width-abs(board_width*3-matrix_column-1);
        }
        else {
            return board_width-abs(board_width*5-matrix_column-2);
        }
    }
    virtual int get_column(int matrix_row, int constraint_num) {
        int row_num = matrix_row/board_width;
        int col_num = matrix_row%board_width;
        switch (constraint_num) {
            case 0:
                return row_num;
            case 1:
                return col_num+board_width;
            case 2:
                return col_num+row_num+board_width*2;
            case 3:
                return board_width-1-row_num+col_num+board_width*2+board_width*2-1;
            default: return -1;
        }
    }
private:
    int board_width;
    int num_mandatory_constraints;
    int num_optional_constraints;
};

struct set_polyomino_compare {
    inline bool operator() (const set<pair<int,int> > l, const set<pair<int,int> > r) const {
        if (l.size()!=r.size()) {
            return l.size()<r.size();
        }
        for (set<pair<int,int> >::iterator l_iter = l.begin(), r_iter = r.begin(); l_iter!=l.end()&&r_iter!=r.end(); l_iter++,r_iter++) { //technically r_iter!=r.end() should happen when l_iter!=l.end() since we confirmed that they are the same size
            if (*l_iter!=*r_iter) {
                return *l_iter<*r_iter;
            }
        }
        return false;
    }
};

class polyomino {
public:
    polyomino(vector<pair<int,int> > block_positions) {
        size = block_positions.size();
        x_len = 0;
        y_len = 0;
        for (int i = 0; i<block_positions.size(); i++) {
            if (block_positions[i].first<0||block_positions[i].second<0) {
                cout << "Negative positions are not allowed, as all positions should relative to a square where the upper left is (0,0).\n";
                exit(1);
            }
            else {
                if (block_positions[i].first>x_len) {
                    x_len = block_positions[i].first;
                }
                if (block_positions[i].second>y_len) {
                    y_len = block_positions[i].second;
                }
            }
        }
        set<set<pair<int,int> >, set_polyomino_compare> shape_set;
        vector<set<set<pair<int,int> >, set_polyomino_compare>::iterator> flipped_orientation(4); //max 4 orientations which differ in dimension from the original
        set<pair<int,int> > last_rotation(block_positions.begin(),block_positions.end());
        shape_set.insert(last_rotation);
        set<pair<int,int> > current_rotation;
        for (int i=0; i<3; i++) { //log each rotation
            current_rotation.clear();
            for (set<pair<int,int> >::iterator last_iter = last_rotation.begin(); last_iter!=last_rotation.end(); last_iter++) {
                current_rotation.insert(pair<int,int>(last_iter->second,((i&1)==1?y_len:x_len)-last_iter->first));
            }
            if ((i&1)==0) { // if the number is even
                flipped_orientation[i/2] = shape_set.insert(current_rotation).first;
            }
            else {
                shape_set.insert(current_rotation);
            }
            last_rotation.swap(current_rotation);
        }
        last_rotation.clear();
        for (int i = 0; i < size; i++) {
            last_rotation.insert(pair<int,int>(block_positions[i].first,y_len-block_positions[i].second));
        }
        shape_set.insert(last_rotation);
        for (int i=0; i<3; i++) { //log each rotation
            current_rotation.clear();
            for (set<pair<int,int> >::iterator last_iter = last_rotation.begin(); last_iter!=last_rotation.end(); last_iter++) {
                current_rotation.insert(pair<int,int>(last_iter->second,((i&1)==1?y_len:x_len)-last_iter->first));
            }
            if ((i&1)==0) { // if the number is even
                flipped_orientation[i/2+2] = shape_set.insert(current_rotation).first;
            }
            else {
                shape_set.insert(current_rotation);
            }
            last_rotation.swap(current_rotation);
        }
        orientations = shape_set.size();
        shapes.resize(orientations);
        flipped.resize(orientations,false);
        int filled_size = 0;
        for (set<set<pair<int,int> >, set_polyomino_compare>::iterator shape_iter = shape_set.begin(); shape_iter!=shape_set.end(); shape_iter++, filled_size++) {
            vector<pair<int,int> > curr_shape(size);
            copy(shape_iter->begin(),shape_iter->end(),curr_shape.begin());
            shapes[filled_size] = curr_shape;
            for (int i = 0; i < 4; i++) {
                if (flipped_orientation[i]==shape_iter) {
                    flipped[filled_size] = true;
                    break;
                }
            }
        }
        //for the duration of these code these were end indices, rather than lengths. Thus, to become lengths, they must be iterated by one
        x_len++;
        y_len++;
    }
    inline int const getXLength() {
        return x_len;
    }
    inline int const getXLength(int orientation_num) {
        return flipped[orientation_num]?y_len:x_len;
    }
    inline int const getYLength() {
        return y_len;
    }
    inline int const getYLength(int orientation_num) {
        return flipped[orientation_num]?x_len:y_len;
    }
    inline int const getSize() {
        return size;
    }
    inline int const getOrientations() {
        return orientations;
    }
    inline pair<int,int> getBlock(int orientation_num, int block_num) {
        //cout << shapes[orientation_num][block_num].first << "," << shapes[orientation_num][block_num].second << endl;
        return shapes[orientation_num][block_num];
    }
    void printShape(int orientation_num) {
        for (int row = 0; row < (flipped[orientation_num]?x_len:y_len); row++) {
            for (int col = 0; col < (flipped[orientation_num]?y_len:x_len); col++) {
                bool found = false;
                for (int i = 0; i < shapes[orientation_num].size(); i++) {
                    if (shapes[orientation_num][i] == pair<int,int>(col,row)) {
                        found = true;
                    }
                }
                if (found) {
                    cout << (char)219;
                }
                else {
                    cout << " ";
                }
                cout << " ";
            }
            cout << endl;
        }
    }
    vector<char> flipped; //I would like a vector of bools, but vector<bool> is terrible, so we have vector<char> instead
private:
    int x_len,y_len, size, orientations;
    vector<vector<pair<int,int> > > shapes;
    
};

class dlx_matrix_polyomino : public dlx_matrix<vector<vector<int> >, vector<vector<int> > > {
public:
    dlx_matrix_polyomino(vector<polyomino> polyomino_list_in,int board_width_in) {
        board_width = board_width_in;
        num_constraints = num_mandatory_constraints;
        array_width = board_width*board_width+polyomino_list_in.size();
        optional_constraint_start_column = array_width;
        matrix_header.resize(array_width);
        polyomino_list = polyomino_list_in;
        chrono::high_resolution_clock::time_point prev_time = chrono::high_resolution_clock::now();
        generate();
        //polyomino_list[1].printShape(1);
        polyomino_generate();
        cout << "Generation took " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
    }
protected:
    virtual bool initialize(vector<vector<int> > &data_in, set<dlx_header_node*,header_compare> &header_tree, set<dlx_header_node*,header_compare> &optional_header_tree) {
        board = data_in;
        for (int row = 0; row < board.size(); row++) {
            for (int col = 0; col < board[0].size(); col++) {
                if (board[row][col]!=0) {
                    dlx_header_node* header_target = &matrix_header[row*board_width+col];
                    if ((*header_tree.begin())->count<=0) {
                        cout << "Input is over-constrained.\n";
                        cout << (*header_tree.begin())->matrix_column << " " << (*header_tree.begin())->count << endl;
                        return false;
                    }
                    header_tree.erase(header_target);
                    remove_column(header_target->top, header_tree,optional_header_tree);
                    header_target->top->right->left = header_target->top->left;
                    header_target->top->left->right = header_target->top->right;
                }
            }
        }
        return true;
    }
    virtual void generate() {
        for (int i = 0; i < array_width; i++) {
            matrix_header[i].count = 0;
            matrix_header[i].matrix_column = i;
            matrix_header[i].top = NULL;
            matrix_header[i].bottom = NULL;
        }
    }
    void polyomino_generate() {
        for (int row = 0; row < board_width; row++) {
            for (int col = 0; col < board_width; col++) {
                for (int poly_num = 0; poly_num < polyomino_list.size(); poly_num++) {
                    polyomino curr_poly = polyomino_list[poly_num];
                    for (int orient = 0; orient < curr_poly.getOrientations(); orient++) {
                        if ((row+curr_poly.getYLength(orient)) <= board_width && (col+curr_poly.getXLength(orient)) <= board_width) {
                            matrix.push_back(vector<dlx_matrix_node>(curr_poly.getSize()+1));
                            for (int i = 0; i < curr_poly.getSize(); i++) {
                                matrix.back()[i].matrix_row = matrix.size()-1;
                                matrix.back()[i].header = &matrix_header[curr_poly.getBlock(orient,i).first+col+(curr_poly.getBlock(orient,i).second+row)*board_width];
                                if (curr_poly.getBlock(orient,i).first+col+(curr_poly.getBlock(orient,i).second+row)*board_width>=board_width*board_width||curr_poly.getBlock(orient,i).first+col+(curr_poly.getBlock(orient,i).second+row)*board_width<0) {
                                    cout << "Error at " << matrix.size() << " " << curr_poly.getBlock(orient,i).first << " " << curr_poly.getBlock(orient,i).second << endl;
                                    exit(1);
                                }
                                matrix.back()[i].right = &matrix.back()[i+1];
                                matrix.back()[i].left = &matrix.back()[((i>0)?i-1:curr_poly.getSize())];
                                dlx_header_node* curr_header = matrix.back()[i].header;
                                if (matrix.back()[i].header->top==NULL) {
                                    matrix.back()[i].header->top = &matrix.back()[i];
                                    matrix.back()[i].header->bottom = &matrix.back()[i];
                                    matrix.back()[i].up = &matrix.back()[i];
                                    matrix.back()[i].down = &matrix.back()[i];
                                }
                                else {
                                    matrix.back()[i].up = curr_header->bottom;
                                    matrix.back()[i].down = curr_header->top;
                                    curr_header->bottom->down = &matrix.back()[i];
                                    curr_header->top->up = &matrix.back()[i];
                                    curr_header->bottom = &matrix.back()[i];
                                }
                                curr_header->count++;
                            }
                            int poly_constr = curr_poly.getSize();
                            matrix.back()[poly_constr].matrix_row = matrix.size()-1;
                            matrix.back()[poly_constr].header = &matrix_header[poly_num+board_width*board_width];
                            matrix.back()[poly_constr].right = &matrix.back()[0];
                            matrix.back()[poly_constr].left = &matrix.back()[curr_poly.getSize()-1];
                            dlx_header_node* curr_header = matrix.back()[poly_constr].header;
                            if (matrix.back()[poly_constr].header->top==NULL) {
                                matrix.back()[poly_constr].header->top = &matrix.back()[poly_constr];
                                matrix.back()[poly_constr].header->bottom = &matrix.back()[poly_constr];
                                matrix.back()[poly_constr].up = &matrix.back()[poly_constr];
                                matrix.back()[poly_constr].down = &matrix.back()[poly_constr];
                            }
                            else {
                                matrix.back()[poly_constr].up = curr_header->bottom;
                                matrix.back()[poly_constr].down = curr_header->top;
                                curr_header->bottom->down = &matrix.back()[poly_constr];
                                curr_header->top->up = &matrix.back()[poly_constr];
                                curr_header->bottom = &matrix.back()[poly_constr];
                            }
                            curr_header->count++;
                        }
                        //cout << endl;
                    }
                }
            }
        }
        array_len = matrix.size();
    }
    virtual vector<vector<int> > interpret_result(stack<pair<dlx_header_node*,dlx_matrix_node*> > backtrack_stack) {
        chrono::high_resolution_clock::time_point prev_time = chrono::high_resolution_clock::now();
        vector<vector<int> > result;
        if (board!=vector<vector<int> >()) {
            result = board;
        }
        else {
            result = vector<vector<int> >(board_width,vector<int>(board_width));
        }
        while (!backtrack_stack.empty()) {
            int matrix_column = matrix[backtrack_stack.top().second->matrix_row][0].header->matrix_column;
            int matrix_row = backtrack_stack.top().second->matrix_row;
            for (int i = 0; i < matrix[matrix_row].size()-1; i++) {
                result[matrix[matrix_row][i].header->matrix_column/board_width][matrix[matrix_row][i].header->matrix_column%board_width] = matrix[matrix_row].back().header->matrix_column-board_width*board_width;
            }
            backtrack_stack.pop();
        }
        if (do_debug_output)
            cout << "Interpretation took " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now()-prev_time).count() << "mus" << endl;
        return result;
    }
    virtual int get_count(int matrix_column) {
        if (matrix_column<board_width*2) {
            return board_width;
        }
        else if (matrix_column<board_width*4-1) {
            return board_width-abs(board_width*3-matrix_column-1);
        }
        else {
            return board_width-abs(board_width*5-matrix_column-2);
        }
    }
    virtual int get_column(int matrix_row, int constraint_num) {
        int row_num = matrix_row/board_width;
        int col_num = matrix_row%board_width;
        switch (constraint_num) {
            case 0:
                return row_num;
            case 1:
                return col_num+board_width;
            case 2:
                return col_num+row_num+board_width*2;
            case 3:
                return board_width-1-row_num+col_num+board_width*2+board_width*2-1;
            default: return -1;
        }
    }
private:
    vector<vector<int> > board;
    vector<polyomino> polyomino_list;
    int board_width;
    int num_mandatory_constraints;
};

#endif
