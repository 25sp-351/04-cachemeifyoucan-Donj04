#ifndef FILE_READER_H
#define FILE_READER_H

#include <stdbool.h>

#include "rodcutsolver.h"
#include "vec.h"

extern size_t MAX_LINE_LENGTH;

bool isBlankLine(const char* line, size_t length);

void trimNewline(char* text);

// Attempts to return an input string as an integer
long int writeLength(const char* input);

// Returns true if 0 < length < INT_MAX
bool validateLength(long int length);

// Returns true if file exists and can be accessed
bool validateFile(const char* filename);

// Returns true if there are 3 or 4 command-line arguments and they are all
// valid
bool validateInput(int arg_count, char* args[]);

// Reads each line from a file and returns a vector of length value pairs
Vec extractFile(const char* filename);

// Validates the given input length and solves rod cutting problem
void processInput(const Vec prices, const char* input);

#endif
