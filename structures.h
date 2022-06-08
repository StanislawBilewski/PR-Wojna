#ifndef STRUCTURESH
#define STRUCTURESH
#pragma once
#include <vector>

#include "constants.h"

using namespace std;

typedef struct {
    int lamportTime;    // lokalny czas lamporta
    int mechanics;      // liczba przydzielonych mechaników
    int docking;        // czy statek dokuje
} packet_t;

enum State {
    FIGHTING,           // walczy
    WAITING_DOCK,       // oczekuje na dok
    WAITING_MECHANIC,   // oczekuje na przydział mechaników
    IN_REPAIR           // jest w naprawie
};

enum Message {
    REQ_D,      // żądanie doku
    ACK_D,      // akceptacja żądania o dok
    RELEASE_D,  // zwolnienie doku
    REQ_M,      // żądanie przydziału mechaników
    ACK_M,      // akceptacja żądania o przydział mechaników
    RELEASE_M   // zwolnienie mechaników
};

// class Process {
// public:
//     int rank;
//     int requestTime;

//     Process(int rank, int requestTime);
// };

class Data {
public:
    State state;                                // obecny stan statku
    int rank, size;
    int lamportTime;                            // lokalny czas lamporta
    int dmg;                                    // liczba otrzymanych punktów obrażeń
    vector<bool> ackDList;                      // vector otrzymanych wiadomości ACK_D
    vector<bool> ackMList;                      // vector otrzymanych wiadomości ACK_M
    vector<bool> shipDocks;                     // vector zajętych doków
    vector<int> shipMechanics;                  // vector zajętych mechaników
    vector<pair<int,int>> requestQueue;         // kolejka oczekujących żądań (priorytet, ranga)

    bool checkDocks();
    void init(int rank, int size);
    bool isAckDFromAll();
    bool isAckMFromAll();
    int sumMechanics();
    bool checkMechanics(int);
    int mechanicsNeeded();
    void lookForDock();
    void lookForMechanic();
};

#endif
