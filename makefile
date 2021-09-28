CPPFLAGS=-O3 -std=c++11

all: main.cpp DLX.h JGraph.h
	g++ $(CPPFLAGS) -o SudokuVisualizer main.cpp DLX.h JGraph.h
test: all
	./SudokuVisualizer test_inputs/0.txt test_output0.jpg < test_inputs/0console.txt
	./SudokuVisualizer test_inputs/1.txt test_output1.jpg < test_inputs/1console.txt
	./SudokuVisualizer test_inputs/2.txt test_output2.jpg < test_inputs/2console.txt
	./SudokuVisualizer test_inputs/3.txt test_output3.jpg < test_inputs/3console.txt
	./SudokuVisualizer test_inputs/4.txt test_output4.jpg < test_inputs/4console.txt
