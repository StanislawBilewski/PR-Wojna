#ifndef COM_THREADH
#define COM_THREADH

#include <mpi.h>

#include "constants.h"
#include "structures.h"
#include "main.h"

void *comLoop(void *ptr);

#endif