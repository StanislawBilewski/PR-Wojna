#include "main_thread.h"
#include "main.h"
#include "structures.h"

void mainLoop() {
    if (DEBUG) println("Hello");

    while(true) {
        switch (data.state) {
            case State::FIGHTING:
                int perc = random() % 100;
                if (perc < HIT_PROB) {
                    // statek otrzymuje losowe obrażenia
                    int lamportTime;
                    int dmg = (rand() % (MAX_DMG - MIN_DMG + 1)) + MIN_DMG;

                    lockMutex();
                        data->dmg = dmg;
                        // zmiana stanu statku
                        data.state = State::WAITING_DOCK;

                        // zerowanie listy ACK_D
                        data.ackDList.resize(data.size, false);
                        data.ackDList[data.rank] = true;
                        
                        // zerowanie listy zajmowanych doków
                        data.shipDocks.resize(data.size, false);

                        // inkrementacja wartości zegaru lamporta (przed wysyłaniem)
                        incLamportTime(LAMPORT_DEF);
                        lamportTime = data.lamportTime;

                    unlockMutex();

                    packet_t *packet = (packet_t*)malloc(sizeof(packet_t));
                    packet->lamportTime = lamportTime;
                    packet->mechanics = 0;
                    packet->docking = 0;

                    println("[%d] I want to dock", rank);
                    if (DEBUG) println("send REQ_D(time = %d) to ALL", packet->lamportTime);

                    // wysyła REQ_D do wszystkich (poza samym sobą)
                    for (int i = 0; i < data.size; i++) {
                        if (i != data.rank)
                            MPI_Send(packet, 1, MPI_PACKET_T, i, Message::REQ_D, MPI_COMM_WORLD);
                    }

                }

                break;

            case State::WAITING_DOCK:
                lockMutex();
                if (data.isAckDFromAll()) {
                    int lamportTime;
                    // UWAGA! MUTEX WCIĄŻ ZABLOKOWANY!
                    data.state = State::WAITING_MECHANIC;

                    // // inkrementacja zegara lamporta
                    // incLamportTime(LAMPORT_DEF);
                    // lamportTime = data.lamportTime;

                    unlockMutex();

                    // packet_t *packet = (packet_t*)malloc(sizeof(packet_t));
                    // packet->lamportTime = lamportTime;
                    // packet->docking = 1;
                    // // packet->mechanics = 0;

                } else {
                    unlockMutex();
                }

                break;

            case State::WAITING_MECHANIC:
                lockMutex();
                if (data.isAckMFromAll()){
                    data.checkMechanics();

                    unlockMutex();

                } else {
                    unlockMutex();
                }

                break;

            case State::IN_REPAIR:


        sleep(WAITING_TIME);
    }
}