#include "main.h"
#include "main_thread.h"

MPI_Datatype MPI_PACKET_T;

Data mainData;

pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;

void lockMutex() {
    pthread_mutex_lock(&stateMutex);
}

void unlockMutex() {
    pthread_mutex_unlock(&stateMutex);
}

void check_thread_support(int provided) {
    if (DEBUG) printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            if (DEBUG) printf("Brak wsparcia dla wątków, kończę\n");
            fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
            MPI_Finalize();
            exit(-1);
            break;
        case MPI_THREAD_FUNNELED: 
            if (DEBUG) printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	        break;
        case MPI_THREAD_SERIALIZED: 
            if (DEBUG) printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	        break;
        case MPI_THREAD_MULTIPLE:
            if (DEBUG) printf("Pełne wsparcie dla wątków\n");
	        break;
        default:
            if (DEBUG) printf("Nikt nic nie wie\n");
    }
}

void initType() {
    const int nItems = 3;
    int blockLengths[nItems] = { 1, 1, 1};
    MPI_Datatype types[nItems] = { MPI_INT, MPI_INT, MPI_INT };
    
    MPI_Aint offsets[nItems];
    offsets[0] = offsetof(packet_t, lamportTime);
    offsets[1] = offsetof(packet_t, mechanics);
    offsets[2] = offsetof(packet_t, docking);

    MPI_Type_create_struct(nItems, blockLengths, offsets, types, &MPI_PACKET_T);
    MPI_Type_commit(&MPI_PACKET_T);
}

void initApp(int *argc, char ***argv) {
    if (DEBUG) printf("Application initialization...\n");

    int provided;

    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    initType();

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

//    srand(time(NULL) * rank);
    srand(rank);
    mainData.init(rank, size);
    
    sleep(1);
    if (DEBUG) println("Application ready");
}

void finalizeApp() {
    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
}

void incLamportTime(int received) {
    if (received > mainData.lamportTime)
        mainData.lamportTime = received + 1;
    else
        mainData.lamportTime++;
}

int main(int argc, char **argv) {
    initApp(&argc, &argv);
    mainLoop();
    finalizeApp();
    return 0;
}