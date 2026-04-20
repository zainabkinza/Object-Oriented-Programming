// =============================================================
//  MEMBER 1 — Smart Intake System
//  (The "Brain": Reporting + Verification + Priority Scoring)
//
//  OOP Concepts Covered:
//   Week 2  → Encapsulation  (private data, public getters/setters)
//   Week 3  → Constructors & Destructors
//   Week 4  → Static members, const members
//   Week 5  → Inheritance   (DisasterReport inherits Report)
//   Week 6  → Operator Overloading  (>, << )
//   Week 10 → Friend Class  (Verifier & PriorityEngine)
//   Week 13 → STL vector used in ReportManager
// =============================================================

#ifndef MEMBER1_H
#define MEMBER1_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>   // transform
#include <ctime>       // time_t

using namespace std;

// ─────────────────────────────────────────────
//  WEEK 2 — Encapsulation
//  All data is private; accessed through public methods only
// ─────────────────────────────────────────────

class Location {
private:
    string city;
    string area;
public:
    // WEEK 3 — Constructor
    Location(string c = "Unknown", string a = "Unknown")
        : city(c), area(a) {}

    string getCity() const { return city; }
    string getArea() const { return area; }
};

// ─────────────────────────────────────────────
class Reporter {
private:
    string name;
    string cnic;
    string type;   // "Civilian" or "Officer"
public:
    Reporter(string n = "", string c = "", string t = "Civilian")
        : name(n), cnic(c), type(t) {}

    string getName() const { return name; }
    string getCNIC() const { return cnic; }
    string getType() const { return type; }
};

// ─────────────────────────────────────────────
//  WEEK 5 — Inheritance (Base Class)
//  Report is the parent — DisasterReport extends it
// ─────────────────────────────────────────────

class Report {
protected:
    Location location;
    Reporter reporter;
    string disasterType;   // "Flood", "Fire", "Medical", etc.
    string description;
    int    severity;       // 1 (low) → 5 (critical)

public:
    // WEEK 3 — Constructor with default args
    Report(Location loc, Reporter rep,
           string type, string desc, int sev)
        : location(loc), reporter(rep),
          disasterType(type), description(desc), severity(sev) {}

    // WEEK 3 — Virtual Destructor (good practice for base class)
    virtual ~Report() {}

    // Simple display override target for child
    virtual void display() const {
        cout << "  Type      : " << disasterType << "\n"
             << "  City      : " << location.getCity() << "\n"
             << "  Severity  : " << severity << "/5\n";
    }

    // Getters
    string getType()        const { return disasterType; }
    string getDescription() const { return description; }
    int    getSeverity()    const { return severity; }
    string getCity()        const { return location.getCity(); }
    Reporter getReporter()  const { return reporter; }
};

// ─────────────────────────────────────────────
//  Forward declarations for friend classes
// ─────────────────────────────────────────────
class Verifier;
class PriorityEngine;

// ─────────────────────────────────────────────
//  WEEK 5  — Inheritance (Child Class)
//  WEEK 4  — Static member (auto-increment ID)
//  WEEK 6  — Operator Overloading (> and <<)
//  WEEK 10 — Friend Class
// ─────────────────────────────────────────────

class DisasterReport : public Report {
private:
    // WEEK 4 — static counter shared across ALL objects
    static int totalReports;

    int    reportID;
    int    priorityScore;
    bool   isVerified;
    string status;   // "PENDING" / "VERIFIED" / "REJECTED"

public:
    // WEEK 3 — Constructor calls parent constructor
    DisasterReport(Location loc, Reporter rep,
                   string type, string desc, int sev)
        : Report(loc, rep, type, desc, sev),
          priorityScore(0), isVerified(false), status("PENDING")
    {
        reportID = ++totalReports;   // WEEK 4 — static usage
    }

    // WEEK 3 — Destructor
    ~DisasterReport() {}

    // WEEK 5 — Overriding parent display()
    void display() const override {
        cout << "  Report ID : " << reportID << "\n"
             << "  Status    : " << status << "\n"
             << "  Priority  : " << priorityScore << "\n";
        Report::display();   // call parent version too
        cout << "  Reporter  : " << reporter.getName()
             << " (" << reporter.getType() << ")\n";
    }

    // WEEK 6 — Operator Overloading (compare by priority)
    bool operator>(const DisasterReport& other) const {
        return this->priorityScore > other.priorityScore;
    }

    // WEEK 6 — Overloading << for easy printing
    friend ostream& operator<<(ostream& os, const DisasterReport& r) {
        os << r.reportID << "," << r.getCity() << ","
           << r.getType() << "," << r.getSeverity() << ","
           << r.isVerified << "," << r.status << ","
           << r.priorityScore;
        return os;
    }

    // WEEK 10 — Friend Classes
    friend class Verifier;
    friend class PriorityEngine;

    // WEEK 4 — Static function
    static int getTotalReports() { return totalReports; }

    // Getters
    int    getID()            const { return reportID; }
    int    getPriorityScore() const { return priorityScore; }
    bool   getVerified()      const { return isVerified; }
    string getStatus()        const { return status; }
};

// WEEK 4 — Static member defined outside class
int DisasterReport::totalReports = 0;

// ─────────────────────────────────────────────
//  WEEK 10 — Friend Class
//  Verifier has direct access to DisasterReport's private data
// ─────────────────────────────────────────────

class Verifier {
private:
    static string toLower(string s) {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }
public:
    // Scans description for fake/prank keywords
    static void check(DisasterReport& r) {
        string desc = toLower(r.description);

        // Flag if description sounds fake
        if (desc.find("prank") != string::npos ||
            desc.find("fake")  != string::npos ||
            desc.find("mazak") != string::npos)
        {
            r.isVerified = false;
            r.status     = "REJECTED";
            cout << "  [Verifier] Report #" << r.reportID
                 << " REJECTED — suspicious keywords found.\n";
            return;
        }

        // Flood in Islamabad? Needs extra check (not a flood-prone city)
        string type = toLower(r.disasterType);
        string city = toLower(r.location.getCity());
        if (type == "flood" && city != "karachi" && city != "swat") {
            r.status     = "NEEDS REVIEW";
            r.isVerified = false;
            cout << "  [Verifier] Report #" << r.reportID
                 << " flagged — flood unlikely in " << r.location.getCity() << ".\n";
            return;
        }

        r.isVerified = true;
        r.status     = "VERIFIED";
        cout << "  [Verifier] Report #" << r.reportID << " VERIFIED.\n";
    }
};

// ─────────────────────────────────────────────
//  WEEK 10 — Friend Class
//  PriorityEngine scores the report
// ─────────────────────────────────────────────

class PriorityEngine {
public:
    static void calculate(DisasterReport& r) {
        // Weight by disaster type
        int typeWeight = 20;
        string t = r.disasterType;
        transform(t.begin(), t.end(), t.begin(), ::tolower);

        if      (t == "medical")    typeWeight = 50;
        else if (t == "earthquake") typeWeight = 45;
        else if (t == "fire")       typeWeight = 40;
        else if (t == "flood")      typeWeight = 35;

        // Formula: (severity × 10) + typeWeight + verifiedBonus
        int verifiedBonus = r.isVerified ? 20 : 0;
        r.priorityScore   = (r.severity * 10) + typeWeight + verifiedBonus;
    }
};

// ─────────────────────────────────────────────
//  WEEK 13 — STL Container (vector)
//  ReportManager stores all reports and sorts them
// ─────────────────────────────────────────────

class ReportManager {
private:
    vector<DisasterReport> reports;  // WEEK 13 — STL vector

public:
    // Add, auto-verify and auto-score
    void addReport(DisasterReport r) {
        Verifier::check(r);
        PriorityEngine::calculate(r);
        reports.push_back(r);
    }

    // Sort by priority (uses overloaded > operator)
    void sortByPriority() {
        sort(reports.begin(), reports.end(),
             [](const DisasterReport& a, const DisasterReport& b) {
                 return a > b;
             });
    }

    vector<DisasterReport>& getAll() { return reports; }

    void showAll() const {
        for (const auto& r : reports) {
            cout << "\n  ── Report ──────────────────────\n";
            r.display();
        }
    }
};

#endif // MEMBER1_H
