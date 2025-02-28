#ifndef INPUTREADER_H
#define INPUTREADER_H

#include <stdbool.h>
#include <stdlib.h>

#include "vec.h"

#define MAX_ROD_LENGTH 100000

#define ARGS_OK 0
#define ARG_COUNT_INVALID 1

#define FILE_LINE_OK 0
#define FILE_INVALID 2
#define FILE_NO_VALID_LINES 3
#define FILE_LENGTH_OUT_OF_RANGE 4
#define FILE_LENGTH_DUPE 5
#define FILE_INVALID_LINE 6

#define INPUT_OK 0
#define INPUT_NOT_INT 7
#define INPUT_OUT_OF_RANGE 8
#define READ_ERROR 9

#define CACHE_INVALID 10

#define FLAG_INVALID 11

#define USER_EXIT 12

extern const size_t MAX_LINE_LENGTH;
extern const size_t COMMAND_LINE_ARG_SIZE;
extern const size_t BUFFER_SIZE;

extern const int MIN_ARGS;
extern const int MAX_ARGS;

extern const int FILE_ARG;
extern const int CACHE_ARG;


// Returns a success code if successfully wrote input,
// returns an error code if failed,
// or returns an exit code if EOF is read
int getInput(char* write_to);

// Clear the buffer in stdin
// This prevents a bug where user inputs longer than BUFFER_SIZE will cause the
// input loop to execute multiple times automatically
void clearBuffer();

// Returns true if there are not too few or too many arguments
bool isArgCountValid(int argc);

// Returns true if 0 < length < INT_MAX
bool isLengthInRange(long length);

// Returns true if file exists and can be accessed
bool isFileValid(const char* filename);

// Reads each line from a file and returns a vector of length value pairs
// Returns NULL if file cannot be read
Vec extractFile(const char* filename);

// Write input string as a long int to write_to
// Returns error code if failed or num is out of range
int writeInputToInt(const char* input, long* write_to);

// Writes a copy of a string without a newline into write_to
void copyWithoutNewline(const char* from, char* write_to, size_t length);

// Prints an error message given the error code, the input causing the error,
// and the max length of the input
void printErr(int err, const char* input, size_t max_length);

#endif
