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
extern const int DEBUG_ARG;

extern const char* DEBUG_FLAG;

// Returns error code if failed to read input, or sends exit code if EOF is read
int getInput(char* write_to);

void clearBuffer();

bool isArgCountValid(int argc);

bool isFlagValid(const char* flag);

// Returns true if 0 < length < INT_MAX
bool isLengthInRange(long length);

// Returns true if file exists and can be accessed
bool isFileValid(const char* filename);

// Reads each line from a file and returns a vector of length value pairs
Vec extractFile(const char* filename);

// Add length and value to vector, return error code if failed
int writeLineToVec(const char* line, Vec add_to);

// Returns true if line is only whitespace or is a comment (begins with '#')
bool isBlankLine(const char* line, size_t length);

// Write input string as int to write_to, return error code if failed or num is
// out of range
int writeInputToInt(const char* input, long* write_to);

// Copies text without newline into write_to
void trimNewline(const char* text, char* write_to, size_t length);

void printErr(int err, const char* input, size_t input_length);

#endif
