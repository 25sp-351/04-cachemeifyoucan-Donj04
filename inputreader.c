#include "inputreader.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "keypair.h"

const size_t MAX_LINE_LENGTH       = 128;
const size_t COMMAND_LINE_ARG_SIZE = 256;
const size_t BUFFER_SIZE           = 64;

const int MIN_ARGS                 = 2;
const int MAX_ARGS                 = 3;

const int FILE_ARG                 = 1;
const int CACHE_ARG                = 2;


int getInput(char* write_to) {
    if (fgets(write_to, BUFFER_SIZE, stdin) != NULL)
        return INPUT_OK;

    if (feof(stdin))
        return USER_EXIT;

    return READ_ERROR;
}

void clearBuffer() {
    char ch = ' ';
    while (ch != '\n' && ch != EOF)
        ch = getchar();
}

bool isArgCountValid(int argc) {
    return argc >= MIN_ARGS && argc <= MAX_ARGS;
}

bool isLengthInRange(long length) {
    return length > 0 && length <= (long)MAX_ROD_LENGTH;
}

bool isFileValid(const char* filename) {
    return access(filename, F_OK) == 0;
}

// Helper function for writeLineToVec()
// Returns true if line is only whitespace or is a comment (begins with '#')
bool isBlankLine(const char* line, size_t length) {
    for (size_t ix = 0; ix < length; ix++) {
        if (line[ix] == '\0' || line[ix] == '#')
            return true;
        if (!isspace(line[ix]))
            return false;
    }
    return true;
}

// Helper function for extractFile()
// Writes the length and price into the vector
// Returns error code if failed
int writeLineToVec(const char* line, Vec add_to) {
    if (isBlankLine(line, MAX_LINE_LENGTH))
        return FILE_LINE_OK;

    long read_length = 0;
    int read_value   = 0;
    char line_copy[MAX_LINE_LENGTH];
    copyWithoutNewline(line, line_copy, MAX_LINE_LENGTH);

    // Write input to length and value and check if successful for both
    if (sscanf(line_copy, "%ld , %d", &read_length, &read_value) == 2) {
        if (!isLengthInRange(read_length))
            return FILE_LENGTH_OUT_OF_RANGE;

        // Check for duplicate lengths
        for (size_t ix = 0; ix < vec_length(add_to); ix++) {
            KeyPair* pair_p = vec_get(add_to, ix);
            if (pair_p->key == (size_t)read_length)
                return FILE_LENGTH_DUPE;
        }
        KeyPair new_pair = createKeyPair(read_length, read_value);
        vec_add(add_to, &new_pair);
        printf("Added length %2ld, value %2d\n", read_length, read_value);
        return FILE_LINE_OK;
    }
    return FILE_INVALID_LINE;
}

Vec extractFile(const char* filename) {
    if (!isFileValid(filename))
        return NULL;

    FILE* file  = fopen(filename, "r");
    Vec lengths = new_vec(sizeof(KeyPair));
    char line[MAX_LINE_LENGTH];

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        int read_state = writeLineToVec(line, lengths);
        if (read_state != FILE_LINE_OK)
            printErr(read_state, line, MAX_LINE_LENGTH);
    }

    fclose(file);
    return lengths;
}

int writeInputToInt(const char* input, long* write_to) {
    if (sscanf(input, "%ld", write_to) != 1)
        return INPUT_NOT_INT;

    if (!isLengthInRange(*write_to))
        return INPUT_OUT_OF_RANGE;

    return INPUT_OK;
}

void copyWithoutNewline(const char* from, char* write_to, size_t length) {
    for (size_t ix = 0; ix < length; ix++) {
        if (from[ix] == '\n' || from[ix] == '\0' || ix == length - 1) {

            write_to[ix] = '\0';
            break;
        }
        write_to[ix] = from[ix];
    }
}

void printErr(int err, const char* input, size_t max_length) {
    char input_copy[max_length];
    copyWithoutNewline(input, input_copy, max_length);

    switch (err) {
        case 0:  // ARGS_OK, FILE_LINE_OK, INPUT_OK
            break;

        case ARG_COUNT_INVALID:
            fprintf(stderr, "Usage: %s lengths_file.txt [cache.so]\n",
                    input_copy);
            break;

        case FILE_INVALID:
            fprintf(stderr,
                    "Error: File path '%s' is invalid or does not exist\n",
                    input_copy);
            break;

        case CACHE_INVALID:
            fprintf(stderr, "Error: Failed to load cache module '%s'\n",
                    input_copy);
            break;

        case FLAG_INVALID:
            fprintf(stderr, "Error: Unknown flag '%s'\n", input_copy);
            break;

        case FILE_NO_VALID_LINES:
            fprintf(stderr, "Error: No valid lengths found in '%s'\n",
                    input_copy);
            break;

        case FILE_LENGTH_OUT_OF_RANGE:
            fprintf(
                stderr,
                "Warning: length in '%s' is out of range. Ignoring line...\n",
                input_copy);
            break;

        case FILE_LENGTH_DUPE:
            fprintf(
                stderr,
                "Warning: length in '%s' is a duplicate. Ignoring line...\n",
                input_copy);
            break;

        case FILE_INVALID_LINE:
            fprintf(stderr,
                    "Warning: line '%s' should be formatted as <int>, <int>. "
                    "Ignoring line...\n",
                    input_copy);
            break;

        case INPUT_NOT_INT:
            fprintf(stderr,
                    "Error: '%s' could not be converted to an integer\n",
                    input_copy);
            break;

        case INPUT_OUT_OF_RANGE:
            fprintf(stderr,
                    "Error: '%s' should be an integer between 1 and %d\n",
                    input_copy, MAX_ROD_LENGTH);
            break;

        case READ_ERROR:
            fprintf(stderr, "Error: Could not read rod length from user\n");
            break;

        default:
            fprintf(stderr, "Error: Received error code %d with input %s\n",
                    err, input_copy);
    }
}
