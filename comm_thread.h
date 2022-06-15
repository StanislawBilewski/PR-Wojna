#ifndef COM_THREADH
#define COM_THREADH

#include <mpi.h>

#include "constants.h"
#include "structures.h"
#include "main.h"
#include <algorithm>

void *comLoop(void *ptr);
bool checkPriority(int, int);

#endif