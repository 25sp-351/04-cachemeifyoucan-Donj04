#ifndef RODCUTSOLVER_H
#define RODCUTSOLVER_H

#include "vec.h"

extern const size_t MAX_OUTPUT_LENGTH;

Vec createCutList(size_t rod_length, const size_t cuts[]);

size_t calculateRemainder(const Vec cuts, size_t rod_length);

// Solves rodcut problem and returns a string of the results
// String will need to be freed by the caller
char* solveRodCutting(const Vec length_prices, size_t rod_length);

// String is dynamically allocated, so it will need to be freed
char* getOutputStr(const Vec length_prices, const Vec cut_list, int profit,
                   size_t remainder);

#endif
