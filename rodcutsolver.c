#include "rodcutsolver.h"

#include <stdbool.h>
#include <stdio.h>

#include "keypair.h"

// typedef struct rodcutsolver {
//     Vec length_prices;  // Keypair = {length, price of length}
//     Vec cut_list;       // Keypair = {length, number of pieces}
//     int result_profit;
//     size_t remainder;
// } *RodCutSolver;

RodCutSolver createRodCutSolver(const Vec new_prices) {
    RodCutSolver solver   = malloc(sizeof(struct rodcutsolver));
    solver->length_prices = vec_copy(new_prices);
    solver->cut_list      = new_vec(sizeof(KeyPair));
    solver->result_profit = 0;
    solver->remainder     = 0;
    return solver;
}

void freeRodCutSolver(RodCutSolver solver) {
    vec_free(solver->length_prices);
    vec_free(solver->cut_list);
}

void setLengthPrices(RodCutSolver solver, const Vec v) {
    vec_free(solver->length_prices);
    solver->length_prices = vec_copy(v);
}

void setCutList(RodCutSolver solver, size_t rod_length, const size_t cuts[]) {
    vec_free(solver->cut_list);
    solver->cut_list    = new_vec(sizeof(KeyPair));

    size_t temp_length  = rod_length;
    int remaining_loops = rod_length;  // To prevent infinite loops

    while (temp_length > 0 && remaining_loops > 0) {
        const size_t cut = cuts[temp_length];

        if (cut > 0) {
            bool unique = true;

            // Search for the KeyPair with a key matching with cut
            for (size_t ix = 0; ix < vec_length(solver->cut_list); ix++) {
                // Get a pointer to the pair at this index of the vec
                KeyPair* pair = vec_get(solver->cut_list, ix);
                if (pair->key == cut) {
                    pair->value++;
                    unique = false;
                    break;
                }
            }
            if (unique) {
                KeyPair new_pair = createKeyPair(cut, 1);
                vec_add(solver->cut_list, &new_pair);
            }
        }
        temp_length -= cut;
        remaining_loops--;
    }
    solver->remainder = temp_length;
}

void printOutput(const RodCutSolver solver, const int prices[]) {
    // Get each KeyPair in the cut_list vector and print it
    for (size_t ix = 0; ix < vec_length(solver->cut_list); ix++) {
        const KeyPair* pair   = vec_get(solver->cut_list, ix);
        const size_t length   = pair->key;
        const int piece_count = pair->value;
        const int price       = prices[length];

        printf("%d @ %zu = %d\n", piece_count, length, piece_count * price);
    }
    printf("Remainder: %zu\n", solver->remainder);
    printf("Value: %d\n", solver->result_profit);
}

void solveRodCutting(RodCutSolver solver, size_t rod_length) {
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
    for (size_t iy = 0; iy < vec_length(solver->length_prices); iy++) {
        KeyPair* pair = vec_get(solver->length_prices, iy);
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
    solver->result_profit = max_profit[rod_length];
    setCutList(solver, rod_length, cuts);
    printOutput(solver, prices);
}
