/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Nazev souboru: dsimLib.cpp                                 *
* Autor: Pavel Bednar (xbedna73@stud.fit.vutbr.cz)           *
* Zadani: viz 4. zadani IMS projektu 2020/2021, FIT VUT      *
* Popis: Knihovna pro diskretni simulaci s podporou SHO      *
* Prekladac: g++ 9.3.0                                       *
* Datum vytvoreni: 21.11.2020                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "dsimLib.h"

simCalendar sim_calendar;

void behaviorItem::setMethodName(std::string name) {
    method_name = name;
}

std::string behaviorItem::getMethodName() {
    return method_name;
}

void behaviorItem::setInt(int x) {
    int_v = x;
}

int behaviorItem::getInt() {
    return int_v;
}

void behaviorItem::setDouble(double x) {
    double_v = x;
}

double behaviorItem::getDouble() {
    return double_v;
}

void behaviorItem::setString(std::string s) {
    string_v = s;
}

std::string behaviorItem::getString() {
    return string_v;
}

void behaviorItem::setFacility(facility *f) {
    facility_v = f;
}

facility *behaviorItem::getFacility() {
    return facility_v;
}

void behaviorItem::setStore(store *stre) {
    store_v = stre;
}

store *behaviorItem::getStore() {
    return store_v;
}

void behaviorItem::setStats(stats *sts) {
    stats_v = sts;
}

stats *behaviorItem::getStats() {
    return stats_v;
}

int process::act_id = 0;

bool process::isBVEmpty() {
    return behavior_vector.empty();
}

behaviorItem process::popFirstBehavior() {
    // vrati prvni prvek fronty a z fronty ho smaze
    behaviorItem a = behavior_vector[0];
    std::reverse(behavior_vector.begin(), behavior_vector.end());
    behavior_vector.pop_back();
    std::reverse(behavior_vector.begin(), behavior_vector.end());
    return a;
}

void process::addBehavior(behaviorItem b) {
    behavior_vector.push_back(b);
}

void process::exec(behaviorItem b) {
    // podle obsahu behaviorItemu se zavola spravna metoda
    if (b.getMethodName() == "setPriority") {
        setPriority_(b.getInt());
    } else if (b.getMethodName() == "passivate") {
        passivate_();
    } else if (b.getMethodName() == "activate") {
        activate_();
    } else if (b.getMethodName() == "activateTime") {
        activateTime_(b.getDouble());
    } else if (b.getMethodName() == "enter") {
        enter_(b.getFacility(), b.getInt());
    } else if (b.getMethodName() == "leave") {
        leave_(b.getFacility());
    } else if (b.getMethodName() == "enterStore") {
        enterStore_(b.getStore(), b.getInt());
    } else if (b.getMethodName() == "leaveStore") {
        leaveStore_(b.getStore(), b.getInt());
    } else if (b.getMethodName() == "wait") {
        wait_(b.getDouble());
    } else if (b.getMethodName() == "setProcessType") {
        setProcessType_(b.getString());
    } else if (b.getMethodName() == "printStats") {
        printStats_(b.getFacility());
    } else if (b.getMethodName() == "printStatsStore") {
        printStatsStore_(b.getStore());
    } else if (b.getMethodName() == "printText") {
        printText_(b.getString());
    } else if (b.getMethodName() == "in") {
        in_(b.getStats());
    } else if (b.getMethodName() == "out") {
        out_(b.getStats());
    }
}

void process::setPriority_(int p) {
    priority = p;
    continue_();
}

void process::passivate_() {
    // behaviorItemy procesu se prestanou vykonavat a pokracuje next event algoritmus
    this->continue_executing_this_process = false;
}

void process::activate_() {
    this->setActivationTime();
    continue_();
}

void process::activateTime_(double d) {
    this->setActivationTime(getTime() + d);
    continue_();
}

void process::enter_(facility *f, int operation_p) {
    this->operation_priority = operation_p;
    if (f->actual_process == nullptr) {
        // pokud nikdo neni v obsluze, jde do obsluhy tento proces
        enter_time = getTime();
        f->actual_process = this;

        // statsitiky o vyuziti linky
        f->process_on_facility_time += (getTime() - f->last_process_in_time);
        continue_();
    } else {
        if (this->getOperationPriority() > f->actual_process->getOperationPriority()) {
            // pokud ma tento proces vyssi prioritu obsluhy nez proces v obsluze, proces v obsluze
            // je vlozen do fronty prerusenych procesu a tento proces jde do obsluhy
            enter_time = getTime();
            f->actual_process->interupt_time = getTime();
            f->interupted_processes.push(f->actual_process);
            f->actual_process = this;
            continue_();
        } else {
            // pokud ma tento proces nizsi nebo stejnou prioritu obsluhy nez proces v obsluze, je tento
            // proces vlozen do fronty cekajicich procesu
            f->waiting_processes->push(this);
            passivate_();
        }
    }

    if (f->print_stats) {
        // tisk statistik o stavu linky
        printf("TIME: %08.2f (", getTime());
        std::cout << "+" << ") " << "ID: ";
        printf("%03i ", this->id);
        std::cout << "TYPE: " << this->getProcessType() << " " << f->name << ": " << "[";
        printf("%03i]|", f->actual_process->id);
        f->printQueue();
        std::cout << std::endl;
    }
}

void process::leave_(facility *f) {
    if (f->actual_process == nullptr) {
        throw simExp("Proces pozaduje byt vytahnut z facility ve ktere neni!\n", leaving_empty_facility);
    } else {
        if (f->interupted_processes.isEmpty()) {
            // pokud nejsou zadne prerusene procesy
            if (!f->waiting_processes->isEmpty()) {
                // pokud nekdo ceka ve fronte na tuto linku, vytahnu prvniho z ni, vlozim do obsluhy linky
                // a naplanuji mu pokracovani
                f->actual_process = f->waiting_processes->pop();
                f->actual_process->enter_time = getTime();
                f->actual_process->setActivationTime();
            } else {
                // pokud neni zadny process ve fronte linka zustane neobsazena
                f->actual_process = nullptr;
                f->last_process_in_time = getTime();
            }
            // tento proces bude pokracovat dalsimi jeho instrukcemi v behaviorItem
            continue_();

            if (was_in) {
                // pokud jsou zaple statistiky o procesu
                stats_place->time_in_facility_store_stats.push_back(getTime() - enter_time);
            }
        } else {
            // pokud jsou nejake prerusene procesy
            if (f->interupted_processes.find(this)) {
                // pokud chci odejit, ale jsem ve fronte prerusenych procesu, pasivuji se
                passivate_();
            } else {
                // pokud chci odejit a jsem proces v obsluze, vytahnu prvniho z fronty prerusenych procesu, nastavim
                // mu jeho pokracovani a naplanuji ho do kalendare
                f->actual_process = f->interupted_processes.pop();

                std::reverse(f->actual_process->behavior_vector.begin(), f->actual_process->behavior_vector.end());

                behaviorItem b1;
                b1.setMethodName("leave");
                b1.setFacility(f);
                f->actual_process->behavior_vector.push_back(b1);

                behaviorItem b2;
                b2.setMethodName("wait");
                b2.setDouble(f->actual_process->wait_time - (f->actual_process->interupt_time - f->actual_process->enter_time));
                f->actual_process->behavior_vector.push_back(b2);

                std::reverse(f->actual_process->behavior_vector.begin(), f->actual_process->behavior_vector.end());

                f->actual_process->setActivationTime();
                // tento proces bude pokracovat dalsimi jeho instrukcemi v behaviorItem
                continue_();

                if (was_in) {
                    // pokud jsou zaple statistiky o procesu
                    stats_place->time_in_facility_store_stats.push_back(getTime() - enter_time);
                }
            }
        }
    }

    if (f->print_stats) {
        // tisk statistik o stavu linky
        printf("TIME: %08.2f (", getTime());
        std::cout << "-" << ") " << "ID: ";
        printf("%03i ", this->id);
        std::cout << "TYPE: " << this->getProcessType() << " " << f->name << ": " << "[";
        if (f->actual_process == nullptr) {
            std::cout << "   ]|";
        } else {
            printf("%03i]|", f->actual_process->id);
        }
        f->printQueue();
        std::cout << std::endl;
    }
}

void process::enterStore_(store *stre, int req_c) {
    required_capacity = req_c;
    if (stre->actual_capacity >= required_capacity) {
        // pokud je kapacita prirad ji procesu, ktery ji pozaduje
        enter_time = getTime();
        stre->actual_capacity -= required_capacity;
        continue_();
    } else {
        // pokud neni kapacita bez do fronty
        stre->waiting_processes->push(this);
        passivate_();
    }

    if (stre->print_stats) {
        // tisk statistik o stavu linky
        printf("TIME: %08.2f (", getTime());
        std::cout << "+" << ") " << "ID: ";
        printf("%03i ", this->id);
        std::cout << "TYPE: " << this->getProcessType() << " " << stre->name << ": ";
        printf("%03i|", stre->actual_capacity);
        stre->printQueue();
        std::cout << std::endl;
    }
}

void process::leaveStore_(store *stre, int req_c) {
    // vracim kapacitu
    stre->actual_capacity += req_c;
    if (stre->actual_capacity > stre->capacity) {
        throw simExp("Proces vraci do store vice kapacity nez si vzal!\n", too_much_capacity_in_store);
    }

    if (!stre->waiting_processes->isEmpty()) {
        // pokud je fronta ke skladu neprazdna, najdu v ni prvni uspokojitelny pozadavek, pridelim
        // mu kapacitu a naplanuji ho do kalendare
        int i = 0;
        while(stre->waiting_processes->q[i]->required_capacity > stre->actual_capacity) {
            i++;
        }
        stre->waiting_processes->q[i]->enter_time = getTime();
        stre->waiting_processes->q[i]->setActivationTime();
        stre->actual_capacity -= stre->waiting_processes->q[i]->required_capacity;
        stre->waiting_processes->q.erase(stre->waiting_processes->q.begin() + i);
    }
    continue_();

    if (was_in) {
        // pokud jsou zaple statistiky o procesu
        stats_place->time_in_facility_store_stats.push_back(getTime() - enter_time);
    }

    if (stre->print_stats) {
        // tisk statistik o stavu linky
        printf("TIME: %08.2f (", getTime());
        std::cout << "-" << ") " << "ID: ";
        printf("%03i ", this->id);
        std::cout << "TYPE: " << this->getProcessType() << " " << stre->name << ": ";
        printf("%03i|", stre->actual_capacity);
        stre->printQueue();
        std::cout << std::endl;
    }
}

void process::wait_(double nat) {
    wait_time = nat;
    setActivationTime(getTime() + nat);
    passivate_();
}

void process::setProcessType_(std::string type) {
    process_type = type;
    continue_();
}

void process::printStats_(facility *f) {
    f->print_stats = true;
    continue_();
}

void process::printStatsStore_(store *stre) {
    stre->print_stats = true;
    continue_();
}

void process::printText_(std::string text) {
    std::cout << text;
    continue_();
}

void process::in_(stats *sts) {
    was_in = true;
    stats_place = sts;
    in_time = getTime();
    continue_();
}

void process::out_(stats *sts) {
    sts->time_in_system_stats.push_back(getTime() - in_time);
    continue_();
}

double process::getActivationTime() {
    return activation_time;
}

int process::getPriority() {
    return priority;
}

int process::getOperationPriority() {
    return operation_priority;
}

std::string process::getProcessType() {
    return process_type;
}

void process::continue_() {
    this->continue_executing_this_process = true;
}

bool process::getContinueExecutingThisProcess() {
    return continue_executing_this_process;
}

void process::setFirstActivation(bool b) {
    first_activation = b;
}

bool process::getFirstAcivation() {
    return first_activation;
}

void process::behavior() {
    // zarizuje povinnou reimplementaci metody behavior
    throw simExp("Musi byt implementovana metoda void behavior() !\n", behavior_missing);
}

void process::setActivationTime() {
    activation_time = getTime();
    calendarEvent c{this};
    sim_calendar.addEvent(c);
}

void process::setActivationTime(double t) {
    activation_time = t;
    calendarEvent c{this};
    sim_calendar.addEvent(c);
}

void process::setPriority(int p) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("setPriority");
    b.setInt(p);
    addBehavior(b);
}

void process::passivate() {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("passivate");
    addBehavior(b);
}

void process::activate() {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("activate");
    addBehavior(b);
}

void process::activate(double d) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("activateTime");
    b.setDouble(d);
    addBehavior(b);
}

void process::enter(facility *f, int operation_p) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("enter");
    b.setFacility(f);
    b.setInt(operation_p);
    addBehavior(b);
}

void process::leave(facility *f) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("leave");
    b.setFacility(f);
    addBehavior(b);
}

void process::enter(store *stre, int required_capacity) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("enterStore");
    b.setStore(stre);
    b.setInt(required_capacity);
    addBehavior(b);
}

void process::leave(store *stre, int required_capacity) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("leaveStore");
    b.setStore(stre);
    b.setInt(required_capacity);
    addBehavior(b);
}

void process::wait(double nat) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("wait");
    b.setDouble(nat);
    addBehavior(b);
}

void process::setProcessType(std::string type) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("setProcessType");
    b.setString(type);
    addBehavior(b);
}

void process::printStats(facility *f) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("printStats");
    b.setFacility(f);
    addBehavior(b);
}

void process::printStats(store *stre) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("printStatsStore");
    b.setStore(stre);
    addBehavior(b);
}

void process::printText(std::string text) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("printText");
    b.setString(text);
    addBehavior(b);
}

void process::in(stats *sts) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("in");
    b.setStats(sts);
    addBehavior(b);
}

void process::out(stats *sts) {
    // volanim metody z behavior se vyvori prislusny behaviorItem
    behaviorItem b;
    b.setMethodName("out");
    b.setStats(sts);
    addBehavior(b);
}

bool operator< (process a, process b) {
    return a.priority < b.priority;
}

bool compareProcessPtr(process *a, process *b) {
    // definice operatoru pro porovnavani procesu a jejich nasledne razeni ve frontach
    return a->priority > b->priority;
}

bool compareProcessPtr2(process *a, process *b) {
    // definice operatoru pro porovnavani procesu a jejich nasledne razeni ve frontach
    return a->operation_priority > b->operation_priority;
}

bool operator== (process a, process b) {
    return a.id == b.id;
}

void event::behavior() {
    // zarizuje povinnou reimplementaci metody behavior
    throw simExp("Musi byt implementovana metoda void behavior() !\n", behavior_missing);
}

void event::setActivationTime() {
    activation_time = getTime();
    calendarEvent c{this};
    sim_calendar.addEvent(c);
}

void event::setActivationTime(double t) {
    activation_time = t;
    calendarEvent c{this};
    sim_calendar.addEvent(c);
}

double event::getActivationTime() {
    return activation_time;
}

bool operator< (calendarEvent a, calendarEvent b) {
    // operator podle ktereho se sezaruji prvky v kalendari
    if (a.p == nullptr) {
        if (b.p == nullptr) {
            return a.e->getActivationTime() < b.e->getActivationTime();
        } else {
            return a.e->getActivationTime() < b.p->getActivationTime();
        }
    } else {
        if (b.p == nullptr) {
            return a.p->getActivationTime() < b.e->getActivationTime();
        } else {
            return a.p->getActivationTime() < b.p->getActivationTime();
        }
    }
}

// aktualni cas simulace
double sim_time = 0;

double getTime() {
    return sim_time;
}

// cas konce simulace
double eos = 0;

// realizace next event algoritmu
void startSim(double end_of_simulation) {
    eos = end_of_simulation;
    while (!sim_calendar.isEmpty()) {
        // vytahnu prvni prvek z kalendare
        calendarEvent a = sim_calendar.popFirstItem();
        if (a.p == nullptr) {
            // pokud je to event
            if (a.e->getActivationTime() > end_of_simulation) {
                // konec simulace
                break;
            } else {
                // nastaveni casu simulace a provedeni popisu chovani
                sim_time = a.e->getActivationTime();
                a.e->behavior();
            }
        } else {
            // pokud je to process
            if (a.p->getActivationTime() > end_of_simulation) {
                // konec simulace
                break;
            } else {
                // nastaveni casu simulace a provedeni popisu chovani
                sim_time = a.p->getActivationTime();

                // pokud prvni aktivace pak
                if (a.p->getFirstAcivation()) {
                    // provedeni popisu chovani ktera naplni behavor vector
                    a.p->behavior();
                    a.p->setFirstActivation(false);
                }
                do {
                    // vychozi stav je, ze se provede jeden prikaz z behavior vectoru
                    a.p->passivate_();

                    if (!a.p->isBVEmpty()) {
                        // exekuce konkretniho prikazu z bahavior vectoru
                        a.p->exec(a.p->popFirstBehavior());
                    }
                } while (a.p->getContinueExecutingThisProcess());
            }
        }
    }
}

int simCalendar::object_cnt = 0;

simCalendar::simCalendar() {
    // zakazani instanciace teto tridy
    object_cnt++;
    if (object_cnt > 1) {
        throw simExp("Nelze vytvaret instance tridy simCalendar!\n", cant_instanciating_sim_calendar);
    }
}

bool simCalendar::isEmpty() {
    return sim_calendar_vector.empty();
}

calendarEvent simCalendar::popFirstItem() {
    // vrati a odstrani prvni prvek z kalendare
    calendarEvent a = sim_calendar_vector[0];
    std::reverse(sim_calendar_vector.begin(), sim_calendar_vector.end());
    sim_calendar_vector.pop_back();
    std::reverse(sim_calendar_vector.begin(), sim_calendar_vector.end());
    return a;
}

void simCalendar::addEvent(calendarEvent e) {
    // prida prvek do kalendare
    sim_calendar_vector.push_back(e);
    std::sort(sim_calendar_vector.begin(), sim_calendar_vector.end());
}

static uint32_t val = 1000;

double uniform(double x) {
    // funkce na ziskani hodnoty rovnomerneho rozlozeni pomoci kongruentniho generatoru
    val = val * 69069u + 1u;
    return (val / ((double)UINT32_MAX + 1.0)) * x;
}

double exponencial(double x) {
    // funkce na ziskani hodnoty exponencialniho rozlozeni pomoci metody inverzni transformace
    return (-x) * log(1 - uniform(1));
}

void processQueue::push(process *p) {
    // prida prvek do fronty a frontu preusporada
    q.push_back(p);
    std::stable_sort(q.begin(), q.end(), compareProcessPtr);
    std::stable_sort(q.begin(), q.end(), compareProcessPtr2);
}

process *processQueue::pop() {
    // vrati a vytahne prvek z fronty
    process *np = q[0];
    std::reverse(q.begin(), q.end());
    q.pop_back();
    std::reverse(q.begin(), q.end());
    return np;
}

bool processQueue::isEmpty() {
    return q.empty();
}

int processQueue::len() {
    return q.size();
}

bool processQueue::find(process *p) {
    // vrati jestli proces s danym ID se nechazi ve fronte
    std::vector<process*>::iterator it;
    it = std::find(q.begin(), q.end(), p);
    if (it != q.end()) {
        return true;
    } else {
        return false;
    }
}

void facility::printQueue() {
    for (int i = 0; i < waiting_processes->len(); i++) {
        std::cout << "[";
        printf("%03i", waiting_processes->q[i]->id);
        std::cout << "]";
    }
}

facility::~facility() {
    if (need_delete) {
        delete waiting_processes;
    }
}

void facility::printUse() {
    std::cout << "-------- USE OF FACILITY: " << name << " --------\n";
    std::cout << "EMPTY: " << process_on_facility_time << std::endl;
    std::cout << "FULL: " << eos - process_on_facility_time << std::endl;
    printf("USE: %2.0f", ((eos - process_on_facility_time) / eos) * 100);
    std::cout << " %\n----------------------------------------\n\n";
}

void store::printQueue() {
    for (int i = 0; i < waiting_processes->len(); i++) {
        std::cout << "[";
        printf("%03i", waiting_processes->q[i]->id);
        std::cout << "]";
    }
}

store::~store() {
    delete waiting_processes;
}

void stats::printStars(double cnt) {
    std::string ret_str;
    for (int i = 0; i < cnt; i++) {
        ret_str += "*";
    }
    std::cout << ret_str << std::endl;
}

void stats::printStats() {
    double sum = 0;
    std::cout << "-------- PROCESS TIME ON FACILITY OR STORE --------\n";
    for (auto i : time_in_facility_store_stats) {
        sum += i;
        printf("%08.2f ", i);
        printStars(static_cast<int>(i / 10));
    }
    std::cout << "---------------------------------------------------\n";
    std::cout << "MIN: " << *std::min_element(time_in_facility_store_stats.begin(), time_in_facility_store_stats.end()) << std::endl;
    std::cout << "MAX: " << *std::max_element(time_in_facility_store_stats.begin(), time_in_facility_store_stats.end()) << std::endl;
    std::cout << "AVG: " << sum / time_in_facility_store_stats.size() << std::endl;
    std::cout << "---------------------------------------------------\n\n";

    sum = 0;
    std::cout << "-------- PROCESS LIFETIME IN SYSTEM --------\n";
    for (auto i : time_in_system_stats) {
        sum += i;
        printf("%08.2f ", i);
        printStars(static_cast<int>(i / 10));
    }
    std::cout << "--------------------------------------------\n";
    std::cout << "MIN: " << *std::min_element(time_in_system_stats.begin(), time_in_system_stats.end()) << std::endl;
    std::cout << "MAX: " << *std::max_element(time_in_system_stats.begin(), time_in_system_stats.end()) << std::endl;
    std::cout << "AVG: " << sum / time_in_system_stats.size() << std::endl;
    std::cout << "--------------------------------------------\n\n";
}

std::string simExp::getMessage() {
    return message;
}

int simExp::getRC() {
    return return_code;
}
