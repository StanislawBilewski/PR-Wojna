#ifndef GLOBALH
#define GLOBALH

#include <mpi.h>
#include <vector>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "constants.h"
#include "structures.h"

#define println(FORMAT, ...) printf("%c[%d;%dm[r: %d,\tt: %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(data.rank/7))%2, 31+(6+data.rank)%7, data.rank, data.lamportTime, ##__VA_ARGS__, 27,0,37);

extern Data data;   // klasa przechowująca zmienne lokalne procesu

extern MPI_Datatype MPI_PACKET_T;

void incLamportTime(int received);

void lockStateMutex();
void unlockStateMutex();

#endif
