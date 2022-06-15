#ifndef CONSTANTSH
#define CONSTANTSH

#define DOCKS 1               	// liczba doków
#define MECHANICS 5           	// liczba mechaników

#define HIT_PROB 15    			// prawdopodobieństwo bycia trafionym
#define WAITING_TIME 1          // liczba sekund spędzonych w jednym stanie
#define REPAIR_TIME 2          // repair time multiplier

                                // ostateczny wzór na czas naprawy to:
                                // REPAIR_TIME * WAITING_TIME * (1 + $dmg/25)
                                // gdzie:
                                // $dmg to liczba otrzymanych punktów obrażeń

// #define MAX_HP 100     			// maksymalna liczba punktów życia statku (nieużywane)

#define MIN_DMG 10				// minimalna liczba otrzymanych obrażeń
#define MAX_DMG 90				// maksymalna liczba otrzymanych obrażeń

#define LAMPORT_DEF 0       	// początkowa wartość zegara Lamporta

#endif

#ifndef DEBUG
#define DEBUG false

#endif
