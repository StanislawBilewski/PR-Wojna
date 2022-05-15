#include "structures.h"
#include "main.h"
#include <stdio.h>
#include <math.h>

Process::Process(int rank, int requestTime) {
    this->rank = rank;
    this->requestTime = requestTime;
}

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

bool Data::isAckMFromAll() {
    for (int i = 0; i < this->ackMList.size(); i++) {
        if (!this->ackMList[i])
            return false;
    }
    return true;
}

int Data::sumMechanics() {
    int sum;
    for (int i = 0; i < this->shipMechanics.size(); i++){
        sum += shipMechanics[i];
    }

    return sum;
}

int Data::mechanicsNeeded() {
    float a = (MECHANICS - 1) / ((float)MAX_DMG - MIN_DMG)
    float b = 1 - a*MIN_DMG 

    return (int) floor((float) this->dmg * a + b);
}

void Data::checkMechanics(){
    if(mechanicsNeeded() <= MECHANICS - sumMechanics()){
        int lamportTime;
        // UWAGA! MUTEX WCIĄŻ ZABLOKOWANY!
        data.state = State::IN_REPAIR;
    }
    else{
        MPI_Status status;
        packet_t packet;
        MPI_Recv(&packet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, Message::RELEASE_M, MPI_COMM_WORLD, &status);

        checkMechanics();
    }
}