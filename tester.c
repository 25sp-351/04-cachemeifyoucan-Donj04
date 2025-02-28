#include <bsd/stdio.h>
#include <bsd/stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "inputreader.h"
#include "rodcutsolver.h"
#include "vec.h"

/*
** This is a cache tester for NON-RECURSIVE functions.
** It does not support the provider function accessing the cache.
*/

#define TEST_COUNT 100
#define MAX_TEST_NUMBER 100


int rand_between(int min, int max);


int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s lengths_file.txt [cache.so]\n", argv[0]);
        return 1;
    }

    // base (real) function
    ProviderFunction get_me_a_value = solveRodCutting;

    bool cache_installed            = argc > 2;
    Cache *cache                    = NULL;

    if (cache_installed) {
        cache = load_cache_module(argv[2]);

        if (cache == NULL) {
            fprintf(stderr, "Failed to load cache module\n");
            return 1;
        }
        // replace our real provider with a caching provider
        get_me_a_value = cache->set_provider_func(get_me_a_value);
    }

    printf("\nReading file '%s'...\n", argv[1]);
    Vec lengths = extractFile(argv[1]);

    if (lengths == NULL || vec_length(lengths) == 0) {
        fprintf(stderr, "File is invalid or contains no valid lengths\n");
        if (lengths != NULL)
            vec_free(lengths);
        return 1;
    }

    // get a random key, and get the value associated with it
    // then get the value a second time to test if cached correctly
    for (int test_number = 0; test_number < TEST_COUNT; test_number++) {
        int randomnumber = rand_between(1, MAX_TEST_NUMBER);

        printf("\n=================================\n");
        printf("\nBeginning test %2d-1: %d\n", test_number, randomnumber);

        ValueType result = get_me_a_value(lengths, randomnumber);

        printf("Done with test %2d-1: Rod length %d solution:\n%s", test_number,
               randomnumber, result);

        printf("\nBeginning test %2d-2: %d\n", test_number, randomnumber);

        result = get_me_a_value(lengths, randomnumber);

        printf("Done with test %2d-2: Rod length %d solution:\n%s", test_number,
               randomnumber, result);

        // if (cache != NULL && test_number == TEST_COUNT / 2) {
        //     printf("Taking a break. Resetting cache statistics.\n");
        //     cache->reset_statistics();
        // }
    }

    if (cache_installed) {
        printf("\n\n");

        CacheStat *list_of_stats = cache->get_statistics();
        print_cache_stats(fileno(stdout), list_of_stats);

        if (list_of_stats)
            free(list_of_stats);

        printf("\n\n");

        cache->cache_cleanup();
        free(cache);
    }

    vec_free(lengths);
}

int rand_between(int min, int max) {
    int range = max - min;
    return min + arc4random_uniform(range);
}
