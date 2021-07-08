/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Nazev souboru: dsim.cpp                                    *
* Autor: Pavel Bednar (xbedna73@stud.fit.vutbr.cz)           *
* Zadani: viz 4. zadani IMS projektu 2020/2021, FIT VUT      *
* Popis: Demo ukazka knihovny dsimLib                        *
* Prekladac: g++ 9.3.0                                       *
* Datum vytvoreni: 21.11.2020                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "dsimLib.h"

facility linka("linka");
stats myStats;

class paket : public process {
    void behavior() {
        in(&myStats);
        setProcessType("paket");
        printStats(&linka);
        enter(&linka);
        opak:
        wait(uniform(100) + 150);
        wait(5);
        if (uniform(1) <= 0.01) {
            goto opak;
        }
        leave(&linka);
        out(&myStats);
    }
};

class generator : public event {
    void behavior() {
        double r = uniform(1);
        paket *p = new paket;
        if (r <= 0.2) {
            p->setPriority(3);
        } else if (r > 0.2 && r <= 0.6) {
            p->setPriority(2);
        } else {
            p->setPriority(1);
        }
        p->setActivationTime();
        setActivationTime(getTime() + exponencial(250));
    }
};

int main() {

    /* Aktivace generatoru. */
    (new generator)->setActivationTime();

    /* Blok try - catch pro pripad, ze by v tomto souboru byla chyba. */
    try {

        /* Spusteni simulace, nastaveni casu konce simulace a vypsani statistik. */
        startSim(11500);
        myStats.printStats();
        linka.printUse();

    /* Osetreni pripadnych vyjimek zpusobenych chybnym pouzitim knihovny dsimLib. */
    } catch (simExp & err) {
        std::cerr << err.getMessage() << std::endl;
        return err.getRC();
    }

    return ok;
}
