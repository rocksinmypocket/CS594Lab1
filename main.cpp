#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <cmath>
#include <algorithm>
#include <regex>
#include "DLX.h"
#include "JGraph.h"
#include <fstream>
#include <sstream>

using namespace std;

/*
 * This class contains a sudoku print to JGraph and an interactive sudoku solver.
 */

class SudokuVisualizer {
private:
	JGraph::Canvas canvas;
public:
	// This constructor sets up the components of Canvas which do not get changed with the value of the sudoku.
	SudokuVisualizer() {
		canvas.bounding_box.X = 0;
		canvas.bounding_box.Y = 0;
		canvas.graphs.push_back(JGraph::Graph());
		JGraph::Graph& graph = canvas.graphs[0];

		JGraph::Axis& xaxis = graph.xaxis;
		xaxis.min = 0;
		xaxis.grid_lines = true;
		xaxis.minor_grid_lines = true;
		xaxis.mgrid_color = JGraph::Gray(.625);
		xaxis.draw = false;

		JGraph::Axis& yaxis = graph.yaxis;
		yaxis.min = 0;
		yaxis.grid_lines = true;
		yaxis.minor_grid_lines = true;
		yaxis.mgrid_color = JGraph::Gray(.625);
		yaxis.draw = false;

		/* new scope */ {
			graph.curves.push_back(JGraph::Curve());
			JGraph::Curve& lastCurve = graph.curves.back();
			lastCurve.lineType = JGraph::Curve::LineType::none;
			lastCurve.curveColor = JGraph::Color(1, 0, 0);
			JGraph::ShapeMark* badMark = new JGraph::ShapeMark();
			badMark->type = JGraph::ShapeMark::Type::box;
			badMark->size = { .925, .925 };
			badMark->pattern = JGraph::ShapeMark::FillPattern::solid;
			badMark->color = JGraph::Color(1, 0, 0);
			lastCurve.marks.reset(badMark);
		}

		/* new scope */ {
			graph.curves.push_back(JGraph::Curve());
			JGraph::Curve& lastCurve = graph.curves.back();
			lastCurve.lineType = JGraph::Curve::LineType::none;
			lastCurve.curveColor = JGraph::Color(0, 1, 0);
			JGraph::ShapeMark* goodMark = new JGraph::ShapeMark();
			goodMark->type = JGraph::ShapeMark::Type::box;
			goodMark->size = { .925, .925 };
			goodMark->pattern = JGraph::ShapeMark::FillPattern::solid;
			goodMark->color = JGraph::Color(0, 1, 0);
			lastCurve.marks.reset(goodMark);
		}

		/* new scope */ {
			graph.curves.push_back(JGraph::Curve());
			JGraph::Curve& lastCurve = graph.curves.back();
			lastCurve.lineType = JGraph::Curve::LineType::none;
			JGraph::TextMark* boardMark = new JGraph::TextMark();
			boardMark->text.font = "Arial-Monospaced-Monotype";
			boardMark->text.size = 20;
			boardMark->text.line_spacing = 20;
			lastCurve.marks.reset(boardMark);
		}
	}
	
	// This visualizer prints each sudoku board given a vector of boards and coordinates of squares which should be painted red (error_squares) and green (correct_squares).
	// boards: vector<vector<vector<int> > >, a vector of boards
	// name_format: a printf-style string which may contain a single int print value which will be filled with the index of the board to create the file name
	// error_squares, correct_squares: a vector of pairs of ints which define board cells to be painted red or green, respectively
	void visualizeSolution(vector<vector<vector<int> > > boards, string name_format, vector<pair<int, int> > error_squares = vector<pair<int, int> >(), vector<pair<int, int> > correct_squares = vector<pair<int, int> >()) {
		vector<char> name_buf = vector<char>(name_format.size() + log10(boards.size()) + 1);
		for (int i = 0; i < boards.size(); i++) {
			canvas.size.width = ((float)boards[i].size())/3;
			canvas.size.height = ((float)boards[i].size())/3;
			canvas.bounding_box.width = canvas.size.width * 72;
			canvas.bounding_box.height = canvas.size.height * 72;

			JGraph::Axis& xaxis = canvas.graphs[0].xaxis;
			xaxis.size_inches = ((float)boards[i].size())/3;
			xaxis.max = boards[i].size();
			xaxis.hash_spacing = sqrt(boards[i].size());
			xaxis.minor_hash_count = sqrt(boards[i].size())-1;

			JGraph::Axis& yaxis = canvas.graphs[0].yaxis;
			yaxis.size_inches = ((float)boards[i].size())/3;
			yaxis.max = boards[i].size();
			yaxis.hash_spacing = sqrt(boards[i].size());
			yaxis.minor_hash_count = sqrt(boards[i].size())-1;

			/* new scope */ {
				vector<JGraph::Point<float> >& points  = canvas.graphs[0].curves[0].points;
				points.clear();
				for (int a = 0; a < error_squares.size(); a++) {
					points.push_back({ (float)(error_squares[a].second + 0.5), (float)((int)boards[i].size() - error_squares[a].first - 1) + 0.5F});
				}
			}

			/* new scope */ {
				vector<JGraph::Point<float> >& points = canvas.graphs[0].curves[1].points;
				points.clear();
				for (int a = 0; a < correct_squares.size(); a++) {
					points.push_back({ (float)(correct_squares[a].second + 0.5), (float)((int)boards[i].size() - correct_squares[a].first - 1) + 0.5F });
				}
			}

			/* new scope */ {
				canvas.graphs[0].curves[2].points = { {((float)boards[i].size())/2, ((float)boards[i].size())/2} };
				JGraph::TextMark* boardMark = static_cast<JGraph::TextMark*>(canvas.graphs[0].curves[2].marks.get());
				boardMark->text.content = "";
				for (int a = 0; a < boards[i].size(); a++) {
					for (int b = 0; b < boards[i][a].size(); b++) {
						if (boards[i][a][b] > 0) {
							boardMark->text.content += boards[i][a][b] + '0';
							boardMark->text.content += " ";
						}
						else {
							boardMark->text.content += "  ";
						}
					}
					boardMark->text.content.erase(boardMark->text.content.size() - 1, 1);
					boardMark->text.content += "\n";
				}
				boardMark->text.content.erase(boardMark->text.content.size() - 1, 1);
			}
			
			int name_size = snprintf(&name_buf[0], name_buf.size(), name_format.c_str(), i);
			JGraph::jgraphToJPG(canvas, string(&name_buf[0], name_size), true);
		}
	}
	
	// This function runs a command-line interface which allows a user to interactively solve a sudoku.
	// It has the following commands:
	// 
	// solution - displays the solution for the current puzzle
	// check - highlights correct and incorrect guesses for the current board in green and red, respectively
	// {int int int} - interpretted as \"value row column\", used for making guesses on the current board
	// nonvalid values will empty the square
	// view - prints the current board
	// save {string} - saves the current board to the given file name
	// file name must contain only alphanumeric characters with a period for the file extension
	// exit - closes the program
	// help - displays a list of commands
	// 
	// in and out: istream and ostream references; they will probably normally be cin and cout
	// target_puzzle: a sudoku as vector<vector<int> >; sides must be the same and squares
	// output_file: name of the output .jpg file
	void interactiveSolver(istream& in, ostream& out, vector<vector<int> > target_puzzle, string output_file) {
		dlx_matrix_sudoku solution_matrix(target_puzzle.size());
		vector<vector<int> > user_board = target_puzzle;
		vector<vector<vector<int> > > solutions = solution_matrix.solve(target_puzzle, 1);
		if (solutions.size() < 1) {
			out << "Puzzle is unsolvable.\n";
			return;
		}
		else if (solutions.size() > 1) {
			out << "Puzzle has more than one solution.\n";
			return;
		}
		out << "Type \"help\" for a list of available commands.\n";
		const string input_regex_str = "^(?:(solution)|(check)|(-?[0-9]*)\\s*(-?[0-9]*)\\s*(-?[0-9]*)|(view)|save ([a-zA-Z0-9]+(?:\\.[a-zA-Z0-9]*)?)|(exit)|(help))\\s*";
		const regex input_regex(input_regex_str);
		string line;
		for (string line; getline(in, line);) {
			//cout << "input: " << line << endl;
			smatch user_input;
			if (regex_match(line, user_input, input_regex)) {
				if (user_input[1].matched) { // solution
					out << "Solution printed to " << output_file << "\n";
					visualizeSolution(solutions, output_file);
				}
				else if (user_input[2].matched) { // check
					vector<pair<int, int> > error_squares;
					vector<pair<int, int> > correct_squares;
					for (int row = 0; row < user_board.size(); row++) {
						for (int col = 0; col < user_board[row].size(); col++) {
							if (user_board[row][col] != -1 && user_board[row][col] != solutions[0][row][col]) {
								error_squares.push_back(pair<int, int>(row, col));
							}
							else if (user_board[row][col] == solutions[0][row][col] && target_puzzle[row][col] == -1) {
								correct_squares.push_back(pair<int, int>(row, col));
							}
						}
					}
					if (error_squares.size() == 0 && user_board == solutions[0]) {
						out << "Board is complete and correct, good job!\n";
					}
					out << "Found " << error_squares.size() << " errors and " << correct_squares.size() << " correct placements, view printed to " << output_file << " with correct highlighted in green and errors in red.\n";
					visualizeSolution(vector<vector<vector<int> > >{user_board}, output_file, error_squares, correct_squares);
				}
				else if (user_input[3].matched) { // answer
					int guess = stoi(user_input[3].str());
					int row = stoi(user_input[4].str());
					int col = stoi(user_input[5].str());
					if (row < 1 || row > 9 || col < 1 || col > 9) {
						out << "Input out of range. Row and column values for a guess must be in the range of 1 to " << 9 << ".\n";
					}
					else if (target_puzzle[row-1][col-1] != -1) {
						out << "Input out of range. Guess would overwrite an original value.\n";
					}
					else {
						if (guess >= 1 && guess <= 9) {
							user_board[row-1][col-1] = guess;
							out << "Guess of " << guess << " placed at row " << row << " and column " << col << ".\n";
						}
						else {
							user_board[row-1][col-1] = -1;
							out << "Row " << row << " and column " << col << " cleared.\n";
						}
					}
				}
				else if (user_input[6].matched) { // view
					visualizeSolution(vector<vector<vector<int> > >{user_board}, output_file);
					out << "View printed to " << output_file << ".\n";
				}
				else if (user_input[7].matched) { // save
					ofstream fout;
					fout.open(user_input[7].str());
					if (fout.is_open()) {
						for (int row = 0; row < user_board.size(); row++) {
							for (int col = 0; col < user_board[row].size(); col++) {
								if (user_board[row][col] == -1) {
									fout << "0 ";
								}
								else {
									fout << user_board[row][col] << " ";
								}
							}
							fout << endl;
						}
						fout.close();
						out << "Board state saved to " << user_input[7].str() << ".\n";
					}
					else {
						out << "Could not open file.\n";
					}
				}
				else if (user_input[user_input.size() - 2].matched) { // exit
					return;
				}
				else if (user_input[user_input.size() - 1].matched) { // help
					out << "Command options are:\n"
						"\tsolution - displays the solution for the current puzzle\n"
						"\tcheck - highlights correct and incorrect guesses for the current board in green and red, respectively\n"
						"\t{int int int} - interpretted as \"value row column\", used for making guesses on the current board\n"
						"\t\tnonvalid values will empty the square\n"
						"\tview - prints the current board\n"
						"\tsave {string} - saves the current board to the given file name"
						"\t\tfile name must contain only alphanumeric characters with a period for the file extension\n"
						"\texit - closes the program\n"
						"\thelp - displays a list of commands\n";
				}
				else {
					out << "Unknown command, type \"help\" for a list of commands.\n";
				}
			}
		}
	}
};

// Interprets the contents of the istream as a sudoku board.
// Cells on a row must be separated by spaces, and rows must be separated by newlines.
// The read will stop when the file ends, two newlines are encountered in a row, or an error occurs.
// Empty cells are represented by any value less than 1.
// Returns the filled board or an empty vector<vector<int> > if an error occurs.
vector<vector<int> > getBoardFromStream(istream& in) {
	vector<vector<int> > to_return;
	string line;
	int size = -1;
	int max = 0;
	while (getline(in, line) && !line.empty()) {
		to_return.push_back(vector<int>());
		stringstream sin(line);
		int value;
		while (sin >> value) {
			if (size == -1) {
				if (value > max) {
					max = value;
				}
			}
			else {
				if (value > size) {
					cout << "Error: Invalid value found in input: " << value << ".\n";
					return vector<vector<int> >();
				}
			}
			if (value < 1) {
				value = -1;
			}
			to_return.back().push_back(value);
		}
		if (size == -1) {
			size = to_return.back().size();
			if (sqrt(size) * sqrt(size) != size) {
				cout << sqrt(size) << " " << sqrt(size) * sqrt(size) << endl;
				cout << "Error: Input must have square side lengths.\n";
				return vector<vector<int> >();
			}
			if (max > size) {
				cout << "Error: Invalid value found in input: " << max << ".\n";
				return vector<vector<int> >();
			}
		}
		else if (size != to_return.back().size()) {
			cout << "Error: Each row must have the same length.\n";
			return vector<vector<int> >();
		}
	}
	if (size != to_return.size()) {
		cout << "Error: Board must have equal side lengths.\n";
		return vector<vector<int> >();
	}
	return to_return;
}

int main(int argc, char* argv[]) {

	string progName = string(argv[0]);
	vector<string> args(argc-1);
	for (int i = 0; i < args.size(); i++) {
		args[i] = string(argv[i + 1]);
	}

	vector<vector<int> > target_puzzle;

	if (args.size() >= 1) {
		ifstream fin(args[0]);
		if (fin.is_open()) {
			target_puzzle = getBoardFromStream(fin);
		}
		else {
			cout << "Error: Could not open file " << args[0] << ".\n";
		}
	}
	else if (args.size() == 0) {
		target_puzzle = getBoardFromStream(cin);
	}
	else if(args.size() > 2) {
		cout << progName << " arguments are optional, first must be the name of a textfile containing a valid sudoku board, second must be the name of the output file.\n";
		return 0;
	}

	if (target_puzzle.size() == 0) {
		return 0;
	}

	string file_name = "sudoku.jpg";

	if (args.size() == 2) {
		file_name = args[1];
	}

	SudokuVisualizer visualizer;
	visualizer.interactiveSolver(cin, cout, target_puzzle, file_name);

	return 0;
}