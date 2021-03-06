#include "comm_thread.h"
#include "main.h"
#include "main_thread.h"
#include "structures.h"

void *comLoop(void *ptr) {
    MPI_Status status;
    packet_t packet;

    while (true) {
        // czekanie na otrzymanie wiadomości
        MPI_Recv(&packet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        lockMutex();
            mainData.shipDocks[status.MPI_SOURCE] = packet.docking;
            mainData.shipMechanics[status.MPI_SOURCE] = packet.mechanics;
        unlockMutex();

        switch (status.MPI_TAG) {
            int takenMechanics;
            int isDocking;
            int state;
            bool ack;
            int lamportTime;
            case Message::REQ_D:
                if (DEBUG) println("receive REQ_D(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);
                lockMutex();
                state = mainData.state;
                ack = 0;
                
                    // aktualizacja zegara lamporta
                    incLamportTime(packet.lamportTime);

                    // sprawdzenie czy statek ubiega się o dostęp do doku
                    if (state != State::WAITING_DOCK){
                        ack = 1;
                    }
                    else {
                        if(checkPriority(packet.lamportTime, status.MPI_SOURCE)){
                            ack = 1;
                        }
                    }
                unlockMutex();

                if (ack){
                    lockMutex();
                    // aktualizacja zegara lamporta przed wysłaniem
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;

                    isDocking = mainData.shipDocks[mainData.rank];
                    takenMechanics = mainData.shipMechanics[mainData.rank];

                    unlockMutex();
                    
                    // tworzenie pakietu
                    packet_t response;
                    response.lamportTime = lamportTime;
                    response.docking = isDocking;
                    response.mechanics = takenMechanics;

                    // wysłanie ACK
                    if (DEBUG) println("send ACK_D(time = %d) to rank = %d", response.lamportTime, status.MPI_SOURCE);
                    MPI_Send(&response, 1, MPI_PACKET_T, status.MPI_SOURCE, Message::ACK_D, MPI_COMM_WORLD);
                }
                else{
                    // zapisuje żądanie w kolejce
                    lockMutex();
                    mainData.requestQueue.push_back(make_pair(packet.lamportTime, status.MPI_SOURCE));
                    
                    // if(DEBUG) 
                    // for(std::pair<int, int> i : mainData.requestQueue){
                    //     println("[QUEUE] t = %d, r = %d", i.first, i.second);
                    // }
                    unlockMutex();
                }

                break;

            case Message::ACK_D:
                if (DEBUG) println("receive ACK_D(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);
        
                lockMutex();
                state = mainData.state;
                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);
                // zapisz ACK na liście
                mainData.ackDList[status.MPI_SOURCE] = 1;
                if(mainData.state == State::WAITING_DOCK){
                    unlockMutex();
                    condVarNotify();
                }
                else unlockMutex();

                break;

            case Message::RELEASE_D:
                if (DEBUG) println("receive RELEASE_D(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);
                
                lockMutex();
                for(int i = 0; i < mainData.requestQueue.size();){
                    if (mainData.requestQueue[i].second == status.MPI_SOURCE){
                        mainData.requestQueue.erase(mainData.requestQueue.begin() + i);
                    }
                    else{
                        i++;
                    }
                }
                state = mainData.state;
                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);

                mainData.shipDocks[status.MPI_SOURCE] = 0;
                if(mainData.state == State::WAITING_DOCK){
                    unlockMutex();
                    condVarNotify();
                }
                else unlockMutex();

                break;

            case Message::REQ_M:
                if (DEBUG) 
                println("receive REQ_M(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);
            
                lockMutex();
                state = mainData.state;
                ack = 0;

                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);

                // sprawdzenie czy statek ubiega się o przydział mechaników
                if (state != State::WAITING_MECHANIC){
                    ack = 1;
                }
                else {
                    if(checkPriority(packet.lamportTime, status.MPI_SOURCE)){
                        ack = 1;
                    }
                }
                unlockMutex();

                if (ack){
                    lockMutex();
                    // aktualizacja zegara lamporta przed wysłaniem
                    incLamportTime(LAMPORT_DEF);
                    lamportTime = mainData.lamportTime;
                    isDocking = mainData.shipDocks[mainData.rank];
                    takenMechanics = mainData.shipMechanics[mainData.rank];

                    unlockMutex();
                    
                    // tworzenie pakietu
                    packet_t response;
                    response.lamportTime = lamportTime;
                    response.docking = isDocking;
                    response.mechanics = takenMechanics;

                    // wysłanie ACK
                    if (DEBUG) println("send ACK_M(time = %d) to rank = %d", response.lamportTime, status.MPI_SOURCE);
                    MPI_Send(&response, 1, MPI_PACKET_T, status.MPI_SOURCE, Message::ACK_M, MPI_COMM_WORLD);
                }
                else{
                    // zapisuje żądanie w kolejce
                    lockMutex();
                    // println("[QUEUE] Zapisałem");
                    mainData.requestQueue.push_back(make_pair(packet.lamportTime, status.MPI_SOURCE));
                    unlockMutex();
                }

                break;

            case Message::ACK_M:
                if (DEBUG) 
                println("receive ACK_M(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);

                lockMutex();
                state = mainData.state;
                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);

                // zapisz ACK na liście
                mainData.ackMList[status.MPI_SOURCE] = 1;

                // zapisz liczbę mechaników
                mainData.shipMechanics[status.MPI_SOURCE] = packet.mechanics;

                if(state == State::WAITING_MECHANIC){
                    unlockMutex();
                    condVarNotify();
                }
                else{
                    unlockMutex();
                }

                break;

            case Message::RELEASE_M:
                if (DEBUG) println("receive RELEASE_M(time = %d, mechanics = %d, docking = %d) from rank = %d", packet.lamportTime, packet.mechanics, packet.docking, status.MPI_SOURCE);

                lockMutex();
                state = mainData.state;
                mainData.shipMechanics[status.MPI_SOURCE] = 0;
                // aktualizacja zegara lamporta
                incLamportTime(packet.lamportTime);


                if(state == State::WAITING_MECHANIC){
                    unlockMutex();
                    condVarNotify();
                }
                else{
                    unlockMutex();
                }

                break;

            default:
                break;
        }

    }
}

bool checkPriority(int time, int rank){
    std::sort(mainData.requestQueue.begin(), mainData.requestQueue.end());
    int ourTime = 99999999;
    for(auto i : mainData.requestQueue){
        if (i.second == mainData.rank){
            ourTime = i.first;
        }
    }

    // println("[CHECK PRIORITY] ourTime = %d, time = %d", ourTime, time);
    if(ourTime > time){
        return 1;
    }else if(ourTime == time && mainData.rank > rank){
        return 1;
    }else return 0;
}