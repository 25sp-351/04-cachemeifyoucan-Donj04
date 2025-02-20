#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inputreader.h"
#include "rodcutsolver.h"

int main(int argc, char* argv[]) {
    int arg_state = validateArgs(argc, argv);

    if (arg_state != ARGS_OK) {
        if (arg_state == ARG_COUNT_INVALID)
            printErr(arg_state, argv[0], COMMAND_LINE_ARG_SIZE);
        else
            printErr(arg_state, argv[1], COMMAND_LINE_ARG_SIZE);
        return 1;
    }

    char* filename    = argv[1];
    Vec length_prices = extractFile(filename);

    if (vec_length(length_prices) == 0) {
        printErr(FILE_NO_VALID_LINES, filename, COMMAND_LINE_ARG_SIZE);
        return 1;
    }

    while (true) {
        printf("Enter a rod length (EOF to exit): ");

        char buffer[BUFFER_SIZE];
        int input_state = getInput(buffer);

        if (input_state == USER_EXIT)
            break;

        if (input_state == INPUT_OK) {
            long rod_length;
            int write_state = writeInputToInt(buffer, &rod_length);

            if (write_state != INPUT_OK) {
                printErr(write_state, buffer, BUFFER_SIZE);

            } else {
                // TODO: implement cache
                char* results =
                    solveRodCutting(length_prices, (size_t)rod_length);
                printf("%s", results);
                free(results);
            }

        } else {
            printErr(input_state, buffer, BUFFER_SIZE);
        }

        // Clear buffer if full
        if (strnlen(buffer, BUFFER_SIZE) == BUFFER_SIZE - 1)
            clearBuffer();
    }
    printf("\n");
    vec_free(length_prices);
    return 0;
}
