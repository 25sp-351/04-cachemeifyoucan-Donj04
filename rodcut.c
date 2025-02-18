#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inputreader.h"
#include "rodcutsolver.h"

int main(int argc, char* argv[]) {
    // initalize a vector a length price pairs by extracting the input file

    // if no valid lengths in the file, return error

    // while input != EOF:
    // ask for length, get length
    // run rodcut solving, cache results

    int state = validateArgs(argc, argv);
    char* filename = argv[1];

    if (state != ARGS_OK) {
        if (state == ARG_COUNT_INVALID)
            printErr(state, "", COMMAND_LINE_ARG_SIZE);
        else
            printErr(state, filename, COMMAND_LINE_ARG_SIZE);
        return 1;
    }

    Vec length_prices = extractFile(filename);

    if (vec_length(length_prices) == 0) {
        printErr(FILE_NO_VALID_LINES, filename, COMMAND_LINE_ARG_SIZE);
        return 2;
    }

    while (true) {
        char buffer[BUFFER_SIZE];
        int input_state = getInput(buffer);

        if (input_state == USER_EXIT)
            break;

        if (input_state == INPUT_OK) {
            long rod_length;
            int write_state = writeInputToInt(buffer, &rod_length);

            if (write_state != INPUT_OK) {
                printErr(write_state, buffer, BUFFER_SIZE);
                continue;
            }
            // TODO: implement cache
            RodCutResult results = solveRodCutting(length_prices, (size_t)rod_length);

            printResult(results, length_prices);

            freeRodCutResult(results); // might not want this if caching
            continue;
        }
        printErr(input_state, buffer, BUFFER_SIZE);
    }

    printf("\n");
    return 0;
}
