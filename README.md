# CS594Lab1
Lab Assignment 1 for CS594 Fall 2021

This program may be compiled by running the included makefile or using:

```
g++ main.cpp DLX.h JGraph.h -o SudokuVisualizer -O3 -std=c++11
```

The compiled binary may be executed using

```
./SudokuVisualizer [input_file] [output_file]
```

where input_file is the name of an input file containing a standard sudoku puzzle with rows separated by spaces and columns separated by newlines, and output file is the desired name of the generated images.

Once the program is running and has been provided a board, the following commands may be used to interact with the puzzle:

```
solution - displays the solution for the current puzzle
check - highlights correct and incorrect guesses for the current board in green and red, respectively
{int int int} - interpretted as \"value row column\", used for making guesses on the current board
nonvalid values will empty the square
view - prints the current board
save {string} - saves the current board to the given file name
file name must contain only alphanumeric characters with a period for the file extension
exit - closes the program
help - displays a list of commands
```

Additionally, the makefile may be made to automatically generate 5 test sudoku boards using

```
make test
```
