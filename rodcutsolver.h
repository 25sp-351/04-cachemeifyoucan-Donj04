#ifndef RODCUTSOLVER_H
#define RODCUTSOLVER_H

#include "vec.h"

typedef struct rodcutsolver {
    Vec length_prices;  // Keypair = {length, price of length}
    Vec cut_list;       // Keypair = {length, number of pieces}
    int result_profit;
    size_t remainder;
} *RodCutSolver;

RodCutSolver createRodCutSolver(const Vec new_prices);

void freeRodCutSolver(RodCutSolver solver);

void setLengthPrices(RodCutSolver solver, const Vec v);

void printOutput(const RodCutSolver solver, const int prices[]);

void solveRodCutting(RodCutSolver solver, size_t rod_length);

#endif
