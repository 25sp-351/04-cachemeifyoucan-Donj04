#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inputreader.h"
#include "rodcutsolver.h"

int main(int argc, char* argv[]) {
    if (validateInput(argc, argv)) {
        Vec length_prices;
        size_t file_arg_idx = (argc == 4) ? 3 : 2;
        length_prices       = extractFile(argv[file_arg_idx]);

        if (vec_length(length_prices) == 0) {
            printf("ERROR: File does not contain any valid length values\n");
            return 1;
        }
        char input[100];
        strcpy(input, argv[1]);
        do {
            processInput(length_prices, input);
            if (argc < 4 || strcmp(argv[2], "-q") != 0)
                printf("Enter another length (EOF to exit): ");
            fgets(input, 100, stdin);
        } while (!feof(stdin));

        printf("\n");
        return 0;
    }
    return 1;
}
