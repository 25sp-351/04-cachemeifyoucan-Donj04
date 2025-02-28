#ifndef RODCUTSOLVER_H
#define RODCUTSOLVER_H

#include <stdlib.h>

#include "vec.h"

extern const size_t MAX_OUTPUT_LENGTH;


// Returns an allocated string of the solution to the rod cutting problem
// Takes a list of possible lengths and prices, and a rod length to cut
// Returned string will need to be freed by the caller
char* solveRodCutting(const Vec length_prices, size_t rod_length);

#endif
