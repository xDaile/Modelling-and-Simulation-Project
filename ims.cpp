#include "simlib.h"
#include <cstdio>

#define SECOND 1
#define MINUTE (SECOND * 60)
#define HOUR (MINUTE * 60)
#define DAY (HOUR * 24)

#define BATTERY_CAPPACITY 210        // one unit is energy for aprox. 250 metres
#define DISTANCE_WORK 44             //plus shopping etc.
#define DISTANCE_TRIP 80             //80 ORIG
#define EMISIONS_FROM_ELECTRICITY 68 //When batterry is high(battery is also made from coal at SVK so it is about 20g/km for electricity)
#define EL_MAKING 400                //grams of c02 from creating 1kW/h of energy //453 orig
#define ELEKTRO_MOTOR_EFFICIENTY 15  //how many kw/h will car consume per 100km//67.95 72.48

#define EMISIONS_FROM_ENGINE 90 // WHEN batterry is low energy
//#define NIGHT_CHARGE 100
#define DEBUG 0

int el_emisions = (EL_MAKING * ELEKTRO_MOTOR_EFFICIENTY) / 100; // emisions for one km
//int el_emisions = 0;
Facility wDay("Day in work");
Facility rDay("Day without work");
Facility human("driver - human");
Facility ride("people can ride");
Facility startChargingShort("chargin of the car");
Store battery("Battery", BATTERY_CAPPACITY);
Stat Emisions;
Stat savedEmisions;
Stat kilometres;
Stat DebugCharge;
/*
Validovane - kolko sa minie tolko sa nabije - cca 40 km sa nabije pocas noci
            - po prvom dni musi byt čas min 86000
            - process charge OK
            - process humanDay
            - proces road
            - proces week
                - skontrolovane ze realne trva jeden tyzden 60*60*24*7 proti Time
            -- ak by boli emisie z elektriky rovnake ako z auta a bol by to rocny priemer... musi to sediet so statistikami


*/

int emisions = 0;
int savedE = 0;
int carCapacity = 3;
int batteryCap = BATTERY_CAPPACITY;
int kilometresToGo = 0;
int doneKM = 0;

class charge : public Process
{

    void Behavior()
    {

        if (!human.Busy())
        {
            if (DEBUG)
            {
                printf("START CHARGE\n");
            }
            Seize(human);
            int chargeD = 0;
            if (DEBUG)
            {
                printf("charging start time= %d\n", (int)Time);
            }
            while (batteryCap < BATTERY_CAPPACITY && ((!wDay.Busy()) || (!rDay.Busy())))
            {
                batteryCap++;
                chargeD++;
                Leave(battery, 1);
                Wait(MINUTE * 2.6);
            }
            if (DEBUG)
            {
                printf("charging end time= %d, charged = %d\n", (int)Time, chargeD);
            }

            DebugCharge(chargeD);
            Release(human);
        }

        if (DEBUG)
        {
            printf("END CHARGE\n");
        }
    }
};

class road : public Process
{

    void Behavior()
    {
        if (DEBUG)
        {
            //    printf("START ROAD\n");
        }
        Seize(human);
        bool mode = NULL;

        if (!rDay.Busy())
        {
            // Seize(rDay);
            mode = true;
        }
        if (!wDay.Busy())
        {
            // Seize(wDay);
            mode = false;
        }
        if (DEBUG)
        {
            printf("START ROAD km:%d\n", kilometresToGo);
        }
        while (kilometresToGo > 0)
        {
            doneKM++;
            //  printf("in process road in while, battery= %d, kilometresToGO=%d\n",batteryCap,kilometresToGo);
            // printf(" time before one km = %d\n", (int)Time);
            Wait(abs(Normal(55, 30))); //avg 55(person in town, parking etc)
            // printf(" time after one km = %d\n", (int)Time);
            if (carCapacity > 0) //if we have free seat in car
            {
                if (Random() > 0.995) // if there is someone we can pick (hitchhiker, share a ride etc.) - we are saving emisions by that
                {
                    savedEmisions(20 * kilometresToGo);
                    savedE += 20 * kilometresToGo;
                    carCapacity--;
                }
            }
            if (batteryCap > 3) // if our battery have some power for road we use battery
            {
                if (DEBUG)
                {
                    printf("je stava na km c. %d\n", kilometresToGo);
                }
                Enter(battery, 4); //LAST CHECK here it fails probably
                batteryCap -= 4;
                Emisions(el_emisions);
                emisions += el_emisions;
            }
            else //if our batterry is empty
            {
                if (DEBUG)
                {
                    printf("nie je stava na km c. %d\n", kilometresToGo);
                }
                Emisions(EMISIONS_FROM_ENGINE);
                emisions += EMISIONS_FROM_ENGINE;
                if (batteryCap < BATTERY_CAPPACITY) //if we can charge our battery - this if is probably  unnecessary
                {                                   //RECUPERATION
                    Leave(battery, 1);              //we generate the power for 250m from 1km that was made cleary on fuel
                    batteryCap += 1;
                }
            }
            kilometresToGo--;
        }
        /*                 if(DEBUG){
        printf("in process road after while\n");
    }
    */
        carCapacity = 3;
        Release(human);
    }
};
class HumanDay : public Process
{
    void Behavior()
    {

        if (DEBUG)
        {
            printf("START DAY\n");
        }
        if (!wDay.Busy())
        {
            Seize(wDay);
            Wait(10);
            kilometresToGo = abs(Normal(DISTANCE_WORK, 20));
            kilometres(kilometresToGo);
            (new road)->Activate();
            Wait(MINUTE); //for road to seize human, when road will be finished we start working shopping etc.
            Seize(human); //road is finished // we can now add another road

            Wait(((int)Normal(12, 2)) * HOUR); //shopping+working etc
            //we do not have to check if it is still same day, there is no chance that it could have been gone 24 hours(roud would have to last 12 hours)
            Release(human);
            if (Random() < 0.9) //chance that the human have enough power to stand up and plug in hybrid
            {                   //chance to activate charger after trip
                (new charge)->Activate();
            }
            // Activate(Time+)
            if (DEBUG)
            {
                printf("After 8hour\n");
            }

            Release(wDay);
        }
        else if (!rDay.Busy())
        {
            Seize(rDay);
            Wait(10);
            if (Random() > 0.25) //25% chance that human will not use car that day //0.25 orig
            {
                kilometresToGo = abs(Exponential(DISTANCE_TRIP));
                kilometres(kilometresToGo);
                (new road)->Activate();
                Wait(MINUTE);
                Seize(human);                      //after road we pick the human and let him charge his car after some time
                Wait(((int)Normal(13, 3)) * HOUR); //trip itself
                Release(human);

                if (Random() < 0.4)
                { //chance that after trip, or at the hotel etc is charger

                    (new charge)->Activate();
                }
            }
            else
            {
                (new charge)->Activate(); //charging while weekend and car is unused
            }
            Release(rDay);
        }

        if (DEBUG)
        {
            printf("END DAY\n");
        }
        Terminate();
    }
};

class week : public Process
{
    void Behavior()
    {

        if (DEBUG)
        {
            printf("in process week\n");
        }
        Seize(wDay);
        Seize(rDay);

        do
        {
            if (DEBUG)
            {
                printf("NEWW WEEK START\n");
            }
            for (int i = 0; i < 5; i++)
            {
                if (DEBUG)
                {
                    printf("NEW work START\n");
                }
                Release(wDay);              //DAY CAN BE seized in other processes
                (new HumanDay)->Activate(); //new day of the human

                Wait(DAY);

                Seize(wDay);
            }
            for (int i = 0; i < 2; i++)
            {
                if (DEBUG)
                {
                    printf("NEW rest START\n");
                }
                Release(rDay);
                (new HumanDay)->Activate();

                Wait(DAY);

                Seize(rDay);
            }

        } while (1);
    }
};

class Gen : public Event
{
    void Behavior()
    {
        (new week)->Activate();
    }
};

int main()
{

    Init(0, 360 * DAY);
    (new week)->Activate();
    Run();

    printf("Emisie stats\n");
    Emisions.Output();
    printf("\n --Emisions count %d --\n", emisions);

    printf("Saved Emisions\n");
    savedEmisions.Output();
    printf(" mine Saved Emisions= %d\n", savedE);

    // printf("Battery output\n");
    // battery.Output();

    printf("km done= %d\n", doneKM);
    printf("Kilomenetres output\n");
    kilometres.Output();

    printf("\n g/km=%f\n", (((float)emisions) / ((float)doneKM)));
    // printf("Charging output\n");
    // DebugCharge.Output();
}
