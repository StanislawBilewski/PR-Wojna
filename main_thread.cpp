#pragma once
#include "main_thread.h"
#include "main.h"
#include "structures.h"

void mainLoop() {
    if (DEBUG) println("Hello");
    checkState();
}

void checkState(){
    switch (mainData.state) {
        case State::FIGHTING:
            int perc;
            perc = rand() % 100;
            if (perc < HIT_PROB) {
                // statek otrzymuje losowe obrażenia
                int lamportTime;
                int dmg = (rand() % (MAX_DMG - MIN_DMG + 1)) + MIN_DMG;

                lockMutex();
                mainData.dmg = dmg;
                // zmiana stanu statku
                mainData.state = State::WAITING_DOCK;

                // zerowanie listy ACK_D
                mainData.ackDList.resize(mainData.size, false);
                mainData.ackDList[mainData.rank] = true;

                // zerowanie listy zajmowanych doków
                mainData.shipDocks.resize(mainData.size, false);
                unlockMutex();

                // wysyła REQ_D do wszystkich (poza samym sobą)
                packet_t *packet = (packet_t *) malloc(sizeof(packet_t));

                println("[%d] I want to dock", mainData.rank);
                if (DEBUG) println("send REQ_D to ALL");

                // zapisuje żądanie w kolejce
                lockMutex();
                mainData.requestQueue.emplace_back(make_pair(lamportTime,mainData.rank));
                unlockMutex();

                for (int i = 0; i < mainData.size; i++) {
                    lockMutex();
                    if (i != mainData.rank){

                        // inkrementacja wartości zegaru lamporta (przed wysyłaniem)

                        incLamportTime(LAMPORT_DEF);
                        lamportTime = mainData.lamportTime;

                        unlockMutex();

                        packet->lamportTime = lamportTime;
                        packet->mechanics = 0;
                        packet->docking = 0;

                        MPI_Send(packet, 1, MPI_PACKET_T, i, Message::REQ_D, MPI_COMM_WORLD);

                    }
                    else{
                        unlockMutex();
                    }
                }
            }
            sleep(WAITING_TIME);
            checkState();

            break;

        case State::WAITING_DOCK:
            mainData.lookForDock();

            break;

        case State::WAITING_MECHANIC:
            sleep(WAITING_TIME);
            mainData.lookForMechanic();

            break;

        case State::IN_REPAIR:
            // przebywanie w naprawie
            println("[%d] I'm in repair", mainData.rank);
            sleep(1);
            int lamportTime;
            
            lockMutex();

                // zmiana stanu statku
                mainData.state = State::FIGHTING;
            
                // zwolnienie mechaników
                mainData.shipMechanics[mainData.rank] = 0;

            unlockMutex();
            
            // wysyłanie RELEASE_M i RELEASE_D
            packet_t *packet = (packet_t *) malloc(sizeof(packet_t));

            println("[%d] I'm leaving the dock", mainData.rank);
            if (DEBUG) {
                println("send RELEASE_M to ALL");
                println("send RELEASE_D to ALL");
            }

            for (int i = 0; i < mainData.size; i++) {
                lockMutex();
                if (i != mainData.rank) {
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;
                    unlockMutex();

                    packet->lamportTime = lamportTime;
                    packet->docking = 0;
                    packet->mechanics = 0;
                    MPI_Send(packet, 1, MPI_PACKET_T, i, Message::RELEASE_M, MPI_COMM_WORLD);

                    
                    lockMutex();
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;
                    unlockMutex();
                    
                    packet->lamportTime = lamportTime;
                    packet->docking = 0;
                    packet->mechanics = 0;
                    MPI_Send(packet, 1, MPI_PACKET_T, i, Message::RELEASE_D, MPI_COMM_WORLD);
                }else{
                    unlockMutex();
                }
            }
            checkState();
            break;
    }
}