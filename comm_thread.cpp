#include "comm_thread.h"
#include "main.h"
#include "main_thread.h"
#include "structures.h"

void *comLoop(void *ptr) {
    MPI_Status status;
    packet_t packet;

    while (true) {
        MPI_Recv(&packet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // TODO: sprawdzić wszystkie lamportTime
        switch (status.MPI_TAG) {
            int state;
            bool ack;
            int lamportTime;
            case Message::REQ_D:
                if (DEBUG) println("receive REQ(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);
                
                state = mainData.state;
                ack = false;

                lockMutex();
                    // aktualizacja zegara lamporta
                    incLamportTime(packet.lamportTime);

                    // sprawdzenie czy statek ubiega się o dostęp do doku
                    if (state != State::WAITING_DOCK){
                        ack = true;
                    }
                    else {
                        if(checkPriority(packet.lamportTime)){
                            ack = true;
                        }
                    }
                unlockMutex();

                if (ack){
                    // tworzenie pakietu
                    packet_t response;
                    response.lamportTime = lamportTime;

                    // aktualizacja zegara lamporta przed wysłaniem
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;

                    // wysłanie ACK
                    MPI_Send(&response, 1, MPI_PACKET_T, status.MPI_SOURCE, Message::ACK_D, MPI_COMM_WORLD);
                    if (DEBUG) println("send ACK(time = %d) to rank = %d", response.lamportTime, status.MPI_SOURCE);
                }
                else{
                    // zapisuje żądanie w kolejce
                    lockMutex();
                    mainData.requestQueue.push_back({status.MPI_SOURCE, packet.lamportTime});
                    unlockMutex();
                }

                break;

            case Message::ACK_D:
                if (DEBUG) println("receive ACK_D(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);
        
                lockMutex();
                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);
                // zapisz ACK na liście
                mainData.ackDList[status.MPI_SOURCE] = true;
                unlockMutex();

                break;

            case Message::RELEASE_D:
                if (DEBUG) println("receive RELEASE_D(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);
                
                lockMutex();
                mainData.shipDocks[status.MPI_SOURCE] = false;
                mainData.lookForDock();
                unlockMutex();

                break;

            case Message::REQ_M:
                if (DEBUG) println("receive REQ(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);

                state = mainData.state;
                ack = false;

                lockMutex();
                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);

                // sprawdzenie czy statek ubiega się o przydział mechaników
                if (state != State::WAITING_MECHANIC){
                    ack = true;
                }
                else {
                    if(checkPriority(packet.lamportTime)){
                        ack = true;
                    }
                }
                unlockMutex();

                if (ack){
                    // tworzenie pakietu
                    packet_t response;
                    response.lamportTime = lamportTime;

                    // aktualizacja zegara lamporta przed wysłaniem
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;

                    // wysłanie ACK
                    MPI_Send(&response, 1, MPI_PACKET_T, status.MPI_SOURCE, Message::ACK_M, MPI_COMM_WORLD);
                    if (DEBUG) println("send ACK(time = %d) to rank = %d", response.lamportTime, status.MPI_SOURCE);
                }
                else{
                    // zapisuje żądanie w kolejce
                    lockMutex();
                    mainData.requestQueue.push_back({status.MPI_SOURCE, packet.lamportTime});
                    unlockMutex();
                }

                break;

            case Message::ACK_M:
                if (DEBUG) println("receive ACK_M(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);

                lockMutex();
                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);

                // zapisz ACK na liście
                mainData.ackMList[status.MPI_SOURCE] = true;

                // zapisz liczbę mechaników
                mainData.shipMechanics[status.MPI_SOURCE] = packet.mechanics;
                unlockMutex();

                break;

            default:
                break;
        }

    }
}

bool checkPriority(int time){
    // TODO: sortowanie kolejki
    if(mainData.requestQueue[0][1] > time){
        return true;
    }else return false;
}