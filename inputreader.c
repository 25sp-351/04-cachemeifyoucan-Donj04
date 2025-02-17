#include "inputreader.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "keypair.h"

size_t MAX_LINE_LENGTH = 128;

bool isBlankLine(const char* line, size_t length) {
    for (size_t ix = 0; ix < length; ix++) {
        if (line[ix] == '\0' || line[ix] == '#')
            return true;
        if (isspace(line[ix]))
            continue;
        return false;
    }
    return true;
}

void trimNewline(char* text) {
    char* ptr        = text;

    size_t max_loops = MAX_LINE_LENGTH;
    while (*ptr != '\0' && max_loops > 0) {  // Move pointer to end of string
        ptr++;
        max_loops--;
    }
    if (*(ptr - 1) == '\n')
        *(ptr - 1) = '\0';
}

long int writeLength(const char* input) {
    long int length = 0;
    sscanf(input, "%ld", &length);
    return length;
}

bool validateLength(long int length) {
    return length > 0 && length < INT_MAX;
}

bool validateFile(const char* filename) {
    return access(filename, F_OK) == 0;
}

bool validateInput(int arg_count, char* args[]) {
    const bool has_4_args = arg_count == 4;

    if (arg_count == 3 || has_4_args) {
        const char* file_arg    = (has_4_args) ? args[3] : args[2];
        const char* q_arg       = args[2];
        const bool length_valid = validateLength(writeLength(args[1]));
        const bool file_valid   = validateFile(file_arg);
        const bool q_arg_valid  = strcmp(q_arg, "-q") == 0;

        if (!length_valid)
            printf(
                "ERROR: \"%s\" is an invalid length input. "
                "Rod length must be a 32-bit integer greater than 0.\n",
                args[1]);
        if (!file_valid)
            printf("ERROR: \"%s\" is not a valid file path.\n", file_arg);
        if (has_4_args && !q_arg_valid)
            printf("ERROR: Unknown parameter \'%s\'\n", q_arg);

        if (has_4_args)
            return length_valid && file_valid && q_arg_valid;
        else
            return length_valid && file_valid;
    }
    printf(
        "Usage: %s length [-q] /path/to/lengths.txt\n"
        "length = integer > 0\n"
        "-q = Disable prompt\n",
        args[0]);
    return false;
}

void readLine(char* line, Vec length_list, bool show_warnings) {
    if (isBlankLine(line, MAX_LINE_LENGTH))
        return;

    long int read_length = 0;
    int read_value       = 0;
    char* line_copy      = line;
    trimNewline(line_copy);

    // Write input to length and value and check if successful for both
    if (sscanf(line_copy, "%ld , %d", &read_length, &read_value) == 2) {
        if (!validateLength(read_length)) {
            if (show_warnings)
                printf(
                    "WARNING: length %ld should be greater than 0 and less "
                    "than %d\n",
                    read_length, INT_MAX);
            return;
        }
        // Check for duplicate lengths
        for (size_t ix = 0; ix < vec_length(length_list); ix++) {
            KeyPair* pair_p = vec_get(length_list, ix);
            if (pair_p->key == (size_t)read_length) {
                if (show_warnings)
                    printf("WARNING: %ld is a duplicate length\n", read_length);
                return;
            }
        }
        KeyPair new_pair = createKeyPair(read_length, read_value);
        vec_add(length_list, &new_pair);
        return;
    }
    if (show_warnings) {
        printf(
            "WARNING: line \"%s\" is invalid. Format should be "
            "<int>, <int>\n",
            line_copy);
    }
}

Vec extractFile(const char* filename) {
    FILE* file  = fopen(filename, "r");
    Vec lengths = new_vec(sizeof(KeyPair));

    if (file != NULL) {
        char line[MAX_LINE_LENGTH];

        while (fgets(line, MAX_LINE_LENGTH, file))
            readLine(line, lengths, true);
    } else
        printf("Error opening %s\n", filename);
    fclose(file);
    return lengths;
}

void processInput(const Vec prices, const char* input) {
    long int length = writeLength(input);
    if (validateLength(length)) {
        RodCutSolver solver = createRodCutSolver(prices);
        solveRodCutting(solver, (int)length);
        freeRodCutSolver(solver);
        return;
    }
    printf("Rod length must be a 32-bit integer greater than 0.\n");
}
