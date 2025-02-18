#ifndef RODCUTSOLVER_H
#define RODCUTSOLVER_H

#include "vec.h"

typedef struct rodcutresult {
    Vec cut_list;  // Keypair = {length, number of pieces}
    int total_profit;
    size_t remainder;
} *RodCutResult;

RodCutResult createRodCutResult(const Vec cuts, int profit, size_t remainder);

void freeRodCutResult(RodCutResult result);

Vec createCutList(size_t rod_length, const size_t cuts[]);

size_t calculateRemainder(const Vec cuts, size_t rod_length);

RodCutResult solveRodCutting(const Vec length_prices, size_t rod_length);

void printResult(const RodCutResult result, const Vec length_prices);

#endif
