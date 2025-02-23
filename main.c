#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "inputreader.h"
#include "rodcutsolver.h"

void processLengths(bool cache_installed, ProviderFunction provider,
                    Vec length_prices);

int main(int argc, char* argv[]) {
    if (!isArgCountValid(argc)) {
        printErr(ARG_COUNT_INVALID, argv[0], COMMAND_LINE_ARG_SIZE);
        return 1;
    }

    const char* filename     = argv[FILE_ARG];
    const char* cache_module = argv[CACHE_ARG];
    const char* debug_flag   = argv[DEBUG_ARG];
    bool debug_on            = false;

    // TODO: seperate into functions maybe

    bool has_flag = argc > DEBUG_ARG;
    if (has_flag) {
        if (!isFlagValid(debug_flag)) {
            printErr(FLAG_INVALID, debug_flag, BUFFER_SIZE);
            return 1;
        }
        debug_on = true;
    }

    ProviderFunction provider = solveRodCutting;

    bool cache_installed      = argc > CACHE_ARG;
    Cache* cache              = NULL;

    if (cache_installed) {
        cache = load_cache_module(cache_module);

        if (cache == NULL) {
            printErr(CACHE_INVALID, cache_module, COMMAND_LINE_ARG_SIZE);
            return 1;
        }

        provider               = cache->set_provider_func(provider);
        *(cache->show_debug_info) = debug_on;

        printf("Cache loaded\n\n");
    }

    printf("Reading lengths from '%s'...\n", filename);

    const Vec length_prices = extractFile(filename);

    printf("\n");

    if (length_prices == NULL) {
        printErr(FILE_INVALID, filename, COMMAND_LINE_ARG_SIZE);
        return 1;
    }
    if (vec_length(length_prices) == 0) {
        printErr(FILE_NO_VALID_LINES, filename, COMMAND_LINE_ARG_SIZE);
        vec_free(length_prices);
        return 1;
    }

    processLengths(cache_installed, provider, length_prices);

    if (cache_installed && debug_on) {
        printf("\n\n");

        CacheStat* list_of_stats = cache->get_statistics();
        print_cache_stats(fileno(stdout), list_of_stats);

        if (list_of_stats)
            free(list_of_stats);

        printf("\n\n");

        cache->cache_cleanup();
        free(cache);
    }

    vec_free(length_prices);
    return 0;
}

void processLengths(bool cache_installed, ProviderFunction provider,
                    Vec length_prices) {
    while (true) {
        printf("Enter a rod length (EOF to exit): ");

        char buffer[BUFFER_SIZE];
        int input_state = getInput(buffer);

        if (input_state == USER_EXIT) {
            printf("\n");
            return;
        }

        if (input_state == INPUT_OK) {
            long rod_length;
            int write_state = writeInputToInt(buffer, &rod_length);

            if (write_state != INPUT_OK) {
                printErr(write_state, buffer, BUFFER_SIZE);

            } else {
                char* results = provider(length_prices, (size_t)rod_length);
                printf("%s", results);
                if (!cache_installed)
                    free(results);
            }

        } else {
            printErr(input_state, buffer, BUFFER_SIZE);
        }

        // Clear buffer if full
        if (strnlen(buffer, BUFFER_SIZE) == BUFFER_SIZE - 1)
            clearBuffer();
    }
}
