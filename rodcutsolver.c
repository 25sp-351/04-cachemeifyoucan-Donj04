#include "rodcutsolver.h"

#include <stdbool.h>
#include <stdio.h>

#include "keypair.h"
#include "vec.h"

// typedef struct rodcutresult {
//     Vec cut_list;  // Keypair = {length, number of pieces}
//     int total_profit;
//     size_t remainder;
// } *RodCutResult;

RodCutResult createRodCutResult(const Vec cuts, int profit, size_t remainder) {
    RodCutResult result  = malloc(sizeof(struct rodcutresult));
    result->cut_list     = vec_copy(cuts);
    result->total_profit = profit;
    result->remainder    = remainder;
    return result;
}

void freeRodCutResult(RodCutResult result) {
    if (result)
        vec_free(result->cut_list);
    free(result);
}

Vec createCutList(size_t rod_length, const size_t cuts[]) {
    Vec cut_list        = new_vec(sizeof(KeyPair));
    size_t temp_length  = rod_length;
    int remaining_loops = rod_length;  // To prevent infinite loops

    while (temp_length > 0 && remaining_loops > 0) {
        const size_t cut = cuts[temp_length];

        if (cut > 0) {
            KeyPair* pair = findPair(cut_list, cut);
            if (pair != NULL) {
                pair->value++;
            } else {
                KeyPair new_pair = createKeyPair(cut, 1);
                vec_add(cut_list, &new_pair);
            }
        }
        temp_length -= cut;
        remaining_loops--;
    }
    return cut_list;
}

size_t calculateRemainder(const Vec cuts, size_t rod_length) {
    size_t temp_length = rod_length;

    for (size_t ix = 0; ix < vec_length(cuts); ix++) {
        KeyPair* pair = vec_get(cuts, ix);
        if (pair != NULL)
            temp_length -= pair->key * pair->value;
    }
    return temp_length;
}

RodCutResult solveRodCutting(const Vec length_prices, size_t rod_length) {
    const size_t arr_size = rod_length + 1;
    int prices[arr_size];
    int max_profit[arr_size];
    size_t cuts[arr_size];

    // Initialize elements of arrays
    for (size_t ix = 0; ix < arr_size; ix++) {
        prices[ix]     = 0;
        max_profit[ix] = 0;
        cuts[ix]       = 0;
    }
    // Set length prices. Each index in prices[] corresponds to a length
    for (size_t iy = 0; iy < vec_length(length_prices); iy++) {
        KeyPair* pair = vec_get(length_prices, iy);
        if (pair->key < arr_size)
            prices[pair->key] = pair->value;
    }

    // Algorithm to solve rod cutting problem
    for (size_t first_cut = 1; first_cut <= rod_length; first_cut++) {
        int curr_max = 0;
        int best_cut = 0;

        for (size_t sub_cut = 1; sub_cut <= first_cut; sub_cut++) {
            int profit = prices[sub_cut] + max_profit[first_cut - sub_cut];
            if (prices[sub_cut] > 0 && profit > curr_max) {
                curr_max = profit;
                best_cut = sub_cut;
            }
        }
        max_profit[first_cut] = curr_max;
        cuts[first_cut]       = best_cut;
    }

    const Vec cut_list     = createCutList(rod_length, cuts);
    const int profit       = max_profit[rod_length];
    const size_t remainder = calculateRemainder(cut_list, rod_length);

    return createRodCutResult(cut_list, profit, remainder);
}

void printResult(const RodCutResult result, const Vec length_prices) {
    // Get each KeyPair in the cut_list vector and print it
    for (size_t ix = 0; ix < vec_length(result->cut_list); ix++) {
        const KeyPair* cut        = vec_get(result->cut_list, ix);

        const size_t cut_length   = cut->key;
        const int cut_count       = cut->value;

        const KeyPair* price_pair = findPair(length_prices, cut_length);

        if (price_pair != NULL) {
            int cut_price = price_pair->value;
            printf("%d @ %zu = %d\n", cut_count, cut_length,
                   cut_count * cut_price);
        }
    }
    printf("Remainder: %zu\n", result->remainder);
    printf("Value: %d\n", result->total_profit);
}
