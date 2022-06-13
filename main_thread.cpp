#pragma once
#include "main_thread.h"
#include "main.h"
#include "structures.h"

void mainLoop() {
    if (DEBUG) println("Hello");
    mainData.shipDocks.resize(mainData.size, 0);
    checkState();
}

void checkState(){
    switch (mainData.state) {
        case State::FIGHTING:
            sleep(WAITING_TIME);
            int perc;
            perc = rand() % 100;
            if (perc < HIT_PROB) {
                int lamportTime;

                // statek otrzymuje losowe obrażenia
                int dmg = (rand() % (MAX_DMG - MIN_DMG + 1)) + MIN_DMG;

                println("I've been hit for %d points of damage!", dmg);

                lockMutex();
                    mainData.dmg = dmg;
                    // zmiana stanu statku
                    mainData.state = State::WAITING_DOCK;

                    // zerowanie listy ACK_D
                    mainData.ackDList.resize(mainData.size, 0);
                    mainData.ackDList[mainData.rank] = 1;

                    // zerowanie listy zajmowanych doków
                    mainData.shipDocks.resize(mainData.size, 0);

                    // zapisuje żądanie w kolejce
                    mainData.requestQueue.emplace_back(make_pair(lamportTime,mainData.rank));
                unlockMutex();

                // wysyła REQ_D do wszystkich (poza samym sobą)
                packet_t *packet = (packet_t *) malloc(sizeof(packet_t));

                println("I want to dock");
                if (DEBUG) println("send REQ_D to ALL");

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

                        if (DEBUG) println("[REQ_D] send REQ_D (time = %d, mechanics = %d, docking = %d) to rank = %d", packet->lamportTime, packet->mechanics, packet->docking, i);
                        MPI_Send(packet, 1, MPI_PACKET_T, i, Message::REQ_D, MPI_COMM_WORLD);

                    }
                    else{
                        unlockMutex();
                    }
                }
                free(packet);
            }
            checkState();

            break;

        case State::WAITING_DOCK:
            sleep(WAITING_TIME);
            mainData.lookForDock();

            break;

        case State::WAITING_MECHANIC:
            sleep(WAITING_TIME);
            mainData.lookForMechanic();

            break;

        case State::IN_REPAIR:
            // przebywanie w naprawie
            println("I'm in repair");
            int lamportTime;
            int docking;
            int mechanics;
            
            lockMutex();
                sleep(REPAIR_TIME/mechanics * WAITING_TIME * (1 + mainData.dmg/25));

                // zmiana stanu statku
                mainData.state = State::FIGHTING;
            
                // zwolnienie mechaników i doku
                mainData.shipDocks[mainData.rank] = 0;
                mainData.shipMechanics[mainData.rank] = 0;

                docking = mainData.shipDocks[mainData.rank];
                mechanics = mainData.shipMechanics[mainData.rank];

            unlockMutex();
            
            // wysyłanie RELEASE_M i RELEASE_D
            packet_t *packet = (packet_t *) malloc(sizeof(packet_t));

            println("I'm leaving the dock");
            if (DEBUG) {
                println("send RELEASE_M to ALL (docking = %d, mechanics = %d)", docking, mechanics);
                println("send RELEASE_D to ALL (docking = %d, mechanics = %d)", docking, mechanics);
            }

            for (int i = 0; i < mainData.size; i++) {
                lockMutex();
                if (i != mainData.rank) {
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;
                    unlockMutex();

                    packet->lamportTime = lamportTime;
                    packet->docking = docking;
                    packet->mechanics = mechanics;
                    MPI_Send(packet, 1, MPI_PACKET_T, i, Message::RELEASE_M, MPI_COMM_WORLD);

                    
                    lockMutex();
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;
                    unlockMutex();
                    
                    packet->lamportTime = lamportTime;
                    packet->docking = docking;
                    packet->mechanics = mechanics;
                    MPI_Send(packet, 1, MPI_PACKET_T, i, Message::RELEASE_D, MPI_COMM_WORLD);
                }else{
                    unlockMutex();
                }
            }

            free(packet);
            checkState();
            break;
    }
}