/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Nazev souboru: dsimLib.h                                   *
* Autor: Pavel Bednar (xbedna73@stud.fit.vutbr.cz)         *
* Zadani: viz 4. zadani IMS projektu 2020/2021, FIT VUT      *
* Popis: Knihovna pro diskretni simulaci s podporou SHO      *
* Prekladac: g++ 9.3.0                                       *
* Datum vytvoreni: 21.11.2020                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DSIMLIB_H
#define DSIMLIB_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <exception>
#include <cstdio>


/***********************
*  Procesy a udalosti  *
***********************/

/* Potrebne kvuli prekladu. */
class facility;
class store;
class calendarEvent;
class event;
class stats;
bool operator< (calendarEvent a, calendarEvent b);

/* Trida pro jednu instrukci v behavior(). */
class behaviorItem {
    // Nazev metody
    std::string method_name;

    // Mozne parametry metody
    int int_v;
    double double_v;
    std::string string_v;
    facility *facility_v;
    store *store_v;
    stats *stats_v;
public:
    behaviorItem() : facility_v{nullptr}, store_v{nullptr}, stats_v{nullptr} {}

    // Metody pro nastaveni a ziskani metody a jejich parametru
    void setMethodName(std::string name);
    std::string getMethodName();
    void setInt(int x);
    int getInt();
    void setDouble(double x);
    double getDouble();
    void setString(std::string s);
    std::string getString();
    void setFacility(facility *f);
    facility *getFacility();
    void setStore(store *stre);
    store *getStore();
    void setStats(stats *sts);
    stats *getStats();
};

/* Trida pro procesy. */
class process {
    // ID a typ procesu pro identifikaci
    int id;
    static int act_id;
    std::string process_type;

    // Atributy procesu
    double activation_time;
    int priority;
    int operation_priority;
    int required_capacity;

    // Pomocne atributy pro preruseni obsluhy a statistiky
    double in_time;
    double enter_time;
    double wait_time;
    double interupt_time;
    stats *stats_place;
    bool was_in;

    // Urcuje jestli se bude naplnovat behavior_vector nebo exekuovat metoda z nej
    bool first_activation;

    // Urcuje jestli bude proces pasivovan nebo ne
    bool continue_executing_this_process;

    // Vektor do ktereho se ulozi metody v behavior() aby se postupne volaly a metody pro praci s nim
    std::vector<behaviorItem> behavior_vector;
    bool isBVEmpty();
    behaviorItem popFirstBehavior();
    void addBehavior(behaviorItem b);
    void exec(behaviorItem b);

    // Metody implementujici funkce volane v behavior()
    void setPriority_(int p);
    void passivate_();
    void activate_();
    void activateTime_(double d);
    void enter_(facility *f, int operation_p);
    void leave_(facility *f);
    void enterStore_(store *stre, int req_c);
    void leaveStore_(store *stre, int req_c);
    void wait_(double nat);
    void setProcessType_(std::string type);
    void printStats_(facility *f);
    void printStatsStore_(store *stre);
    void printText_(std::string text);
    void in_(stats *sts);
    void out_(stats *sts);

    // Pomocne metody
    double getActivationTime();
    int getPriority();
    int getOperationPriority();
    std::string getProcessType();
    void continue_();
    bool getContinueExecutingThisProcess();
    void setFirstActivation(bool b);
    bool getFirstAcivation();
public:
    process() : id{act_id++}, activation_time{-1}, priority{0}, operation_priority{0}, required_capacity{0}, stats_place{nullptr}, was_in{false}, first_activation{true}, continue_executing_this_process{true} {}
    virtual void behavior();
    void setActivationTime();
    void setActivationTime(double t);

    // Metody volatelne z behavior()
    void setPriority(int p);
    void passivate();
    void activate();
    void activate(double d);
    void enter(facility *f, int operation_p = 0);
    void leave(facility *f);
    void enter(store *stre, int required_capacity);
    void leave(store *stre, int required_capacity);
    void wait(double nat);
    void setProcessType(std::string type);
    void printStats(facility *f);
    void printStats(store *stre);
    void printText(std::string text);
    void in(stats *sts);
    void out(stats *sts);

    friend void startSim(double end_of_simulation);
    friend bool operator< (process a, process b);
    friend bool compareProcessPtr(process *a, process *b);
    friend bool compareProcessPtr2(process *a, process *b);
    friend bool operator< (calendarEvent a, calendarEvent b);
    friend bool operator== (process a, process b);
    friend event;
    friend facility;
    friend store;
};

/* Trida pro udalosti. */
class event {
    // Atributy udalosti
    double activation_time;
public:
    event() : activation_time{-1} {}
    virtual void behavior();
    void setActivationTime();
    void setActivationTime(double t);
    double getActivationTime();
};

/* Spolecna trida pro ulozeni udalosti a procesu do kalendare. */
class calendarEvent {
    process *p;
    event *e;

    // Tento konstruktor vola pouze process
    calendarEvent(process *proc) : p{proc}, e{nullptr} {}

    // Tento konstruktor vola pouze event
    calendarEvent(event *ev) : p{nullptr}, e{ev} {}
public:
    friend process;
    friend event;
    friend void startSim(double end_of_simulation);
    friend bool operator< (calendarEvent a, calendarEvent b);
};


/*******************************
*  Prostredky k behu simulace  *
*******************************/

/* Funkce, ktera vraci aktualni cas simulace. Primarne urceno jako rozhrani
   knihovny pro uzivatelsky pristup k simulacnimu casu. */
double getTime();

/* Funkce, ktera spousti simulaci. Parametr funkce znaci cas konce simulace. */
void startSim(double end_of_simulation);

/* Trida pro kalendar simulace. Uzivatel ma zakazano vytvorit objekt tohoto
   typu. Pokud se o to pokusi bude vyvolana vyjimka. */
class simCalendar {
    static int object_cnt;
    std::vector<calendarEvent> sim_calendar_vector;
public:
    simCalendar();
    bool isEmpty();
    calendarEvent popFirstItem();
    void addEvent(calendarEvent e);
};


/*********************************************
*  Generatory pravdepodobnostnich rozlozeni  *
*********************************************/

/* Funkce, ktera vrati hodnotu rovnomerneho rozlozeni pravdepodobnosti z
   intervalu 0 az x. */
double uniform(double x);

/* Funkce, ktera vrati hodnotu exponencialniho rozlozeni pravdepodobnosti se
   stredem x. */
double exponencial(double x);


/*********************
*  Prostredky k SHO  *
*********************/

/* Trida pro frontu. */
class processQueue {
    // Frontu zde predstavuje vektor z duvodu lepsiho zpusobu razeni jeho prvku
    std::vector<process*> q;
public:
    processQueue() {}
    void push(process *p);
    process *pop();
    bool isEmpty();
    int len();
    bool find(process *p);

    friend process;
    friend facility;
    friend store;
};

/* Trida pro zarizeni. */
class facility {
    // Jmeno zarizeni pro identifikaci
    std::string name;

    // Indikator jestli ma zarizeni vypisovat statistiky
    bool print_stats;

    // Aktualne obsluhovany proces
    process *actual_process;

    // Fronta k zarizeni
    processQueue *waiting_processes;

    // Fronta prerusenych procesu
    processQueue interupted_processes;

    // Pomocne promenne pro statistiku vyuziti linky
    double last_process_in_time;
    double process_on_facility_time;

    // Pomocne promenne a metody
    bool need_delete;
    void printQueue();
public:
    facility(std::string n) : name{n}, print_stats{false}, actual_process{nullptr}, waiting_processes{new processQueue}, last_process_in_time{0}, process_on_facility_time{0}, need_delete{true} {}
    facility(std::string n, processQueue *q) : name{n}, print_stats{false}, actual_process{nullptr}, waiting_processes{q}, last_process_in_time{0}, process_on_facility_time{0}, need_delete{false} {}
    ~facility();

    // Metoda pro vytisknuti statistik o vyuziti linky
    void printUse();

    friend process;
};

/* Trida pro sklad. */
class store {
    // Jmeno skladu pro identifikaci
    std::string name;

    // Indikator jestli ma zarizeni vypisovat statistiky
    bool print_stats;

    // Fronta ke skladu
    processQueue *waiting_processes;

    // Kapacita skladu
    int capacity;
    int actual_capacity;

    // Pomocne metody
    void printQueue();
public:
    store(std::string n, int c) : name{n}, print_stats{false}, waiting_processes{new processQueue}, capacity{c}, actual_capacity{c} {}
    ~store();

    friend process;
};


/******************************************************
*  Statistiky vyuziti linek a casu procesu v systemu  *
******************************************************/

/* Trida pro statistiky. */
class stats {
    std::vector<double> time_in_facility_store_stats;
    std::vector<double> time_in_system_stats;
    void printStars(double cnt);
public:
    stats() {}
    void printStats();

    friend process;
};


/****************************
*  Vyjimky a chybove stavy  *
****************************/

/* Trida pro vyjimky. */
class simExp : std::exception {
    std::string message;
    int return_code;
public:
    simExp(std::string m, int rc) : message{m}, return_code{rc} {}
    std::string getMessage();
    int getRC();
};

/* Seznam chybovych stavu. */
enum return_codes : int {
    ok,                                  // 0
    cant_instanciating_sim_calendar,     // 1
    behavior_missing,                    // 2
    behavior_after_wait_missing,         // 3
    leaving_empty_facility,              // 4
    bad_variable_type,                   // 5
    too_much_capacity_in_store           // 6
};


#endif // DSIMLIB_H
