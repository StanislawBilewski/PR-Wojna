#ifndef GLOBALH
#define GLOBALH

#pragma once
#include <mpi.h>
#include <vector>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>

#include "constants.h"
#include "structures.h"

#define println(FORMAT, ...) printf("%c[%d;%dm[r: %d,\tt: %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(mainData.rank/7))%2, 31+(6+mainData.rank)%7, mainData.rank, mainData.lamportTime, ##__VA_ARGS__, 27,0,37);

extern Data mainData;   // klasa przechowująca zmienne lokalne procesu

extern MPI_Datatype MPI_PACKET_T;

void incLamportTime(int received);

void lockMutex();
void unlockMutex();

#endif
