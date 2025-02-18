#include "inputreader.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "keypair.h"

const size_t MAX_LINE_LENGTH       = 128;
const size_t COMMAND_LINE_ARG_SIZE = 256;
const size_t BUFFER_SIZE           = 64;

int getInput(char* write_to) {
    printf("Enter a rod length (EOF to exit): ");

    if (fgets(write_to, BUFFER_SIZE, stdin) != NULL)
        return INPUT_OK;

    if (feof(stdin))
        return USER_EXIT;

    return READ_ERROR;
}

int validateArgs(int arg_count, char* args[]) {
    if (arg_count != 2)
        return ARG_COUNT_INVALID;

    if (!isFileValid(args[1]))
        return FILE_INVALID;

    return ARGS_OK;
}

bool isLengthInRange(long length) {
    return length > 0 && length < INT_MAX;
}

bool isFileValid(const char* filename) {
    return access(filename, F_OK) == 0;
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

int writeLineToVec(const char* line, Vec add_to) {
    if (isBlankLine(line, MAX_LINE_LENGTH))
        return FILE_LINE_OK;

    long read_length = 0;
    int read_value   = 0;
    char line_copy[MAX_LINE_LENGTH];
    trimNewline(line, line_copy, MAX_LINE_LENGTH);

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
        return FILE_LINE_OK;
    }
    return FILE_INVALID_LINE;
}

bool isBlankLine(const char* line, size_t length) {
    for (size_t ix = 0; ix < length; ix++) {
        if (line[ix] == '\0' || line[ix] == '#')
            return true;
        if (!isspace(line[ix]))
            return false;
    }
    return true;
}

int writeInputToInt(const char* input, long* write_to) {
    if (sscanf(input, "%ld", write_to) != 1)
        return INPUT_NOT_INT;

    if (!isLengthInRange(*write_to))
        return INPUT_OUT_OF_RANGE;

    return INPUT_OK;
}

void trimNewline(const char* text, char* write_to, size_t length) {
    for (size_t ix = 0; ix < length; ix++) {
        if (text[ix] == '\n' || text[ix] == '\0' || ix == length - 1) {
            write_to[ix] = '\0';
            break;
        }
        write_to[ix] = text[ix];
    }
}

void printErr(int err, const char* input, size_t input_length) {
    char input_copy[input_length];
    trimNewline(input, input_copy, input_length);

    switch (err) {
        case 0:  // ARGS_OK, FILE_LINE_OK, INPUT_OK
            break;

        case ARG_COUNT_INVALID:
            printf("Usage: ./rodcut /path/to/file\n");
            break;

        case FILE_INVALID:
            printf("Error: File path '%s' is invalid or does not exist\n",
                   input_copy);
            break;

        case FILE_NO_VALID_LINES:
            printf("Error: No valid lengths found in '%s'\n", input_copy);
            break;

        case FILE_LENGTH_OUT_OF_RANGE:
            printf(
                "Warning: length in '%s' is out of range. Ignoring line...\n",
                input_copy);
            break;

        case FILE_LENGTH_DUPE:
            printf("Warning: length in '%s' is a duplicate. Ignoring line...\n",
                   input_copy);
            break;

        case FILE_INVALID_LINE:
            printf(
                "Warning: line '%s' is invalid. Format should be <int>, <int>. "
                "Ignoring line...\n",
                input_copy);
            break;

        case INPUT_NOT_INT:
            printf("Error: '%s' could not be converted to an integer\n",
                   input_copy);
            break;

        case INPUT_OUT_OF_RANGE:
            printf(
                "Error: '%s' is out of range. Length should be positive and "
                "fit "
                "within a signed int (%d)\n",
                input_copy, INT_MAX);
            break;

        case READ_ERROR:
            printf("Error: Could not read rod length from user\n");
            break;

        default:
            printf("Error: Received error code %d with input %s\n", err,
                   input_copy);
    }
}

// void processInput(const Vec prices, const char* input) {
//     long length = getIntFromStr(input);
//     if (isLengthInRange(length)) {
//         RodCutSolver solver = createRodCutSolver(prices);
//         solveRodCutting(solver, (int)length);
//         freeRodCutSolver(solver);
//         return;
//     }
//     printf("Rod length must be a 32-bit integer greater than 0.\n");
// }
