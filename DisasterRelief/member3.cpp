// =============================================================
//  MEMBER 3 — Command Center + Alerts + Filing + Main
//  (The "Voice": Admin Dashboard + Notifications + main())
//
//  OOP Concepts Covered:
//   Week 4  → Static const arrays (SystemConfig translations)
//   Week 7  → Abstract Notification class
//   Week 10 → Aggregation (AdminDashboard references managers)
//   Week 13 → Exception Handling (try/catch)
//   Week 14 → File Handling (fstream)
//   Week 15 → Stack using dynamic array (ReportStack)
// =============================================================

#ifndef MEMBER3_H
#define MEMBER3_H

#include "member2.h"
#include <fstream>

// ─────────────────────────────────────────────
//  WEEK 4 — Static + Const arrays for multi-language support
// ─────────────────────────────────────────────

class SystemConfig {
private:
    static int currentLang;   // 0=English, 1=Urdu, 2=Sindhi, 3=Pashto

public:
    // WEEK 4 — const static arrays (compile-time constants)
    static const string welcomeMsg[4];
    static const string reportLabel[4];
    static const string donationLabel[4];

    static void chooseLang() {
        cout << "\n  Select Language:\n"
             << "  [0] English\n  [1] Urdu (Khushamdeed)\n"
             << "  [2] Sindhi (Bhalli Kare)\n  [3] Pashto (Har Kala)\n"
             << "  Choice: ";
        cin >> currentLang;
        if (currentLang < 0 || currentLang > 3) currentLang = 0;
    }

    static void printWelcome() {
        cout << "\n  " << welcomeMsg[currentLang] << "\n";
    }

    static void printReportLabel() {
        cout << "  " << reportLabel[currentLang] << "\n";
    }
};

int SystemConfig::currentLang = 0;

const string SystemConfig::welcomeMsg[4] = {
    "Welcome to Disaster Relief Coordination System",
    "Khushamdeed — Disaster Relief System",
    "Bhalli Kare Aaya — Rehat Nizam",
    "Har Kala Rasha — Da Musibat Madad Nizam"
};
const string SystemConfig::reportLabel[4] = {
    "Active Reports",
    "Faryal Reports",
    "Raporton ji List",
    "Raportona Lista"
};
const string SystemConfig::donationLabel[4] = {
    "Donations Received",
    "Amdan Mili",
    "Aamadan Milya",
    "Aamadan Rasedali"
};

// ─────────────────────────────────────────────
//  WEEK 7 — Abstract Notification class
//  All alert types MUST implement send()
// ─────────────────────────────────────────────

class Notification {
public:
    virtual ~Notification() {}
    virtual void send(const string& area, const string& msg) = 0;  // pure virtual
};

class SmsAlert : public Notification {
public:
    void send(const string& area, const string& msg) override {
        cout << "  [SMS  → " << area << "] " << msg << "\n";
    }
};

class EmailAlert : public Notification {
public:
    void send(const string& area, const string& msg) override {
        cout << "  [EMAIL→ " << area << "] " << msg << "\n";
    }
};

// AlertSystem holds multiple channels (polymorphic)
class AlertSystem {
private:
    vector<shared_ptr<Notification>> channels;
public:
    void addChannel(shared_ptr<Notification> n) { channels.push_back(n); }

    void alert(const string& area, const string& msg) {
        for (auto& c : channels) c->send(area, msg);
    }
};

// ─────────────────────────────────────────────
//  WEEK 14 — File Handling (fstream)
//  WEEK 13 — Exception Handling (try / catch)
// ─────────────────────────────────────────────

class DatabaseManager {
public:
    // Save all reports to a .txt file
    static void saveReports(const string& filename,
                            vector<DisasterReport>& reports) {
        try {
            ofstream file(filename);
            if (!file.is_open())
                throw runtime_error("Cannot open " + filename);

            for (auto& r : reports)
                file << r << "\n";   // uses overloaded <<

            file.close();
            cout << "  [DB] Reports saved to " << filename << "\n";
        }
        catch (const runtime_error& e) {
            cout << "  [DB ERROR] " << e.what() << "\n";
        }
    }

    // Save total donation fund
    static void saveFunds(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open())
                throw runtime_error("Cannot open " + filename);

            file << "Total Relief Fund: Rs." << DonationManager::getFund() << "\n";
            file.close();
            cout << "  [DB] Funds saved to " << filename << "\n";
        }
        catch (const runtime_error& e) {
            cout << "  [DB ERROR] " << e.what() << "\n";
        }
    }
};

// ─────────────────────────────────────────────
//  WEEK 10 — Aggregation
//  AdminDashboard does NOT own managers — it just references them
// ─────────────────────────────────────────────

class AdminDashboard {
private:
    ReportManager*    reportMgr;     // aggregation (pointer, not ownership)
    VolunteerManager* volunteerMgr;
    LogisticsManager* logisticsMgr;
public:
    AdminDashboard(ReportManager* r, VolunteerManager* v, LogisticsManager* l)
        : reportMgr(r), volunteerMgr(v), logisticsMgr(l) {}

    void showStats() {
        int verified = 0, critical = 0;
        for (const auto& r : reportMgr->getAll()) {
            if (r.getVerified())  verified++;
            if (r.getSeverity() >= 5) critical++;
        }

        cout << "\n  ╔══════════════════════════════════╗\n"
             << "  ║     ADMIN EMERGENCY DASHBOARD    ║\n"
             << "  ╚══════════════════════════════════╝\n"
             << "  Total Reports   : " << reportMgr->getAll().size()   << "\n"
             << "  Verified        : " << verified                      << "\n"
             << "  Critical (Sev 5): " << critical                      << "\n"
             << "  Volunteers Ready: " << volunteerMgr->count()         << "\n"
             << "  Relief Fund     : Rs." << DonationManager::getFund() << "\n";

        cout << "\n  ── Camp Status ─────────────────────\n";
        logisticsMgr->showAll();
    }

    // Admin manually verifies a pending report
    void verifyReport(int id) {
        for (auto& r : reportMgr->getAll()) {
            if (r.getID() == id) {
                Verifier::check(r);
                PriorityEngine::calculate(r);
                return;
            }
        }
        cout << "  Report #" << id << " not found.\n";
    }
};

// ─────────────────────────────────────────────
//  WEEK 15 — Stack using dynamic array
//  Stores report IDs for quick "last-in, first-out" processing
// ─────────────────────────────────────────────

class ReportStack {
private:
    int* arr;      // dynamic array
    int  capacity;
    int  top;
public:
    ReportStack(int cap = 50)
        : capacity(cap), top(-1) {
        arr = new int[capacity];   // dynamic memory
    }

    ~ReportStack() { delete[] arr; }  // WEEK 3 — Destructor frees memory

    void push(int id) {
        if (top >= capacity - 1) {
            cout << "  [Stack] Overflow!\n";
            return;
        }
        arr[++top] = id;
    }

    int pop() {
        if (top < 0) throw runtime_error("Stack underflow!");
        return arr[top--];
    }

    int peek() const {
        if (top < 0) throw runtime_error("Stack is empty!");
        return arr[top];
    }

    bool isEmpty() const { return top == -1; }

    void showAll() const {
        cout << "  Stack (top→bottom): ";
        for (int i = top; i >= 0; i--)
            cout << arr[i] << " ";
        cout << "\n";
    }
};

#endif // MEMBER3_H

// ─────────────────────────────────────────────
//  MAIN — Brings everything together
// ─────────────────────────────────────────────

int main() {
    // ── Language Selection ──
    SystemConfig::chooseLang();
    SystemConfig::printWelcome();

    // ── Setup Core Managers ──
    ReportManager    reportMgr;
    VolunteerManager volunteerMgr;
    LogisticsManager logisticsMgr;
    AlertSystem      alertSystem;

    alertSystem.addChannel(make_shared<SmsAlert>());
    alertSystem.addChannel(make_shared<EmailAlert>());

    // ── Pre-load Relief Camps ──
    logisticsMgr.addCamp(ReliefCamp("Karachi", 200));
    logisticsMgr.addCamp(ReliefCamp("Swat",    150));
    logisticsMgr.addCamp(ReliefCamp("Quetta",  100));

    // ── Register Volunteers ──
    volunteerMgr.add(make_shared<MedicalVolunteer>(1, "Ali",    "Karachi", "Doctor"));
    volunteerMgr.add(make_shared<MedicalVolunteer>(2, "Sana",   "Swat",    "Nurse"));
    volunteerMgr.add(make_shared<TechVolunteer>   (3, "Tariq",  "Quetta",  "Driver"));
    volunteerMgr.add(make_shared<SeniorVolunteer> (4, "Dr.Omar","Karachi", "Surgeon", "★ Senior Field Lead"));

    // ──────────────────────────────────────────────
    //  MAIN MENU LOOP
    // ──────────────────────────────────────────────
    ReportStack urgentStack;
    int choice = 0;

    do {
        cout << "\n  ┌──────────────────────────────────┐\n"
             << "  │   DISASTER RELIEF SYSTEM MENU   │\n"
             << "  ├──────────────────────────────────┤\n"
             << "  │  1. Submit a Disaster Report     │\n"
             << "  │  2. Make a Donation              │\n"
             << "  │  3. Deploy Volunteers            │\n"
             << "  │  4. View All Reports             │\n"
             << "  │  5. Admin Dashboard              │\n"
             << "  │  6. Send Emergency Alert         │\n"
             << "  │  7. Save Data to Files           │\n"
             << "  │  8. View Urgent Stack            │\n"
             << "  │  0. Exit                         │\n"
             << "  └──────────────────────────────────┘\n"
             << "  Choice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            // ── Submit Report ──
            string city, area, repName, cnic, repType, disType, desc;
            int sev;

            cout << "\n  City: ";        getline(cin, city);
            cout << "  Area: ";         getline(cin, area);
            cout << "  Your Name: ";    getline(cin, repName);
            cout << "  CNIC: ";         getline(cin, cnic);
            cout << "  Type (Civilian/Officer): "; getline(cin, repType);
            cout << "  Disaster Type (Medical/Fire/Flood/Earthquake/Other): ";
            getline(cin, disType);
            cout << "  Description: ";  getline(cin, desc);
            cout << "  Severity (1-5): "; cin >> sev;
            cin.ignore();

            DisasterReport r(
                Location(city, area),
                Reporter(repName, cnic, repType),
                disType, desc, sev
            );
            reportMgr.addReport(r);
            cout << "  ✓ Report submitted and processed.\n";

            // Push critical reports to urgent stack
            if (sev >= 4)
                urgentStack.push(r.getID());
        }

        else if (choice == 2) {
            // ── Donations ──
            cout << "\n  Donation type:\n"
                 << "  [1] Cash\n  [2] Items (Food/Medicine)\n  Choice: ";
            int t; cin >> t; cin.ignore();

            string donor;
            cout << "  Donor name: "; getline(cin, donor);

            if (t == 1) {
                double amt;
                cout << "  Amount (Rs.): "; cin >> amt; cin.ignore();
                CashDonation cd(donor, amt);
                cd.process(logisticsMgr);
            } else {
                string itemType, itemCity;
                int qty;
                cout << "  Item (Food/Medicine): "; getline(cin, itemType);
                cout << "  Quantity: ";              cin >> qty;  cin.ignore();
                cout << "  Destination city: ";      getline(cin, itemCity);
                ItemDonation id(donor, itemType, qty, itemCity);
                id.process(logisticsMgr);
            }
        }

        else if (choice == 3) {
            // ── Deploy Volunteers ──
            string city, task;
            cout << "\n  City to deploy in: "; getline(cin, city);
            cout << "  Task description : "; getline(cin, task);
            volunteerMgr.deployByCity(city, task);
        }

        else if (choice == 4) {
            // ── Show Reports ──
            SystemConfig::printReportLabel();
            reportMgr.sortByPriority();
            reportMgr.showAll();
        }

        else if (choice == 5) {
            // ── Admin Dashboard ──
            AdminDashboard admin(&reportMgr, &volunteerMgr, &logisticsMgr);
            admin.showStats();
        }

        else if (choice == 6) {
            // ── Alerts ──
            string area, msg;
            cout << "\n  Target city/area: "; getline(cin, area);
            cout << "  Alert message  : "; getline(cin, msg);
            alertSystem.alert(area, msg);
        }

        else if (choice == 7) {
            // ── Save Data ── (File Handling + Exception Handling)
            DatabaseManager::saveReports("reports.txt", reportMgr.getAll());
            DatabaseManager::saveFunds("funds.txt");
        }

        else if (choice == 8) {
            // ── Urgent Stack ──
            cout << "\n  Urgent Report IDs (stack):\n";
            urgentStack.showAll();

            // WEEK 15 + WEEK 13 — stack pop with exception handling
            try {
                cout << "  Top urgent report ID: " << urgentStack.peek() << "\n";
            }
            catch (const runtime_error& e) {
                cout << "  " << e.what() << "\n";
            }
        }

    } while (choice != 0);

    cout << "\n  Thank you. Stay safe!\n";
    return 0;
}
