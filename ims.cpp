#include "simlib.h"
#include "math.h"
#include "stdio.h"

#define CAPACITY 800 //800 x 0.5km, one unit is 500 metres

Facility vehicle ("Autonomny kamion je v prevadzke");
Facility roadThere("Cesta s plnym kamionom tam");
Facility roadBackE("Cesta s praznym kamionom spat");
Facility roadBackF("Cesta s plnym kamionom spat");

Facility BatteryReadyForEmptyKM("Bateria je pripravena na kilometer s praznym kamionom");
Facility BatteryReadyForFullKM("Bateria je pripravena na kilometer s plnym kamionom");
Facility ChargedAfterRoad("Battery is fully charged");
Facility ChargeAfterRoad("Battery needs to be charged");

