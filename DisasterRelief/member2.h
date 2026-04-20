// =============================================================
//  MEMBER 2 — Resource & Volunteer Hub
//  (The "Hands": Volunteers + Relief Camps + Donations)
//
//  OOP Concepts Covered:
//   Week 7  → Abstract Classes & Pure Virtual Functions
//   Week 6  → Operator Overloading (+=, ++)
//   Week 5  → Inheritance (MedicalVol, TechVol from Volunteer)
//   Week 6  → Function Overloading (process() variations)
//   Week 10 → Composition (LogisticsManager HAS ReliefCamp)
//   Week 11 → Multiple Inheritance (SkillVolunteer demo)
//   Week 12 → Class Template (ItemStack<T>)
//   Week 13 → STL map for city-based camp lookup
// =============================================================

#ifndef MEMBER2_H
#define MEMBER2_H

#include "member1.h"
#include <map>
#include <memory>   // shared_ptr

// ─────────────────────────────────────────────
//  WEEK 7 — Abstract Class + Pure Virtual Function
//  Any class that derives from Volunteer MUST implement deploy()
// ─────────────────────────────────────────────

class Volunteer {
protected:
    int    id;
    string name;
    string city;
public:
    Volunteer(int i, string n, string c)
        : id(i), name(n), city(c) {}

    virtual ~Volunteer() {}

    // WEEK 7 — Pure Virtual Function (makes this class Abstract)
    virtual void deploy(const string& task) = 0;

    // Normal virtual function (can be overridden but has a default)
    virtual void showInfo() const {
        cout << "  Volunteer : " << name << " | City: " << city << "\n";
    }

    string getCity() const { return city; }
    string getName() const { return name; }
};

// ─────────────────────────────────────────────
//  WEEK 5 — Inheritance from Volunteer
//  WEEK 7 — Providing implementation of pure virtual deploy()
// ─────────────────────────────────────────────

class MedicalVolunteer : public Volunteer {
private:
    string specialization;  // e.g. "Doctor", "Nurse"
public:
    MedicalVolunteer(int i, string n, string c, string sp)
        : Volunteer(i, n, c), specialization(sp) {}

    // WEEK 7 — Implementing the pure virtual function
    void deploy(const string& task) override {
        cout << "  [Medical] " << name << " (" << specialization
             << ") → deployed for: " << task << "\n";
    }

    void showInfo() const override {
        cout << "  [Medical] " << name << " | " << specialization
             << " | City: " << city << "\n";
    }
};

class TechVolunteer : public Volunteer {
private:
    string skill;   // e.g. "Driver", "Electrician"
public:
    TechVolunteer(int i, string n, string c, string sk)
        : Volunteer(i, n, c), skill(sk) {}

    void deploy(const string& task) override {
        cout << "  [Tech]    " << name << " (" << skill
             << ") → deployed for: " << task << "\n";
    }

    void showInfo() const override {
        cout << "  [Tech]    " << name << " | " << skill
             << " | City: " << city << "\n";
    }
};

// ─────────────────────────────────────────────
//  WEEK 11 — Multiple Inheritance
//  A Senior Volunteer can have BOTH Medical & Tech roles
// ─────────────────────────────────────────────

class SeniorProfile {
public:
    string badge;
    SeniorProfile(string b) : badge(b) {}
    void showBadge() const { cout << "  Badge: " << badge << "\n"; }
};

class SeniorVolunteer : public MedicalVolunteer, public SeniorProfile {
public:
    SeniorVolunteer(int i, string n, string c, string sp, string b)
        : MedicalVolunteer(i, n, c, sp), SeniorProfile(b) {}

    void deploy(const string& task) override {
        showBadge();
        MedicalVolunteer::deploy(task);
    }
};

// ─────────────────────────────────────────────
//  Volunteer Manager — stores all volunteers
// ─────────────────────────────────────────────

class VolunteerManager {
private:
    vector<shared_ptr<Volunteer>> volunteers;
public:
    void add(shared_ptr<Volunteer> v) { volunteers.push_back(v); }

    // WEEK 6 — Function Overloading: deploy by city OR by name
    void deployByCity(const string& city, const string& task) {
        bool found = false;
        for (auto& v : volunteers)
            if (v->getCity() == city) { v->deploy(task); found = true; }
        if (!found)
            cout << "  No volunteers found in " << city << "\n";
    }

    void deployByName(const string& name, const string& task) {
        for (auto& v : volunteers)
            if (v->getName() == name) { v->deploy(task); return; }
        cout << "  Volunteer " << name << " not found.\n";
    }

    void showAll() const {
        for (const auto& v : volunteers) v->showInfo();
    }

    int count() const { return (int)volunteers.size(); }
};

// ─────────────────────────────────────────────
//  WEEK 7 — Abstract Class for Resources
// ─────────────────────────────────────────────

class Resource {
protected:
    string city;
public:
    Resource(string c = "") : city(c) {}
    virtual ~Resource() {}

    virtual void showStatus() const = 0;   // pure virtual
    string getCity() const { return city; }
};

// ─────────────────────────────────────────────
//  WEEK 6 — Operator Overloading (+= and ++)
//  WEEK 10 — Composition (camp IS-A Resource and HAS stocks)
// ─────────────────────────────────────────────

class ReliefCamp : public Resource {
private:
    int occupants;
    int maxCapacity;
    int foodStock;
    int medicineStock;
public:
    ReliefCamp(string c = "", int cap = 100)
        : Resource(c), occupants(0), maxCapacity(cap),
          foodStock(0), medicineStock(0) {}

    // WEEK 6 — Operator Overloading: add food stock easily
    void operator+=(int food) { foodStock += food; }

    // WEEK 6 — Operator Overloading: admit one occupant
    ReliefCamp& operator++() {
        if (occupants < maxCapacity) occupants++;
        return *this;
    }

    void addMedicine(int qty) { medicineStock += qty; }

    // WEEK 7 — Implementing pure virtual from Resource
    void showStatus() const override {
        cout << "  Camp [" << city << "] "
             << "Occupants: " << occupants << "/" << maxCapacity
             << " | Food: " << foodStock
             << " | Medicine: " << medicineStock << "\n";
    }

    int getFoodStock()     const { return foodStock; }
    int getMedicineStock() const { return medicineStock; }
};

// ─────────────────────────────────────────────
//  WEEK 10 — Composition
//  LogisticsManager HAS a collection of ReliefCamps
// ─────────────────────────────────────────────

class LogisticsManager {
private:
    map<string, ReliefCamp> camps;  // WEEK 13 — STL map
public:
    void addCamp(const ReliefCamp& c) { camps[c.getCity()] = c; }

    ReliefCamp& getCamp(const string& city) {
        if (!camps.count(city))
            camps[city] = ReliefCamp(city, 100);
        return camps[city];
    }

    void showAll() const {
        for (const auto& pair : camps)
            pair.second.showStatus();
    }
};

// ─────────────────────────────────────────────
//  WEEK 7 — Abstract Donation Base Class
// ─────────────────────────────────────────────

class Donation {
protected:
    string donorName;
public:
    Donation(string d) : donorName(d) {}
    virtual ~Donation() {}

    virtual void process(LogisticsManager& lm) = 0;  // pure virtual
};

// ─────────────────────────────────────────────
//  WEEK 4 — Static member in DonationManager
// ─────────────────────────────────────────────

class DonationManager {
private:
    static double totalFund;  // WEEK 4 — static shared across all instances
public:
    static void addCash(double amount) { totalFund += amount; }
    static double getFund()            { return totalFund; }
};

double DonationManager::totalFund = 0.0;

// ─────────────────────────────────────────────
//  Concrete Donations
// ─────────────────────────────────────────────

class ItemDonation : public Donation {
private:
    string itemType;   // "Food" or "Medicine"
    int    quantity;
    string city;
public:
    ItemDonation(string donor, string type, int qty, string c)
        : Donation(donor), itemType(type), quantity(qty), city(c) {}

    void process(LogisticsManager& lm) override {
        if (itemType == "Food")
            lm.getCamp(city) += quantity;
        else if (itemType == "Medicine")
            lm.getCamp(city).addMedicine(quantity);

        cout << "  [Donation] " << donorName << " donated "
             << quantity << " " << itemType << " to " << city << "\n";
    }
};

class CashDonation : public Donation {
private:
    double amount;
public:
    CashDonation(string donor, double amt)
        : Donation(donor), amount(amt) {}

    void process(LogisticsManager&) override {
        DonationManager::addCash(amount);
        cout << "  [Cash]     " << donorName << " donated Rs."
             << amount << "\n";
    }
};

// ─────────────────────────────────────────────
//  WEEK 12 — Class Template
//  Generic stack to hold any type of relief item
// ─────────────────────────────────────────────

template <typename T>
class ItemStack {
private:
    vector<T> items;  // dynamic storage using vector
public:
    void push(T item) { items.push_back(item); }

    T pop() {
        if (items.empty()) throw runtime_error("Stack is empty!");
        T top = items.back();
        items.pop_back();
        return top;
    }

    T peek() const {
        if (items.empty()) throw runtime_error("Stack is empty!");
        return items.back();
    }

    bool empty() const { return items.empty(); }
    int  size()  const { return (int)items.size(); }
};

#endif // MEMBER2_H
