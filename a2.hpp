#ifndef __A2_H__   
#define __A2_H__


#include <iostream>
#include <map>
#include <random>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stack> 
#include <ctime> 
#include <iomanip>
#include <chrono>
#include <cmath>
#include <climits>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <utility>      

using namespace std;

const int BILLION = 1000000000L;

const int BLACK = 0;
const int WHITE = 1;
const int EMPTY = 2;
const int OFFBOARD = 3;
const int NUM_COLORS = 3;


int oppositeColor(int curr){
    if (curr == BLACK)
        return WHITE;
    else if (curr == WHITE)
        return BLACK;
    else 
        return -1;
}

string showBoard(int* board, int boardsize){
	string s = "";
	int digits = boardsize > 0 ? (int) log10 ((double) boardsize) + 1 : 1;
	s.append(digits + 1, ' ');
	char col = 'a';

	for (int i = 0; i < boardsize; i++){
		s.append(" ");
		s.append(1, col);
		col++;
	}
	s.append("\n \n");

	int n = boardsize;
	int board_index = n * n + 3 * (n + 1) - 1;
	board_index -= n + 1 + n + 1;
	for (int i = boardsize; i > 0; i--){
		int digits_i = i > 0 ? (int) log10 ((double) i) + 1 : 1;
		s.append(digits - digits_i, ' ');
		s.append(to_string(i));
		s.append(" ");
		for (int j = 0; j < boardsize; j++){
			s.append(" ");
			if (board[board_index] == WHITE){
				s.append("O");
			} else if (board[board_index] == BLACK){
				s.append("X");
			} else if (board[board_index] == EMPTY){
				s.append(".");
			} else if (board[board_index] == OFFBOARD){
				s.append(" ");
			} else {
				s.append("e");
			}
			board_index++; 
		}
		board_index -= n;
		board_index -= n + 1;
		s.append("  ");
		s.append(digits - digits_i, ' ');
		s.append(to_string(i));
		s.append("\n");
	}

	s.append(" \n");

	s.append(digits + 1, ' ');
	col = 'a';
	for (int i = 0; i < boardsize; i++){
		s.append(" ");
		s.append(1, col);
		col++;
	}
	s.append("\n\n");

	return s;
}

int compareInt(const void * a, const void * b)
{
	return - ( *(int*)a - *(int*)b );
}



#endif