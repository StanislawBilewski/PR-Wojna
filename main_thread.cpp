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

                // inkrementacja wartości zegaru lamporta (przed wysyłaniem)
                incLamportTime(LAMPORT_DEF);
                lamportTime = mainData.lamportTime;

                unlockMutex();

                // wysyła REQ_D do wszystkich (poza samym sobą)
                packet_t *packet = (packet_t *) malloc(sizeof(packet_t));
                packet->lamportTime = lamportTime;
                packet->mechanics = 0;
                packet->docking = 0;

                println("[%d] I want to dock", mainData.rank);
                if (DEBUG) println("send REQ_D(time = %d) to ALL", packet->lamportTime);

                // zapisuje żądanie w kolejce
                lockMutex();
                mainData.requestQueue.emplace_back(make_pair(lamportTime,mainData.rank));
                unlockMutex();

                for (int i = 0; i < mainData.size; i++) {
                    if (i != mainData.rank)
                        MPI_Send(packet, 1, MPI_PACKET_T, i, Message::REQ_D, MPI_COMM_WORLD);
                }
            }
            sleep(WAITING_TIME)
            checkState();

            break;

        case State::WAITING_DOCK:
            mainData.lookForDock();

            break;

        case State::WAITING_MECHANIC:
            mainData.lookForMechanic();

            break;

        case State::IN_REPAIR:
            // przebywanie w naprawie
            sleep(1);
            int lamportTime;
            
            lockMutex();

                // zmiana stanu statku
                mainData.state = State::FIGHTING;

                lamportTime = mainData.lamportTime;

            unlockMutex();
            
            // wysyłanie RELEASE_M i RELEASE_D
            packet_t *packet = (packet_t *) malloc(sizeof(packet_t));
            packet->lamportTime = lamportTime;
            packet->docking = 0;
            packet->mechanics = 0;

            println("[%d] I'm leaving the dock", mainData.rank);
            if (DEBUG) {
                println("send RELEASE_M(time = %d) to ALL", packet->lamportTime);
                println("send RELEASE_D(time = %d) to ALL", packet->lamportTime);
            }

            for (int i = 0; i < mainData.size; i++) {
                if (i != mainData.rank) {
                    MPI_Send(packet, 1, MPI_PACKET_T, i, Message::RELEASE_M, MPI_COMM_WORLD);
                    MPI_Send(packet, 1, MPI_PACKET_T, i, Message::RELEASE_D, MPI_COMM_WORLD);
                }
            }
            checkState();
            break;
    }
}