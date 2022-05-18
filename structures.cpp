#pragma once
#include "structures.h"
#include "main_thread.h"
#include "main.h"
#include <stdio.h>
#include <math.h>

// Process::Process(int rank, int requestTime) {
//     this->rank = rank;
//     this->requestTime = requestTime;
// }

void Data::init(int rank, int size) {
    this->state = State::FIGHTING;
    this->rank = rank;
    this->size = size;
    this->lamportTime = LAMPORT_DEF;
    this->ackDList.resize(size, 0);
    this->ackMList.resize(size, 0);
    this->shipDocks.resize(size, 0);
    this->shipMechanics.resize(size, 0);
}

bool Data::isAckDFromAll() {
    for (int i = 0; i < this->ackDList.size(); i++) {
        if (!this->ackDList[i])
            return false;
    }
    return true;
}

void Data::lookForDock() {
    lockMutex();
    if (isAckDFromAll()) {
        int lamportTime;
        // inkrementacja zegara lamporta
        incLamportTime(LAMPORT_DEF);
        lamportTime = mainData.lamportTime;

        unlockMutex();

        // wysyłanie ACK_D do oczekujących procesów
        packet_t *packet = (packet_t*)malloc(sizeof(packet_t));
        packet->lamportTime = lamportTime;
        packet->docking = 1;
        packet->mechanics = 0;
        //TODO: wysyłanie ACK_D do wszystkich z kolejki oczekujących

        lockMutex();

        // zmiana stanu statku
        mainData.state = State::WAITING_MECHANIC;

        // zerowanie listy zajętych mechaników
        mainData.shipMechanics.resize(mainData.size, 0);

        // zerowanie listy ACK_M
        mainData.ackMList.resize(mainData.size, false);
        mainData.ackMList[mainData.rank] = true;

        // inkrementacja wartości zegaru lamporta (przed wysyłaniem)
        incLamportTime(LAMPORT_DEF);
        lamportTime = mainData.lamportTime;

        // zapisuje żądanie w kolejce
        mainData.requestQueue.push_back({mainData.rank, lamportTime});
        unlockMutex();

        // wysyła REQ_M do wszystkich (poza samym sobą)
        for (int i = 0; i < mainData.size; i++) {
            if (i != mainData.rank)
                MPI_Send(packet, 1, MPI_PACKET_T, i, Message::REQ_M, MPI_COMM_WORLD);
        }

        checkState();

    } else {
         unlockMutex();
    }
}

int Data::sumMechanics() {
    int sum;
    for (int i = 0; i < this->shipMechanics.size(); i++){
        sum += shipMechanics[i];
    }

    return sum;
}

int Data::mechanicsNeeded() {
    float a = (MECHANICS - 1) / ((float)MAX_DMG - MIN_DMG);
    float b = 1 - a*MIN_DMG;

    return (int) floor((float) this->dmg * a + b);
}

bool Data::checkMechanics(int needed){
    if(needed <= MECHANICS - sumMechanics()){
        // UWAGA! MUTEX WCIĄŻ ZABLOKOWANY!
        return true;
    }else return false;
}

bool Data::isAckMFromAll() {
    for (int i = 0; i < this->ackMList.size(); i++) {
        if (!this->ackMList[i])
            return false;
    }
    return true;
}

void Data::lookForMechanic() {
    lockMutex();
    int mechanics = mechanicsNeeded();
    if (isAckMFromAll()) {
        if(checkMechanics(mechanics)){
            int lamportTime;
            // inkrementacja zegara lamporta
            incLamportTime(LAMPORT_DEF);
            lamportTime = mainData.lamportTime;

            unlockMutex();

            // wysyłanie ACK_M do oczekujących procesów
            packet_t *packet = (packet_t*)malloc(sizeof(packet_t));
            packet->lamportTime = lamportTime;
            packet->docking = 1;
            packet->mechanics = mechanics;
            //TODO: wysyłanie ACK_M do wszystkich z kolejki oczekujących

            lockMutex();

            // zmiana stanu statku
            mainData.state = State::IN_REPAIR;

            checkState();
        }
        unlockMutex();
    } else {
        unlockMutex();
    }
}

