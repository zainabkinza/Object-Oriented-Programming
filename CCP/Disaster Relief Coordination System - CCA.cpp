// DISASTER RELIEF COORDINATION SYSTEM 

// SECTION 1: HEADER FILES (LIBRARIES)

// Each header file adds specific functionality to the program
#include <iostream>     // For cout, cin - basic input/output operations
#include <string>       // For string class - handling text like names, CNIC
#include <vector>       // For vector - dynamic array that can grow/shrink
#include <algorithm>    // For transform, find, min - various algorithms
#include <fstream>      // For file operations - saving/loading data to disk
#include <ctime>        // For time() - getting current time for cooldowns
#include <cctype>       // For isdigit() - validating CNIC and phone numbers
#include <sstream>      // For stringstream - parsing pipe-separated data from files
#include <memory>       // For unique_ptr - automatic memory management (no memory leaks!)
#include <limits>       // For numeric_limits - clearing invalid input from cin
#include <iomanip>      // For setw, setfill, setprecision - formatting output
#include <map>          // For map - dictionary for CNIC to person type lookup
#include <cstdlib>      // For system() - clearing console screen
#ifdef _WIN32
#include <windows.h>    // Windows-specific functions (only on Windows)
#else
#include <unistd.h>     // Linux/Mac specific functions (only on non-Windows)
#endif
using namespace std;    // So we can write 'cout' instead of 'std::cout'

// SECTION 2: CONSTANTS (Fixed values that never change)

// These numbers control how resources are calculated based on disaster severity
const int FOOD_PER_SEVERITY = 100;      // Severity 5 needs 500 food units
const int MEDICINE_PER_SEVERITY = 50;   // Severity 5 needs 250 medicine units
const int FUNDS_PER_SEVERITY = 10000;   // Severity 5 needs Rs. 50,000

const int MIN_SEVERITY = 1;             // Lowest disaster severity (minor issue)
const int MAX_SEVERITY = 10;            // Highest disaster severity (catastrophic)

// File names where data is stored on hard drive
const string REPORTS_FILE = "reports_data.txt";        // Stores all disaster reports
const string VOLUNTEERS_FILE = "volunteers_data.txt";  // Stores all volunteers
const string CAMPS_FILE = "camps_data.txt";            // Stores all relief camps
const string FUNDS_FILE = "funds_data.txt";            // Stores total donation money
const string EVACUATIONS_FILE = "evacuations_data.txt"; // Stores evacuation orders

const string OFFICER_SECRET_CODE = "131";   // Secret code for government officers

// Cooldown periods to prevent report spam (in seconds)
const int SAME_AREA_COOLDOWN_SECONDS = 24 * 3600;        // 24 hours
const int DIFFERENT_AREA_COOLDOWN_SECONDS = 30 * 24 * 3600; // 30 days

// SECTION 3: FORWARD DECLARATIONS (Sign that these classes will be defined later)

// Tells the compiler: These class names exist, will be defined below
class DisasterReport;
class ReliefCamp;
class Volunteer;
class ItemDonation;

// SECTION 4: GLOBAL VARIABLES (Accessible from anywhere in the program)

// These hold ALL the data for the entire system
vector<DisasterReport> allReports;                      // Every disaster report ever submitted
vector<unique_ptr<Volunteer>> allVolunteers;            // Every registered volunteer (smart pointers = auto cleanup)
vector<ReliefCamp> allCamps;                            // Every relief camp in the country
map<string, string> cnicTypeMap;                        // Maps CNIC -> "Civilian" or "Officer" (prevents double registration)

// SECTION 5: EVACUATION RECORD STRUCTURE

// A simple data container (struct = all members public by default)
struct EvacuationRecord {
    string area;            // Specific neighborhood/sector (e.g., "Gulshan-e-Iqbal")
    string city;            // City name (e.g., "Karachi")
    string disasterType;    // Type of disaster (FLOOD, EARTHQUAKE, etc.)
    int severity;           // How severe (1-10 scale)
    time_t issuedAt;        // When evacuation was ordered (seconds since 1970)
    bool lifted;            // Is evacuation still active? (false = active, true = lifted)
};
vector<EvacuationRecord> allEvacuations;   // List of all evacuation orders ever issued

// SECTION 6: UTILITY FUNCTIONS (Small helper functions used everywhere)

// Clears the console screen - works on both Windows and Linux/Mac
void clearScreen() {
#ifdef _WIN32
    system("cls");      // Windows command to clear screen
#else
    system("clear");    // Linux/Mac command to clear screen
#endif
}

// Makes the computer beep - used for emergency alerts
void printBeep() { cout << '\a'; }  // '\a' is the bell/alert character

// Pauses the program until user presses Enter - lets user read messages
void waitForEnter() {
    cout << "\nPress Enter to continue...";
    cin.ignore();   // Discard any leftover characters in input buffer
    cin.get();      // Wait for Enter key press
}

// Converts a number like 1000000 to "Rs. 1,000,000" with commas
string formatCurrency(double amount) {
    stringstream ss;                        // Create a string builder
    ss << fixed << setprecision(0) << amount; // No decimals, no scientific notation
    string str = ss.str();                  // Convert to regular string
    int len = str.length();                 // Get length (e.g., 7 for "1000000")
    // Insert commas from right to left, every 3 digits
    for (int i = len - 3; i > 0; i -= 3)
        str.insert(i, ",");                 // "1000000" -> "1,000,000"
    return "Rs. " + str;                    // Add currency prefix
}

// SECTION 7: LANGUAGE SYSTEM (Multi-language support using Singleton pattern)

// All static members = one copy shared across entire program
class SystemConfig {
    static int currentLang;                     // 1=English, 2=Urdu, 3=Sindhi, 4=Pashto
    static const string welcomeMsgs[4];         // Welcome messages in 4 languages
    static const string menuMsgs[4][7];         // Menu options in 4 languages × 7 options

   public:
    static void setLang(int c) { currentLang = (c >= 1 && c <= 4) ? c : 1; } // Validate range
    static int getLang() { return currentLang; }
    
    // Simulates audio announcement for accessibility
    static void displayLanguageMessage() {
        cout << "\n[*AUDIO PLAYING FOR LANGUAGE: ";
        if (currentLang == 1) cout << "English*";
        else if (currentLang == 2) cout << "Urdu*";
        else if (currentLang == 3) cout << "Sindhi*";
        else cout << "Pashto*";
        cout << "]" << endl;
        printBeep();    // Beep to simulate audio
    }
    
    static void printWelcome() { cout << welcomeMsgs[currentLang - 1] << endl; }
    
    static void printMenu() {
        for (int i = 0; i < 7; i++)
            cout << menuMsgs[currentLang - 1][i] << endl;
    }
};

// Initialize static members (must be defined outside the class)
int SystemConfig::currentLang = 1;  // Start with English

// Welcome messages for each language
const string SystemConfig::welcomeMsgs[4] = {
    "Welcome to Disaster Relief Coordination System - Pakistan",      // English
    "Khushamdeed - Nizam e Qudrati Aafaat o Imdaad - Pakistan",     // Urdu
    "Bhalli Kare Aaya - Aafatan je Intizamia ain Ham-Ahangi jo Nizam - Pakistan",         // Sindhi
    "Har kala rasha - Da aafatuno da jawab warkolo ao hamghag-ay nizam - Pakistan"};          // Pashto

// Menu messages: [language][option_number]
const string SystemConfig::menuMsgs[4][7] = {
    // ENGLISH (language 1)
    {"1. Submit Report", "2. Register as Volunteer", "3. Donate", "4. Emergency Numbers", "5. Admin Login", "6. Exit", "Choice: "},
    // URDU (language 2)
    {"1. Report jama karein", "2. Razakaar banein", "3. Atiyya dein", "4. Hangaami Imdaadi Number", "5. Admin laagin", "6. Bahir jayein", "Intikhab: "},
    // SINDHI (language 3)
    {"1. Report jama kayo", "2. Razakaar thao", "3. Chando deyan", "4. Hangami Haalat ja Number", "5. Admin laagin", "6. Bahar wano", "Chuno: "},
    // PASHTO (language 4)
    {"1. Report darj ka", "2. Volunteer jorsha", "3. Donation oka", "4. Be?anai nambarona", "5. Admin dakhill sha oza", "6. Waza", "Intikhab: "}};

// SECTION 8: LOCATION CLASS (Stores where a disaster happened)

// COMPOSITION: Location objects are embedded inside DisasterReport
class Location {
    string city, area, landmark;    // Private = hidden from outside

   public:
    Location() : city(""), area(""), landmark("") {}  // Default constructor
    Location(string c, string a, string l) : city(c), area(a), landmark(l) {}  // Parameterized
    
    // Getter methods (const means they don't modify the object)
    string getCity() const { return city; }
    string getArea() const { return area; }
    string getLandmark() const { return landmark; }
    
    void display() const { cout << city << ", " << area << " (Near: " << landmark << ")"; }
    bool isSameArea(const Location& o) const { return city == o.city && area == o.area; }
    string getAreaKey() const { return city + "|" + area; }  // Unique key for maps
};

// SECTION 9: REPORTER CLASS (Person who submits a disaster report)

// Validates CNIC (Pakistan ID card), phone number, and officer code
class Reporter {
    string cnic, name, phone, type, officerCode;
    bool verified;    // True if CNIC and phone are valid
    
    // PRIVATE VALIDATION METHODS 
    
    // Validates Pakistani CNIC (13 digits, not all same, not starting with 0)
    bool validateCNIC(const string& cn) {
        string c = "";
        // Extract only digits (ignore dashes, spaces)
        for (char ch : cn) if (isdigit(ch)) c += ch;
        if (c.length() != 13 || c[0] == '0') return false;  // Wrong length or starts with 0
        // Check if all digits are same (e.g., "1111111111111" is invalid)
        bool allSame = true;
        for (int i = 1; i < 13; i++) if (c[i] != c[0]) { allSame = false; break; }
        return !allSame;    // Return true if NOT all same
    }
    
    // Validates Pakistani phone number (03XXXXXXXXX or 3XXXXXXXXX)
    bool validatePhone(const string& ph) {
        string c = "";
        for (char ch : ph) if (isdigit(ch)) c += ch;
        if (c.length() == 11) return c[0] == '0' && c[1] == '3';  // 03 format
        if (c.length() == 10) return c[0] == '3';                  // 3 format
        return false;
    }
    
    // Validates officer code (must be exactly "131")
    bool validateOfficerCode(const string& code) {
        if (code.length() != 3) return false;
        for (char c : code) if (!isdigit(c)) return false;
        return code == OFFICER_SECRET_CODE;  // Compare with "131"
    }

   public:
    Reporter() : cnic(""), name(""), phone(""), type("Civilian"), officerCode(""), verified(false) {}
    
    // Main constructor - does ALL validation
    Reporter(string c, string n, string p, string t, string offCode = "") 
        : name(n), phone(p), type(t), officerCode(offCode), verified(false) {
        
        bool cnicValid = validateCNIC(c), phoneValid = validatePhone(p);
        
        // Check if this CNIC is already registered with a DIFFERENT type
        if (cnicTypeMap.find(c) != cnicTypeMap.end() && cnicTypeMap[c] != t) {
            verified = false; cnic = "INVALID";
            cout << "\n[ERROR] This CNIC is already registered in the system.\n";
            return;
        }
        
        if (type == "Officer") {
            // Officer needs valid CNIC, phone, AND officer code
            if (cnicValid && phoneValid && validateOfficerCode(offCode)) {
                cnic = c; verified = true;
                if (cnicTypeMap.find(c) == cnicTypeMap.end()) cnicTypeMap[c] = "Officer";
            } else { 
                cnic = "INVALID"; verified = false; 
                cout << "\n[ERROR] Invalid Code!" << endl; 
            }
        } else {
            // Civilian only needs valid CNIC and phone
            if (cnicValid && phoneValid) {
                cnic = c; verified = true;
                if (cnicTypeMap.find(c) == cnicTypeMap.end()) cnicTypeMap[c] = "Civilian";
            } else if (!cnicValid) { 
                cnic = "INVALID"; verified = false; 
                cout << "\n[ERROR] Invalid CNIC!" << endl; 
            } else { 
                cnic = "INVALID"; verified = false; 
                cout << "\n[ERROR] Invalid Phone Number!" << endl; 
            }
        }
    }
    
    // Getter methods
    bool isVerified() const { return verified; }
    string getName() const { return name; }
    string getType() const { return type; }
    string getPhone() const { return phone; }
    string getCNIC() const { return cnic; }
    string getOfficerCode() const { return officerCode; }
    
    void display() const {
        cout << name << " (" << type;
        if (type == "Officer" && verified) cout << ":****";
        cout << ") - CNIC: " << (verified ? "Verified" : "INVALID") 
             << " | Phone: " << (verified ? "Verified" : "INVALID");
    }
    
    string toFileString() const { 
        return cnic + "|" + name + "|" + phone + "|" + type + "|" + officerCode + "|" + (verified ? "1" : "0"); 
    }
};

// SECTION 10: DISASTER TYPES (Polymorphic hierarchy using INHERITANCE)

// Why inheritance? Each disaster type has different weight, instructions, and evacuation rules

enum DisasterCategory { FLOOD, EARTHQUAKE, CYCLONE, LANDSLIDE, FIRE, DROUGHT, PANDEMIC };

// ABSTRACT BASE CLASS - Cannot create objects of this type directly
class Disaster {
   protected:
    string description;
    DisasterCategory category;
    int severity;
    time_t reportTime;   // When disaster was reported (for cooldown calculations)

   public:
    Disaster() : description(""), severity(0) { reportTime = time(0); }
    Disaster(string desc, DisasterCategory cat, int sev) 
        : description(desc), category(cat), severity(sev) { reportTime = time(0); }
    
    // Pure virtual functions = must be overridden by child classes
    virtual int getWeight() const = 0;          // Urgency factor (higher = more urgent)
    virtual string getTypeName() const = 0;     // "FLOOD", "EARTHQUAKE", etc.
    virtual string getInstructions() const = 0; // What response is needed
    
    virtual bool requiresEvacuation() const { return false; }  // Default: no evacuation
    
    int getSeverity() const { return severity; }
    string getDescription() const { return description; }
    DisasterCategory getCategory() const { return category; }
    time_t getReportTime() const { return reportTime; }
    
    virtual ~Disaster() {}  // Virtual destructor = proper cleanup of derived objects
};

// Each specific disaster type OVERRIDES the virtual functions
class FloodDisaster : public Disaster {
   public:
    FloodDisaster(string d, int s) : Disaster(d, FLOOD, s) {}
    int getWeight() const override { return 30; }
    string getTypeName() const override { return "FLOOD"; }
    string getInstructions() const override { return "Send boats, sandbags, and evacuation shelters."; }
    bool requiresEvacuation() const override { return severity >= 6; }  // Evacuate if severe
};

class EarthquakeDisaster : public Disaster {
   public:
    EarthquakeDisaster(string d, int s) : Disaster(d, EARTHQUAKE, s) {}
    int getWeight() const override { return 50; }   // Earthquakes are VERY urgent
    string getTypeName() const override { return "EARTHQUAKE"; }
    string getInstructions() const override { return "Send search & rescue teams and trauma kits."; }
    bool requiresEvacuation() const override { return severity >= 5; }
};

class CycloneDisaster : public Disaster {
   public:
    CycloneDisaster(string d, int s) : Disaster(d, CYCLONE, s) {}
    int getWeight() const override { return 45; }
    string getTypeName() const override { return "CYCLONE"; }
    string getInstructions() const override { return "Order early evacuation and secure shelters."; }
    bool requiresEvacuation() const override { return true; }   // Always evacuate for cyclones!
};

class LandslideDisaster : public Disaster {
   public:
    LandslideDisaster(string d, int s) : Disaster(d, LANDSLIDE, s) {}
    int getWeight() const override { return 40; }
    string getTypeName() const override { return "LANDSLIDE"; }
    string getInstructions() const override { return "Deploy heavy machinery and rescue teams."; }
    bool requiresEvacuation() const override { return severity >= 7; }
};

class FireDisaster : public Disaster {
   public:
    FireDisaster(string d, int s) : Disaster(d, FIRE, s) {}
    int getWeight() const override { return 35; }
    string getTypeName() const override { return "FIRE"; }
    string getInstructions() const override { return "Dispatch fire trucks and burn treatment units."; }
    bool requiresEvacuation() const override { return severity >= 7; }
};

class DroughtDisaster : public Disaster {
   public:
    DroughtDisaster(string d, int s) : Disaster(d, DROUGHT, s) {}
    int getWeight() const override { return 25; }   // Less urgent than earthquakes
    string getTypeName() const override { return "DROUGHT"; }
    string getInstructions() const override { return "Arrange water tankers and food supplies."; }
    bool requiresEvacuation() const override { return false; }  // Drought doesn't require moving people
};

class PandemicDisaster : public Disaster {
   public:
    PandemicDisaster(string d, int s) : Disaster(d, PANDEMIC, s) {}
    int getWeight() const override { return 55; }   // Highest urgency
    string getTypeName() const override { return "PANDEMIC"; }
    string getInstructions() const override { return "Set up hospitals and enforce lockdown."; }
    bool requiresEvacuation() const override { return severity >= 8; }
};

// FACTORY FUNCTION - Creates the correct disaster type based on string input
Disaster* createDisaster(const string& type, const string& desc, int severity) {
    string t = type;
    transform(t.begin(), t.end(), t.begin(), ::toupper);  // Convert to uppercase for case-insensitive matching
    
    if (t == "FLOOD")      return new FloodDisaster(desc, severity);
    if (t == "EARTHQUAKE") return new EarthquakeDisaster(desc, severity);
    if (t == "CYCLONE")    return new CycloneDisaster(desc, severity);
    if (t == "LANDSLIDE")  return new LandslideDisaster(desc, severity);
    if (t == "FIRE")       return new FireDisaster(desc, severity);
    if (t == "DROUGHT")    return new DroughtDisaster(desc, severity);
    if (t == "PANDEMIC")   return new PandemicDisaster(desc, severity);
    return nullptr;  // Unknown disaster type
}


// SECTION 11: FORWARD DECLARATIONS FOR INPUT FUNCTIONS

int getValidInt(const string& prompt, int minVal, int maxVal);
double getValidDouble(const string& prompt);

// SECTION 12: STRATEGY PATTERN (For priority calculation - can be swapped at runtime)

// Allows different algorithms for calculating report priority

class PriorityStrategy {
   public:
    virtual int calculate(int severity, int weight, bool isOfficer) = 0;  // Pure virtual
    virtual ~PriorityStrategy() {}
};

// Standard formula: severity*10 + weight + (officer?20:5)
class StandardPriority : public PriorityStrategy {
   public:
    int calculate(int severity, int weight, bool isOfficer) override {
        return severity * 10 + weight + (isOfficer ? 20 : 5);
    }
};

// Urgent formula: severity*15 + weight*2 + (officer?30:10) - gives higher scores
class UrgentPriority : public PriorityStrategy {
   public:
    int calculate(int severity, int weight, bool isOfficer) override {
        return (severity * 15) + (weight * 2) + (isOfficer ? 30 : 10);
    }
};

// SECTION 13: DISASTER REPORT CLASS (Main data structure)

// COMPOSITION: Contains Location and Reporter objects
// AGGREGATION: Contains pointer to Disaster (polymorphism)
class DisasterReport {
    static int nextId;          // Static = shared across all reports, auto-increments
    int id;                      // Unique ID for this report (starts at 1001)
    Location location;           // COMPOSITION - where disaster happened
    Reporter reporter;           // COMPOSITION - who reported it
    Disaster* disaster;          // AGGREGATION - pointer to disaster object (polymorphism)
    int priorityScore;           // Higher score = more urgent
    bool verified;               // Has Verifier checked this report?
    string status;               // PENDING, VERIFIED, REJECTED, RESOLVED
    bool resolved;               // Has disaster been handled?
    time_t dispatchTime;         // When resources were sent (0 if not dispatched)
    
    // Helper for deep copying Disaster objects (polymorphic cloning)
    Disaster* cloneDisaster(const Disaster* src) const { 
        return src ? createDisaster(src->getTypeName(), src->getDescription(), src->getSeverity()) : nullptr; 
    }

   public:
    // CONSTRUCTORS 
    DisasterReport() : id(0), disaster(nullptr), priorityScore(0), verified(false), 
                       status("PENDING"), resolved(false), dispatchTime(0) {}
    
    // Main constructor - assigns new ID from nextId
    DisasterReport(Location loc, Reporter rep, Disaster* dis) 
        : location(loc), reporter(rep), disaster(dis), priorityScore(0), 
          verified(false), status("PENDING"), resolved(false), dispatchTime(0) { 
        id = nextId++; 
    }
    
    // COPY CONSTRUCTOR - Deep copy (creates new Disaster object)
    DisasterReport(const DisasterReport& o) 
        : id(o.id), location(o.location), reporter(o.reporter), 
          disaster(cloneDisaster(o.disaster)), priorityScore(o.priorityScore),
          verified(o.verified), status(o.status), resolved(o.resolved), dispatchTime(o.dispatchTime) {}
    
    // MOVE CONSTRUCTOR - Steals resources from temporary object (no copying)
    DisasterReport(DisasterReport&& o) noexcept 
        : id(o.id), location(o.location), reporter(o.reporter), disaster(o.disaster),
          priorityScore(o.priorityScore), verified(o.verified), status(o.status),
          resolved(o.resolved), dispatchTime(o.dispatchTime) { 
        o.disaster = nullptr;  // Original no longer owns the disaster
    }
    
    // ASSIGNMENT OPERATORS 
    DisasterReport& operator=(DisasterReport&& o) noexcept {
        if (this != &o) {
            delete disaster;  // Clean up current disaster
            id = o.id; location = o.location; reporter = o.reporter;
            disaster = o.disaster; priorityScore = o.priorityScore; verified = o.verified;
            status = o.status; resolved = o.resolved; dispatchTime = o.dispatchTime;
            o.disaster = nullptr;
        }
        return *this;
    }
    
    DisasterReport& operator=(const DisasterReport& o) {
        if (this != &o) {
            delete disaster;
            id = o.id; location = o.location; reporter = o.reporter;
            disaster = cloneDisaster(o.disaster); priorityScore = o.priorityScore;
            verified = o.verified; status = o.status; resolved = o.resolved; dispatchTime = o.dispatchTime;
        }
        return *this;
    }
    
    // Destructor - cleans up disaster pointer to prevent memory leak
    ~DisasterReport() { delete disaster; }
    
    // GETTERS 
    int getId() const { return id; }
    int getPriorityScore() const { return priorityScore; }
    bool isVerified() const { return verified; }
    string getStatus() const { return status; }
    Location getLocation() const { return location; }
    Disaster* getDisaster() const { return disaster; }
    Reporter getReporter() const { return reporter; }
    bool isResolved() const { return resolved; }
    time_t getDispatchTime() const { return dispatchTime; }
    
    // SETTERS 
    void setPriorityScore(int s) { priorityScore = s; }
    void setVerified(bool v) { verified = v; }
    void setStatus(string s) { status = s; }
    void setResolved(bool r) { resolved = r; }
    void setDispatchTime(time_t t) { dispatchTime = t; }
    
    // Calculates priority score based on severity, weight, and reporter type
    void calculatePriority() {
        if (!verified) { priorityScore = 0; return; }
        StandardPriority strategy;  // Using StandardPriority (can swap to UrgentPriority later)
        priorityScore = strategy.calculate(disaster->getSeverity(), disaster->getWeight(), 
                                           reporter.getType() == "Officer");
    }
    
    // Pretty-prints all report information
    void display() const {
        cout << "\n========================================\n";
        cout << "Report ID: " << id << " | Status: " << status << " | Resolved: " 
             << (resolved ? "YES" : "NO") << " | Priority: " << priorityScore << "\n";
        cout << "Disaster: " << disaster->getTypeName() << " (Severity: " << disaster->getSeverity() << "/10)\n";
        cout << "Location: "; location.display(); cout << "\n";
        cout << "Reporter: "; reporter.display(); cout << "\n";
        cout << "Description: " << disaster->getDescription() << "\n";
        cout << "Instructions: " << disaster->getInstructions() << "\n";
        cout << "========================================\n";
    }
    
    // Saves report to file with pipe '|' separators
    string toFileString() const {
        return to_string(id) + "|" + location.getCity() + "|" + location.getArea() + "|" + location.getLandmark() + "|" +
               reporter.toFileString() + "|" + disaster->getTypeName() + "|" + to_string(disaster->getSeverity()) + "|" +
               disaster->getDescription() + "|" + (verified ? "1" : "0") + "|" + status + "|" +
               to_string(priorityScore) + "|" + (resolved ? "1" : "0");
    }
    
    static void setNextId(int n) { nextId = n; }
    static int getNextId() { return nextId; }
};

// Initialize static member - reports start at ID 1001
int DisasterReport::nextId = 1001;

// SECTION 14: VERIFIER CLASS (Checks if reports are real or fake)

// Uses DEPENDENCY on DisasterReport (methods take DisasterReport as parameter)
class Verifier {
    static const string fakeWords[15];  // List of words that indicate fake reports
    static string toLower(string s) { transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }

   public:
    static bool verify(DisasterReport& report) {
        // Check if description contains any fake words
        string desc = toLower(report.getDisaster()->getDescription());
        for (int i = 0; i < 15; i++)
            if (desc.find(fakeWords[i]) != string::npos) {  // string::npos = "not found"
                report.setVerified(false); 
                report.setStatus("REJECTED");
                cout << "\n[VERIFIER] Report REJECTED - Fake/invalid content detected.\n";
                return false;
            }
        
        // Check if reporter has valid CNIC and phone
        if (!report.getReporter().isVerified()) {
            report.setVerified(false); 
            report.setStatus("REJECTED");
            cout << "\n[VERIFIER] Report REJECTED - Invalid CNIC or Phone.\n";
            return false;
        }
        
        // All checks passed!
        report.setVerified(true);
        report.setStatus("VERIFIED");
        report.calculatePriority();
        cout << "\n[VERIFIER] Report VERIFIED! Priority Score: " << report.getPriorityScore() << "\n";
        return true;
    }
};

// List of words that trigger automatic rejection (prevents fake/prank reports)
const string Verifier::fakeWords[15] = {"prank","fake","joke","hoax","test","lol","trolling",
                                        "false","made up","not real","pretend","drama","acting",
                                        "fiction","dummy"};

// SECTION 15: VOLUNTEER SYSTEM (Template Method Pattern + Inheritance)

// ABSTRACT BASE CLASS - Template Method Pattern
class Volunteer {
   protected:
    string name, id, cnic, phone, type;
    bool available;
    
    // Hook methods - can be overridden by derived classes
    virtual void preDeployCheck() { cout << "Checking equipment for " << name << "...\n"; }
    virtual void performDeploy() = 0;  // Pure virtual - must override

   public:
    Volunteer() : name(""), id(""), cnic(""), phone(""), type(""), available(true) {}
    Volunteer(string n, string i, string c, string p, string t) 
        : name(n), id(i), cnic(c), phone(p), type(t), available(true) {}
    
    // TEMPLATE METHOD - Defines the skeleton of the deployment algorithm
    virtual void deploy() final {  // 'final' means derived classes cannot override this
        if (!available) { 
            cout << name << " is already deployed.\n"; 
            return; 
        }
        preDeployCheck();    // Step 1: Prepare (can vary by type)
        performDeploy();     // Step 2: Execute (MUST be defined by each type)
        available = false;   // Step 3: Mark as deployed (common for all)
    }
    
    void setAvailable(bool a) { available = a; }
    bool isAvailable() const { return available; }
    string getName() const { return name; }
    string getId() const { return id; }
    string getCNIC() const { return cnic; }
    string getPhone() const { return phone; }
    string getType() const { return type; }
    
    virtual void display() const { 
        cout << name << " (ID: " << id << ", CNIC: " << cnic << ") - " 
             << (available ? "Available" : "Deployed"); 
    }
    
    virtual string toFileString() const { 
        return name + "|" + id + "|" + cnic + "|" + phone + "|" + type + "|" + (available ? "1" : "0"); 
    }
    
    virtual ~Volunteer() {}  // Virtual destructor for proper cleanup
};

// Medical Volunteer - Specializes in healthcare
class MedicalVolunteer : public Volunteer {
    string specialization;   // Doctor, Nurse, Paramedic, Surgeon, etc.
    
   protected:
    void preDeployCheck() override { 
        cout << "Preparing medical kit for " << name << " (" << specialization << ")...\n"; 
    }
    void performDeploy() override { 
        cout << "DEPLOYING Medical Volunteer: " << name << " (" << specialization << ")\n"; 
    }
    
   public:
    MedicalVolunteer(string n, string i, string c, string p, string spec) 
        : Volunteer(n, i, c, p, "Medical"), specialization(spec) {}
    
    void display() const override { 
        Volunteer::display(); 
        cout << " | Medical | " << specialization; 
    }
    
    string toFileString() const override { 
        return Volunteer::toFileString() + "|" + specialization; 
    }
};

// Tech Volunteer - Specializes in rescue operations, driving, heavy machinery
class TechVolunteer : public Volunteer {
    string skill;   // Driver, Technician, Rescue, Commander, etc.
    
   protected:
    void preDeployCheck() override { 
        cout << "Preparing field equipment for " << name << " (" << skill << ")...\n"; 
    }
    void performDeploy() override { 
        cout << "DEPLOYING Tech Volunteer: " << name << " (" << skill << ")\n"; 
    }
    
   public:
    TechVolunteer(string n, string i, string c, string p, string sk) 
        : Volunteer(n, i, c, p, "Tech"), skill(sk) {}
    
    void display() const override { 
        Volunteer::display(); 
        cout << " | Tech | " << skill; 
    }
    
    string toFileString() const override { 
        return Volunteer::toFileString() + "|" + skill; 
    }
};

// Global counter for generating unique volunteer IDs
int lastVolunteerId = 15;  // Starting at 15 because we have 15 pre-loaded volunteers

string generateVolunteerId() {
    lastVolunteerId++;
    stringstream ss;
    ss << "GOV" << setw(3) << setfill('0') << lastVolunteerId;  // Format: GOV016, GOV017, etc.
    return ss.str();
}

// SECTION 16: RELIEF CAMP CLASS

// Stores food and medicine stock for a specific city
class ReliefCamp {
    string name, city;
    int foodStock, medicineStock;
    
   public:
    ReliefCamp() : name(""), city(""), foodStock(0), medicineStock(0) {}
    ReliefCamp(string n, string c) : name(n), city(c), foodStock(0), medicineStock(0) {}
    
    // Add or remove items (negative quantity removes)
    void addFood(int q) { foodStock = max(0, foodStock + q); }      // Never go below 0
    void addMedicine(int q) { medicineStock = max(0, medicineStock + q); }
    
    int getFoodStock() const { return foodStock; }
    int getMedicineStock() const { return medicineStock; }
    string getName() const { return name; }
    string getCity() const { return city; }
    
    void display() const { 
        cout << name << " (" << city << ") | Food: " << foodStock << " | Medicine: " << medicineStock << endl; 
    }
    
    string toFileString() const { 
        return name + "|" + city + "|" + to_string(foodStock) + "|" + to_string(medicineStock); 
    }
    
    // Load from pipe-separated string
    void fromFileString(const string& line) {
        stringstream ss(line);
        string food, med;
        getline(ss, name, '|');
        getline(ss, city, '|');
        getline(ss, food, '|');
        getline(ss, med, '|');
        foodStock = stoi(food);
        medicineStock = stoi(med);
    }
};


// SECTION 17: DONATION SYSTEM

// STATIC class - all methods and data belong to the class itself, not objects
class DonationManager {
    static double totalFund;   // One copy shared across entire program
    
   public:
    static void addFunds(double amount, string donor, string cnic) {
        if (amount <= 0) { cout << "Invalid donation amount.\n"; return; }
        totalFund += amount;
        cout << "Donation of Rs. " << amount << " received from " << donor 
             << " (CNIC: " << cnic << "). Total: " << formatCurrency(totalFund) << endl;
    }
    
    static double getFunds() { return totalFund; }
    
    static void allocateFunds(double amount, string purpose) {
        if (amount <= totalFund) { 
            totalFund -= amount; 
            cout << "Allocated " << formatCurrency(amount) << " for: " << purpose 
                 << ". Remaining: " << formatCurrency(totalFund) << endl; 
        } else {
            cout << "Insufficient funds for " << purpose << "!\n";
        }
    }
    
    static void setInitialFunds(double a) { totalFund = a; }
    static void displayFunds() { cout << formatCurrency(totalFund) << endl; }
};

// Initialize static member
double DonationManager::totalFund = 0;

// Handles physical item donations (food, medicine)
class ItemDonation {
    string donor, itemType, city;
    int quantity;
    
   public:
    ItemDonation() : donor(""), itemType(""), quantity(0), city("") {}
    ItemDonation(string d, string it, int q, string c) : donor(d), itemType(it), quantity(q), city(c) {}
    
    void process(vector<ReliefCamp>& camps) {
        bool found = false;
        
        // Try to find a camp in the target city
        for (auto& camp : camps) {
            if (camp.getCity() == city) {
                string it = itemType;
                transform(it.begin(), it.end(), it.begin(), ::tolower);
                if (it == "food") camp.addFood(quantity);
                else if (it == "medicine") camp.addMedicine(quantity);
                found = true;
                cout << "Donation from " << donor << " delivered to " << camp.getName() << endl;
                break;
            }
        }
        
        // If no camp in that city, send to central warehouse
        if (!found) {
            cout << "No relief camp in " << city << ". Donation sent to CENTRAL WAREHOUSE.\n";
            for (auto& camp : camps) {
                if (camp.getCity() == "Central") {
                    string it = itemType;
                    transform(it.begin(), it.end(), it.begin(), ::tolower);
                    if (it == "food") camp.addFood(quantity);
                    else if (it == "medicine") camp.addMedicine(quantity);
                    cout << "Donation stored in CENTRAL WAREHOUSE.\n";
                    return;
                }
            }
        }
    }
};

// SECTION 18: OBSERVER PATTERN (For emergency alerts)

// OBSERVER INTERFACE - All observers must implement onAlert()
class AlertObserver {
   public:
    virtual void onAlert(const string& area, const string& message) = 0;
    virtual ~AlertObserver() {}
};

// CONCRETE OBSERVER 1: NDMA (National Disaster Management Authority)
class NDMAObserver : public AlertObserver {
   public:
    void onAlert(const string& area, const string& msg) override {
        cout << "[NDMA notified] Area: " << area << " | " << msg << endl;
    }
};

// CONCRETE OBSERVER 2: Rescue 1122 (Emergency rescue service)
class RescueObserver : public AlertObserver {
   public:
    void onAlert(const string& area, const string& msg) override {
        cout << "[Rescue 1122 notified] Area: " << area << " | " << msg << endl;
    }
};

// CONCRETE OBSERVER 3: PDMA (Provincial Disaster Management Authority)
class PDMAObserver : public AlertObserver {
   public:
    void onAlert(const string& area, const string& msg) override {
        cout << "[PDMA notified] Area: " << area << " | " << msg << endl;
    }
};

// SECTION 19: ALERT SYSTEM (Subject in Observer Pattern)

class AlertSystem {
    static vector<AlertObserver*> observers;   // List of all subscribers
    
   public:
    static void addObserver(AlertObserver* obs) { observers.push_back(obs); }
    
    // Send alert to a specific area (notifies all observers)
    static void sendAlert(const string& area, const string& message) {
        cout << "\n[ALERT to " << area << "] " << message << endl;
        for (auto* obs : observers) obs->onAlert(area, message);
        printBeep();  // Emergency sound!
    }
    
    // Broadcast to ALL areas
    static void broadcast(const string& message) {
        cout << "\n[BROADCAST to ALL] " << message << endl;
        for (auto* obs : observers) obs->onAlert("ALL", message);
        printBeep();
    }
    
    // SPECIAL: Mass citizen evacuation broadcast (SMS, Radio, TV, Sirens)
    static void broadcastEvacuation(const string& city, const string& area,
                                    const string& disasterType, int severity) {
        cout << "\n";
        cout << "##############################################################\n";
        cout << "#          !!! EMERGENCY EVACUATION ALERT !!!                #\n";
        cout << "#          SENT TO ALL CITIZENS IN AFFECTED AREA             #\n";
        cout << "##############################################################\n";
        cout << "#                                                            #\n";
        cout << "#  [SMS]  Sent to ALL registered phone numbers in:          #\n";
        cout << "#         " << city << " - " << area << "\n";
        cout << "#                                                            #\n";
        cout << "#  [RADIO] Emergency broadcast activated on FM/AM channels  #\n";
        cout << "#  [TV]    Breaking news alert sent to all news channels     #\n";
        cout << "#  [SIREN] Area sirens activated                             #\n";
        cout << "#                                                            #\n";
        cout << "#  MESSAGE TO CITIZENS:                                      #\n";
        cout << "#  -------------------------------------------------------  #\n";
        cout << "#  DEAR CITIZEN OF " << area << ",\n";
        cout << "#                                                            #\n";
        cout << "#  A " << disasterType << " of SEVERITY " << severity << "/10 has been reported        #\n";
        cout << "#  in your area. IMMEDIATE EVACUATION IS REQUIRED.          #\n";
        cout << "#                                                            #\n";
        cout << "#  >> LEAVE YOUR HOMES IMMEDIATELY <<                       #\n";
        cout << "#  >> MOVE TO NEAREST RELIEF CAMP IN " << city << " <<\n";
        cout << "#  >> TAKE ONLY ESSENTIAL ITEMS <<                          #\n";
        cout << "#  >> DO NOT USE ELEVATORS <<                               #\n";
        cout << "#  >> HELP ELDERLY AND CHILDREN FIRST <<                    #\n";
        cout << "#                                                            #\n";
        cout << "#  Emergency Helpline: 1122  |  NDMA: 1131                  #\n";
        cout << "#                                                            #\n";
        cout << "##############################################################\n";
        
        // Also notify all agencies
        for (auto* obs : observers)
            obs->onAlert(city + " - " + area,
                         "EVACUATION ORDER ISSUED for " + disasterType +
                         " (Severity " + to_string(severity) + ")");
        printBeep();
    }
};

// Initialize static member
vector<AlertObserver*> AlertSystem::observers;

// SECTION 20: EVACUATION PERSISTENCE (Save/Load to file)

void saveEvacuations() {
    ofstream f(EVACUATIONS_FILE);
    for (auto& e : allEvacuations) {
        f << e.area << "|" << e.city << "|" << e.disasterType << "|"
          << e.severity << "|" << e.issuedAt << "|" << (e.lifted ? "1" : "0") << "\n";
    }
    f.close();
}

void loadEvacuations() {
    ifstream f(EVACUATIONS_FILE);
    if (!f) return;  // File doesn't exist - if for first run
    
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        EvacuationRecord e;
        string sev, issued, lifted;
        getline(ss, e.area, '|');
        getline(ss, e.city, '|');
        getline(ss, e.disasterType, '|');
        getline(ss, sev, '|');
        getline(ss, issued, '|');
        getline(ss, lifted);
        e.severity = stoi(sev);
        e.issuedAt = (time_t)stoll(issued);  // Convert string to time_t
        e.lifted = (lifted == "1");
        allEvacuations.push_back(e);
    }
    f.close();
}

// SECTION 21: EMERGENCY NUMBERS DISPLAY

void showEmergencyNumbers() {
    clearScreen();
    cout << "\n==================================================\n";
    cout << "           EMERGENCY CONTACTS - PAKISTAN           \n";
    cout << "==================================================\n";
    cout << "\n  RESCUE SERVICES:\n";
    cout << "     * Rescue 1122               - 1122\n";
    cout << "     * Pakistan Army Rescue      - 1125\n";
    cout << "\n  MEDICAL EMERGENCIES:\n";
    cout << "     * Edhi Ambulance            - 115\n";
    cout << "     * Chhipa Ambulance          - 1020\n";
    cout << "     * Pakistan Red Crescent     - 111\n";
    cout << "\n  FIRE EMERGENCY:\n";
    cout << "     * Fire Brigade              - 16\n";
    cout << "\n  POLICE:\n";
    cout << "     * Police Emergency          - 15\n";
    cout << "\n  NDMA (National Disaster Management):\n";
    cout << "     * NDMA Helpline             - 1131\n";
    cout << "\n==================================================\n";
    waitForEnter();
}

// SECTION 22: CNIC COOLDOWN SYSTEM (Prevents report spam)

struct CNICRecord {
    string cnic, city, areaKey, disasterType;
    time_t lastSameAreaReport;                      // Same location, same disaster
    time_t lastSameAreaDifferentDisasterReport;     // Same location, different disaster (suspicious!)
    time_t lastAnyReport;                           // Any report from anywhere
    bool resolved;
};
vector<CNICRecord> cnicHistory;

// Checks if a person can submit a new report based on cooldown rules
bool canSubmitReport(const string& cnic, const string& city, const string& area,
                     const string& disasterType, const string& reporterType) {
    time_t now = time(0);
    string currentAreaKey = city + "|" + area;
    
    for (auto& record : cnicHistory) {
        if (record.cnic != cnic) continue;  // Not this person, skip
        
        // RULE 1: Same area, same disaster - must wait 24 hours
        if (record.city == city && record.areaKey == currentAreaKey && record.disasterType == disasterType) {
            double hoursGap = difftime(now, record.lastSameAreaReport) / 3600.0;
            if (hoursGap < 24) {
                int hoursLeft = (int)(24 - hoursGap);
                int minsLeft = (int)((24 - hoursGap) * 60) % 60;
                cout << "\n[BLOCKED] Help is already on the way to your area.\n";
                cout << "   Next update allowed after " << hoursLeft << " hours " << minsLeft << " minutes.\n";
                cout << "   For urgent help call: 1122\n";
                return false;
            }
        }
        
        // RULE 2: Same area, DIFFERENT disaster - SUSPICIOUS! Always block
        if (record.city == city && record.areaKey == currentAreaKey && record.disasterType != disasterType) {
            cout << "\n[BLOCKED] Suspicious activity detected!\n";
            cout << "   Cannot report different disaster from same location.\n";
            cout << "   Please call 1122 or NDMA: 1131 directly.\n";
            return false;
        }
        
        // RULE 3: Different city - must wait 30 days
        if (record.city != city) {
            double daysGap = difftime(now, record.lastAnyReport) / (24 * 3600.0);
            if (daysGap < 30) {
                int daysLeft = (int)(30 - daysGap);
                cout << "\n[BLOCKED] Report already submitted with this CNIC.\n";
                cout << "   Next report allowed after " << daysLeft << " days.\n";
                cout << "   Call 1122 or NDMA: 1131 for emergencies.\n";
                return false;
            }
        }
    }
    return true;  // All checks passed
}

// Records a successful report submission for cooldown tracking
void logCNICSubmission(const string& cnic, const string& city, const string& area, const string& disasterType) {
    time_t now = time(0);
    string areaKey = city + "|" + area;
    
    // Find existing record for this CNIC
    for (auto& record : cnicHistory) {
        if (record.cnic != cnic) continue;
        
        record.lastAnyReport = now;
        record.city = city;
        
        if (record.areaKey == areaKey) {
            if (record.disasterType == disasterType) 
                record.lastSameAreaReport = now;
            else { 
                record.lastSameAreaDifferentDisasterReport = now; 
                record.disasterType = disasterType; 
            }
        } else { 
            record.areaKey = areaKey; 
            record.disasterType = disasterType; 
            record.lastSameAreaReport = now; 
        }
        record.resolved = false;
        return;
    }
    
    // No existing record - create new one
    CNICRecord r;
    r.cnic = cnic; r.city = city; r.areaKey = areaKey; r.disasterType = disasterType;
    r.lastSameAreaReport = r.lastSameAreaDifferentDisasterReport = r.lastAnyReport = now;
    r.resolved = false;
    cnicHistory.push_back(r);
}

void saveCNICHistory() {
    ofstream f("cnic_history.txt");
    for (auto& r : cnicHistory) {
        f << r.cnic << "|" << r.city << "|" << r.areaKey << "|" << r.disasterType << "|"
          << r.lastSameAreaReport << "|" << r.lastSameAreaDifferentDisasterReport << "|"
          << r.lastAnyReport << "|" << (r.resolved ? "1" : "0") << "\n";
    }
    f.close();
}

void loadCNICHistory() {
    ifstream f("cnic_history.txt");
    if (!f) return;
    
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        CNICRecord r;
        string t1, t2, t3, res;
        getline(ss, r.cnic, '|'); getline(ss, r.city, '|'); getline(ss, r.areaKey, '|');
        getline(ss, r.disasterType, '|'); getline(ss, t1, '|'); getline(ss, t2, '|');
        getline(ss, t3, '|'); getline(ss, res);
        r.lastSameAreaReport = (time_t)stoll(t1);
        r.lastSameAreaDifferentDisasterReport = (time_t)stoll(t2);
        r.lastAnyReport = (time_t)stoll(t3);
        r.resolved = (res == "1");
        cnicHistory.push_back(r);
    }
    f.close();
}


// SECTION 23: ADMIN DASHBOARD (All admin functionality)

// AGGREGATION: Contains POINTERS to global vectors (does NOT own the data)
class AdminDashboard {
    vector<DisasterReport>* reports;
    vector<unique_ptr<Volunteer>>* volunteers;
    vector<ReliefCamp>* camps;

   public:
    AdminDashboard(vector<DisasterReport>* r, vector<unique_ptr<Volunteer>>* v, vector<ReliefCamp>* c)
        : reports(r), volunteers(v), camps(c) {}

    void showStats() {
        cout << "\n========== ADMIN DASHBOARD ==========\n";
        int verified = 0, pending = 0, resolved = 0;
        for (auto& r : *reports) {
            if (r.isVerified()) verified++;
            else if (r.getStatus() == "PENDING") pending++;
            if (r.isResolved()) resolved++;
        }
        cout << "Total Reports: " << reports->size() << endl;
        cout << "Verified: " << verified << endl;
        cout << "Resolved: " << resolved << endl;
        cout << "Relief Fund: " << formatCurrency(DonationManager::getFunds()) << endl;
        
        int avail = 0;
        for (auto& v : *volunteers) if (v->isAvailable()) avail++;
        cout << "Volunteers: " << volunteers->size() << " (" << avail << " available)" << endl;
        cout << "Relief Camps: " << camps->size() << endl;
        
        int totalFood = 0, totalMedicine = 0;
        for (auto& c : *camps) { totalFood += c.getFoodStock(); totalMedicine += c.getMedicineStock(); }
        cout << "Total Food Stock: " << totalFood << " units" << endl;
        cout << "Total Medicine Stock: " << totalMedicine << " units" << endl;
        
        int activeEvac = 0;
        for (auto& e : allEvacuations) if (!e.lifted) activeEvac++;
        cout << "Active Evacuations: " << activeEvac << endl;
        cout << "=====================================\n";
    }

    void showAllReports() {
        if (reports->empty()) { cout << "\nNo reports found.\n"; return; }
        cout << "\n========== ALL DISASTER REPORTS ==========\n";
        for (auto& r : *reports) r.display();
    }

    void showAllResources() {
        cout << "\n========== RESOURCE PORTAL ==========\n";
        cout << "Relief Fund: " << formatCurrency(DonationManager::getFunds()) << endl;
        cout << "\nVOLUNTEERS (" << volunteers->size() << " registered):\n";
        if (volunteers->empty()) cout << "   No volunteers yet.\n";
        else for (auto& v : *volunteers) { cout << "   - "; v->display(); cout << endl; }
        
        cout << "\nRELIEF CAMPS (" << camps->size() << " operational):\n";
        for (auto& c : *camps) { cout << "   - "; c.display(); }
        
        cout << "\nCENTRAL WAREHOUSE:\n";
        for (auto& c : *camps) if (c.getCity() == "Central") c.display();
        cout << "=====================================\n";
    }

    void imposeLockdown(const string& area) {
        AlertSystem::sendAlert(area, "LOCKDOWN imposed. Stay indoors.");
        AlertSystem::broadcast("Lockdown announced for " + area);
        DonationManager::allocateFunds(100000, "Lockdown support for " + area);
    }

    void orderEvacuation(const string& city, const string& area,
                         const string& disasterType, int severity) {
        // Check if evacuation already active for this area
        for (auto& e : allEvacuations) {
            if (e.city == city && e.area == area && !e.lifted) {
                cout << "\n[INFO] Evacuation already ACTIVE for " << area << ", " << city << ".\n";
                cout << "       Use 'Lift Evacuation' to cancel it first.\n";
                return;
            }
        }

        // Record the evacuation
        EvacuationRecord rec;
        rec.area = area; rec.city = city; rec.disasterType = disasterType;
        rec.severity = severity; rec.issuedAt = time(0); rec.lifted = false;
        allEvacuations.push_back(rec);

        // Mass citizen broadcast
        AlertSystem::broadcastEvacuation(city, area, disasterType, severity);
        AlertSystem::sendAlert(city + " - " + area, "EVACUATION ORDER issued by Admin.");
        AlertSystem::broadcast("EVACUATION in progress: " + area + ", " + city);
        DonationManager::allocateFunds(200000, "Evacuation logistics for " + area);
        
        cout << "\n[EVACUATION] Order recorded and all citizens notified.\n";
        saveEvacuations();
    }

    void liftEvacuation() {
        bool found = false;
        cout << "\n========== ACTIVE EVACUATION ORDERS ==========\n";
        for (int i = 0; i < (int)allEvacuations.size(); i++) {
            auto& e = allEvacuations[i];
            if (!e.lifted) {
                cout << i + 1 << ". " << e.area << ", " << e.city
                     << " [" << e.disasterType << " | Severity: " << e.severity << "]\n";
                found = true;
            }
        }
        if (!found) { cout << "No active evacuation orders.\n"; return; }
        
        int idx = getValidInt("Select evacuation to lift (number): ", 1, (int)allEvacuations.size());
        allEvacuations[idx - 1].lifted = true;
        
        string area = allEvacuations[idx - 1].area;
        string city = allEvacuations[idx - 1].city;
        AlertSystem::sendAlert(city + " - " + area, "EVACUATION ORDER LIFTED. Area is now safe.");
        AlertSystem::broadcast("Evacuation lifted for " + area + ", " + city + ". Citizens may return.");
        cout << "\n[EVACUATION LIFTED] Citizens notified it is safe to return.\n";
        saveEvacuations();
    }

    void viewEvacuations() {
        cout << "\n========== EVACUATION ORDERS LOG ==========\n";
        if (allEvacuations.empty()) { cout << "No evacuation orders issued yet.\n"; return; }
        for (auto& e : allEvacuations) {
            char buf[32];
            tm* t = localtime(&e.issuedAt);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", t);
            cout << "Area: " << e.area << ", " << e.city
                 << " | Disaster: " << e.disasterType
                 << " | Severity: " << e.severity
                 << " | Issued: " << buf
                 << " | Status: " << (e.lifted ? "LIFTED" : "ACTIVE") << "\n";
        }
        cout << "============================================\n";
    }

    void deployResources() {
        for (auto& r : *reports) {
            if (r.isVerified() && !r.isResolved() && r.getPriorityScore() > 0) {
                cout << "\nDEPLOYING RESOURCES for: "; r.getLocation().display(); cout << endl;
                string city = r.getLocation().getCity();
                int sev = r.getDisaster()->getSeverity();
                int foodNeeded = sev * FOOD_PER_SEVERITY;
                int medNeeded = sev * MEDICINE_PER_SEVERITY;
                
                bool resourceFound = false;
                // Try local camp first
                for (auto& camp : *camps) {
                    if (camp.getCity() == city) {
                        int foodSent = min(foodNeeded, camp.getFoodStock());
                        int medSent = min(medNeeded, camp.getMedicineStock());
                        camp.addFood(-foodSent);
                        camp.addMedicine(-medSent);
                        cout << "   Food sent: " << foodSent << " | Medicine sent: " << medSent << endl;
                        resourceFound = true;
                        break;
                    }
                }
                // If no local camp, use central warehouse
                if (!resourceFound) {
                    for (auto& camp : *camps) {
                        if (camp.getCity() == "Central") {
                            int foodSent = min(foodNeeded, camp.getFoodStock());
                            int medSent = min(medNeeded, camp.getMedicineStock());
                            camp.addFood(-foodSent);
                            camp.addMedicine(-medSent);
                            cout << "   Food sent from WAREHOUSE: " << foodSent << " | Medicine sent: " << medSent << endl;
                            break;
                        }
                    }
                }
                
                DonationManager::allocateFunds(50000, "Emergency response");
                for (auto& v : *volunteers) { if (v->isAvailable()) { v->deploy(); break; } }
                AlertSystem::broadcast("Resources deployed to " + r.getLocation().getCity());
                cout << "\n[CONFIRMATION] A confirmation message will be sent to you shortly.\n";
                return;
            }
        }
        cout << "No verified high-priority disaster requiring resources.\n";
    }

    void addReliefCamp(const string& name, const string& city) {
        camps->push_back(ReliefCamp(name, city));
        cout << "New relief camp added: " << name << " in " << city << endl;
    }

    void markReportResolved(int id) {
        for (auto& r : *reports) {
            if (r.getId() == id) {
                r.setResolved(true);
                r.setStatus("RESOLVED");
                cout << "Report #" << id << " marked as RESOLVED.\n";
                return;
            }
        }
        cout << "Report not found.\n";
    }

    void followUpResources() {
        time_t now = time(0);
        bool anyFollowup = false;
        for (auto& report : *reports) {
            if (!report.isVerified() || report.isResolved() || report.getStatus() == "REJECTED") continue;
            double hoursSinceReport = difftime(now, report.getDisaster()->getReportTime()) / 3600.0;
            if (hoursSinceReport >= 0.0167) {  // About 1 minute after deployment
                cout << "\n========== FOLLOW-UP: RESOURCE CONFIRMATION ==========\n";
                cout << "Report ID: " << report.getId() << " | Location: ";
                report.getLocation().display();
                cout << "\nResources were dispatched " << (int)hoursSinceReport << " hours ago.\n";
                cout << "Have the resources arrived? (1=YES / 2=NO): ";
                int ans; cin >> ans; cin.ignore();
                if (ans < 1 || ans > 2) ans = 2;
                
                if (ans == 1) {
                    cout << "\n[RESOLVED] Report #" << report.getId() << " marked as RESOLVED.\n";
                    const_cast<DisasterReport&>(report).setResolved(true);
                    const_cast<DisasterReport&>(report).setStatus("RESOLVED");
                } else {
                    cout << "\n[ESCALATED] Resources not reached. Re-dispatching...\n";
                    AlertSystem::broadcast("URGENT: Report #" + to_string(report.getId()) + " - Resources not reached!");
                    deployResources();
                }
                anyFollowup = true;
                break;
            }
        }
        if (!anyFollowup) cout << "\nNo pending resources to follow up.\n";
    }
};

// SECTION 24: FILE PERSISTENCE (Save/Load all data)

void saveAllData(const vector<DisasterReport>& reports, const vector<unique_ptr<Volunteer>>& volunteers,
                 const vector<ReliefCamp>& camps, double funds) {
    ofstream reportFile(REPORTS_FILE);
    for (const auto& r : reports) reportFile << r.toFileString() << endl;
    reportFile.close();
    
    ofstream volFile(VOLUNTEERS_FILE);
    for (const auto& v : volunteers) volFile << v->toFileString() << endl;
    volFile.close();
    
    ofstream campFile(CAMPS_FILE);
    for (const auto& c : camps) campFile << c.toFileString() << endl;
    campFile.close();
    
    ofstream fundsFile(FUNDS_FILE);
    fundsFile << funds << endl;
    fundsFile.close();
}

void loadAllData(vector<DisasterReport>& reports, vector<unique_ptr<Volunteer>>& volunteers,
                 vector<ReliefCamp>& camps, double& funds) {
    // Load funds
    ifstream fundsFile(FUNDS_FILE);
    if (fundsFile) { fundsFile >> funds; DonationManager::setInitialFunds(funds); fundsFile.close(); }
    
    // Load disaster reports
    ifstream reportFile(REPORTS_FILE);
    if (reportFile) {
        string line; int maxId = 1000;
        while (getline(reportFile, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr,city,area,landmark,cnic,name,phone,type,officerCode,verifiedStr,
                   disasterType,severityStr,description,verStr,status,priorityStr,resolvedStr;
            getline(ss,idStr,'|'); getline(ss,city,'|'); getline(ss,area,'|'); getline(ss,landmark,'|');
            getline(ss,cnic,'|'); getline(ss,name,'|'); getline(ss,phone,'|'); getline(ss,type,'|');
            getline(ss,officerCode,'|'); getline(ss,verifiedStr,'|'); getline(ss,disasterType,'|');
            getline(ss,severityStr,'|'); getline(ss,description,'|'); getline(ss,verStr,'|');
            getline(ss,status,'|'); getline(ss,priorityStr,'|'); getline(ss,resolvedStr);
            
            int id = (idStr.empty()) ? 1001 : stoi(idStr);
            int severity = (severityStr.empty()) ? 1 : stoi(severityStr);
            maxId = max(maxId, id);
            
            Location loc(city, area, landmark);
            Reporter reporter(cnic, name, phone, type, officerCode);
            Disaster* disaster = createDisaster(disasterType, description, severity);
            if (disaster) {
                DisasterReport report(loc, reporter, disaster);
                report.setVerified(verStr == "1");
                report.setStatus(status);
                int priority = (priorityStr.empty()) ? 0 : stoi(priorityStr);
                report.setPriorityScore(priority);
                report.setResolved(resolvedStr == "1");
                reports.push_back(report);
            }
        }
        DisasterReport::setNextId(maxId + 1);
        reportFile.close();
    }
    
    // Load volunteers
    ifstream volFile(VOLUNTEERS_FILE);
    if (volFile) {
        string line;
        while (getline(volFile, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string vname,vid,vcnic,vphone,vtype,vavail,vextra;
            getline(ss,vname,'|'); getline(ss,vid,'|'); getline(ss,vcnic,'|');
            getline(ss,vphone,'|'); getline(ss,vtype,'|'); getline(ss,vavail,'|'); getline(ss,vextra);
            bool avail = (vavail == "1");
            
            if (vtype == "Medical") {
                auto v = make_unique<MedicalVolunteer>(vname,vid,vcnic,vphone,vextra);
                v->setAvailable(avail);
                volunteers.push_back(move(v));
            } else {
                auto v = make_unique<TechVolunteer>(vname,vid,vcnic,vphone,vextra);
                v->setAvailable(avail);
                volunteers.push_back(move(v));
            }
        }
        volFile.close();
    }
    
    // Load relief camps
    ifstream campFile(CAMPS_FILE);
    if (campFile) {
        string line;
        while (getline(campFile, line)) {
            if (line.empty()) continue;
            ReliefCamp camp;
            camp.fromFileString(line);
            camps.push_back(camp);
        }
        campFile.close();
    }
}

// SECTION 25: INPUT VALIDATION HELPERS

int getValidInt(const string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minVal && value <= maxVal) { 
            cin.ignore(); 
            return value; 
        }
        cout << "Invalid input. Please enter a number between " << minVal << " and " << maxVal << ".\n";
        cin.clear();  // Clear error flags
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Discard bad input
    }
}

double getValidDouble(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value > 0) { 
            cin.ignore(); 
            return value; 
        }
        cout << "Invalid amount. Please enter a positive number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string getValidOfficerCode() {
    string code;
    while (true) {
        cout << "Officer Code (3 digits): ";
        cin >> code;
        if (code.length() == 3) {
            bool allDigits = true;
            for (char c : code) if (!isdigit(c)) { allDigits = false; break; }
            if (allDigits) { cin.ignore(); return code; }
        }
        cout << "Invalid! Officer Code must be exactly 3 digits.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

bool validateCNICForVolunteer(const string& cn) {
    string c = "";
    for (char ch : cn) if (isdigit(ch)) c += ch;
    if (c.length() != 13 || c[0] == '0') return false;
    bool allSame = true;
    for (int i = 1; i < 13; i++) if (c[i] != c[0]) { allSame = false; break; }
    return !allSame;
}

// SECTION 26: ADMIN PASSWORD (Simple hash for security)

// djb2 hash algorithm - not cryptographic, just to avoid plain-text password
size_t simpleHash(const string& str) {
    size_t hash = 5381;
    for (char c : str) hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    return hash;
}

const size_t ADMIN_PASSWORD_HASH = simpleHash("admin123");  // Default password: admin123
bool checkAdminPassword(const string& input) { return simpleHash(input) == ADMIN_PASSWORD_HASH; }

// SECTION 27: INITIALIZE GOVERNMENT DATA (Pre-loaded volunteers and camps)

void initializeGovernmentData() {
    DonationManager::setInitialFunds(5000000);  // Rs. 5,000,000 starting fund
    
    // Create relief camps in major Pakistani cities
    allCamps.push_back(ReliefCamp("Central Camp Karachi",       "Karachi"));
    allCamps.push_back(ReliefCamp("Lahore Relief Hub",          "Lahore"));
    allCamps.push_back(ReliefCamp("Islamabad Emergency Center", "Islamabad"));
    allCamps.push_back(ReliefCamp("Peshawar Base Camp",         "Peshawar"));
    allCamps.push_back(ReliefCamp("Quetta Relief Camp",         "Quetta"));
    allCamps.push_back(ReliefCamp("Multan Central Camp",        "Multan"));
    allCamps.push_back(ReliefCamp("Gilgit Base Camp",           "Gilgit"));
    allCamps.push_back(ReliefCamp("Central Warehouse",          "Central"));
    
    // Give each camp initial supplies
    for (auto& camp : allCamps) { camp.addFood(5000); camp.addMedicine(2000); }
    
    // Pre-load 15 government volunteers
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Dr. Ahmed Khan",      "GOV001","4250106707876","03001234567","Doctor"));
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Dr. Fatima Ali",      "GOV002","4250106707877","03001234568","Doctor"));
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Nurse Sara",          "GOV003","4250106707878","03001234569","Nurse"));
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Dr. Usman Chaudhry", "GOV004","4250106707879","03001234572","Doctor"));
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Nurse Ayesha",        "GOV005","4250106707880","03001234573","Nurse"));
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Dr. Bilal Ahmed",     "GOV006","4250106707881","03001234574","Surgeon"));
    allVolunteers.push_back(make_unique<TechVolunteer>("Commander Raza",         "GOV007","4250106707882","03001234570","Rescue Specialist"));
    allVolunteers.push_back(make_unique<TechVolunteer>("Engineer Kashif",        "GOV008","4250106707883","03001234571","Heavy Machinery"));
    allVolunteers.push_back(make_unique<TechVolunteer>("Squad Leader Tariq",     "GOV009","4250106707884","03001234575","Rescue"));
    allVolunteers.push_back(make_unique<TechVolunteer>("Driver Zafar",           "GOV010","4250106707885","03001234576","Driver"));
    allVolunteers.push_back(make_unique<TechVolunteer>("Technician Ali",         "GOV011","4250106707886","03001234577","Technician"));
    allVolunteers.push_back(make_unique<TechVolunteer>("Logistics Incharge",     "GOV012","4250106707887","03001234578","Logistics"));
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Dr. Sana Mir",        "GOV013","4250106707888","03001234579","Pediatrician"));
    allVolunteers.push_back(make_unique<MedicalVolunteer>("Nurse Kiran",         "GOV014","4250106707889","03001234580","Nurse"));
    allVolunteers.push_back(make_unique<TechVolunteer>("Field Commander",        "GOV015","4250106707890","03001234581","Commander"));
}

// SECTION 28: PUBLIC FUNCTIONS (Main user interactions)

void submitReport() {
    clearScreen();
    string city, area, landmark, name, cnic, phone, reporterType, disasterType, description, officerCode = "";
    
    cout << "\n========== SUBMIT DISASTER REPORT ==========\n";
    cout << "City: "; getline(cin, city);
    cout << "Area/Sector: "; getline(cin, area);
    cout << "Nearby Landmark: "; getline(cin, landmark);
    Location loc(city, area, landmark);
    
    cout << "\n--- Your Information ---\n";
    cout << "Full Name: "; getline(cin, name);
    cout << "CNIC (13 digits without dashes): "; getline(cin, cnic);
    cout << "Phone (03xxxxxxxxx): "; getline(cin, phone);
    cout << "Type (Civilian/Officer): "; getline(cin, reporterType);
    
    string typeLower = reporterType;
    transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);
    if (typeLower == "officer") { 
        reporterType = "Officer"; 
        officerCode = getValidOfficerCode(); 
    } else {
        reporterType = "Civilian";
    }
    
    Reporter reporter(cnic, name, phone, reporterType, officerCode);
    if (!reporter.isVerified()) { cout << "\nVerification failed!\n"; waitForEnter(); return; }
    
    cout << "\n--- Disaster Details ---\n";
    cout << "Disaster Types: FLOOD, EARTHQUAKE, CYCLONE, LANDSLIDE, FIRE, DROUGHT, PANDEMIC\n";
    cout << "Disaster Type: "; getline(cin, disasterType);
    
    if (!canSubmitReport(cnic, city, area, disasterType, reporterType)) { waitForEnter(); return; }
    
    int severity = getValidInt("Severity (1-10): ", MIN_SEVERITY, MAX_SEVERITY);
    cout << "Description: "; getline(cin, description);
    
    Disaster* disaster = createDisaster(disasterType, description, severity);
    if (!disaster) { cout << "Invalid disaster type!\n"; waitForEnter(); return; }
    
    DisasterReport newReport(loc, reporter, disaster);
    Verifier::verify(newReport);
    
    if (newReport.isVerified()) {
        allReports.push_back(newReport);
        logCNICSubmission(cnic, city, area, disasterType);
        
        // Automatic evacuation if disaster type/severity requires it
        if (disaster->requiresEvacuation()) {
            cout << "\n[SYSTEM] Disaster severity requires IMMEDIATE EVACUATION.\n";
            AlertSystem::broadcastEvacuation(city, area, disaster->getTypeName(), severity);
            
            EvacuationRecord rec;
            rec.area = area; rec.city = city; rec.disasterType = disaster->getTypeName();
            rec.severity = severity; rec.issuedAt = time(0); rec.lifted = false;
            allEvacuations.push_back(rec);
        }
        
        clearScreen();
        cout << "\n+====================================================================+\n";
        cout << "|          REPORT SUBMITTED SUCCESSFULLY                             |\n";
        cout << "+====================================================================+\n";
        cout << "|  Report ID: " << newReport.getId() << "                                        |\n";
        cout << "|                                                                    |\n";
        cout << "|  Alert conveyed to:                                                |\n";
        cout << "|  * NDMA (National Disaster Management Authority)                   |\n";
        cout << "|  * PDMA (Provincial Disaster Management Authority)                 |\n";
        cout << "|  * Rescue 1122                                                     |\n";
        cout << "|  * Pakistan Army Emergency Response Unit                           |\n";
        cout << "|  * Local Administration                                            |\n";
        if (disaster->requiresEvacuation()) {
            cout << "|                                                                    |\n";
            cout << "|  !! EVACUATION ALERT SENT TO ALL CITIZENS IN YOUR AREA !!         |\n";
            cout << "|     SMS, Radio, TV & Siren alerts activated.                       |\n";
        }
        cout << "|                                                                    |\n";
        cout << "|  For updates, call: 1122                                           |\n";
        cout << "|  You will receive a confirmation call on your registered number.   |\n";
        cout << "|  [PHONE] Please keep your phone nearby.                            |\n";
        cout << "+====================================================================+\n";
    } else {
        delete disaster;
    }
    waitForEnter();
}

void registerVolunteer() {
    clearScreen();
    string name, cnic, phone, type, spec;
    
    cout << "\n========== VOLUNTEER REGISTRATION ==========\n";
    cout << "Full Name: "; getline(cin, name);
    cout << "CNIC (13 digits): "; getline(cin, cnic);
    if (!validateCNICForVolunteer(cnic)) { cout << "\n[ERROR] Invalid CNIC!\n"; waitForEnter(); return; }
    
    cout << "Phone Number: "; getline(cin, phone);
    cout << "Type (Medical/Tech): "; getline(cin, type);
    
    string typeLower = type;
    transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);
    string volunteerId = generateVolunteerId();
    
    if (typeLower == "medical") {
        cout << "Specialization (Doctor/Nurse/Paramedic): "; getline(cin, spec);
        allVolunteers.push_back(make_unique<MedicalVolunteer>(name, volunteerId, cnic, phone, spec));
    } else {
        cout << "Skill (Driver/Technician/Rescue): "; getline(cin, spec);
        allVolunteers.push_back(make_unique<TechVolunteer>(name, volunteerId, cnic, phone, spec));
    }
    
    clearScreen();
    cout << "\n+----------------------------------------------------------+\n";
    cout << "|           VOLUNTEER REGISTERED SUCCESSFULLY              |\n";
    cout << "+----------------------------------------------------------+\n";
    cout << "|  Welcome " << name << "!\n";
    cout << "|  Your Volunteer ID: " << volunteerId << "                |\n";
    cout << "|  You will be contacted during emergencies.               |\n";
    cout << "+----------------------------------------------------------+\n";
    waitForEnter();
}

void makeDonation() {
    clearScreen();
    string name, type, city, cnic;
    
    cout << "\n========== MAKE A DONATION ==========\n";
    cout << "Your Name: "; getline(cin, name);
    cout << "CNIC (13 digits): "; getline(cin, cnic);
    cout << "Type (Money/Item): "; getline(cin, type);
    
    string typeLower = type;
    transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);
    
    if (typeLower == "money") {
        double money = getValidDouble("Amount (Rs.): ");
        DonationManager::addFunds(money, name, cnic);
    } else {
        string itemType;
        cout << "Item Type (Food/Medicine): "; getline(cin, itemType);
        int qty = getValidInt("Quantity: ", 1, 1000000);
        cout << "Target City: "; getline(cin, city);
        ItemDonation newDonation(name, itemType, qty, city);
        newDonation.process(allCamps);
        cout << "\nThank you for your donation, " << name << "!\n";
    }
    waitForEnter();
}

// SECTION 29: MAIN FUNCTION (Program entry point)

int main() {
    clearScreen();
    double loadedFunds = 0;
    
    // Load all saved data from previous runs
    loadAllData(allReports, allVolunteers, allCamps, loadedFunds);
    loadCNICHistory();
    loadEvacuations();
    
    // Rebuild CNIC type map from loaded reports
    cnicTypeMap.clear();
    for (auto& r : allReports) {
        string c = r.getReporter().getCNIC();
        string t = r.getReporter().getType();
        if (!c.empty() && c != "INVALID") cnicTypeMap[c] = t;
    }
    
    // If first run,Initialize with default data
    if (allVolunteers.empty() && allCamps.empty()) initializeGovernmentData();
    
    // Language selection
    int langChoice = getValidInt("Select Language (1. English  2. Urdu  3. Sindhi  4. Pashto): ", 1, 4);
    SystemConfig::setLang(langChoice);
    clearScreen();
    SystemConfig::displayLanguageMessage();
    SystemConfig::printWelcome();
    cout << "\n";
    
    // Create admin dashboard
    AdminDashboard dashboard(&allReports, &allVolunteers, &allCamps);
    
    // Register alert observers
    static NDMAObserver ndmaObs;
    static RescueObserver rescueObs;
    static PDMAObserver pdmaObs;
    AlertSystem::addObserver(&ndmaObs);
    AlertSystem::addObserver(&rescueObs);
    AlertSystem::addObserver(&pdmaObs);
    
    int choice;
    do {
        SystemConfig::printMenu();
        choice = getValidInt("", 1, 6);
        
        switch (choice) {
            case 1:  // Submit Report
                clearScreen();
                submitReport();
                saveAllData(allReports, allVolunteers, allCamps, DonationManager::getFunds());
                saveCNICHistory();
                saveEvacuations();
                break;
                
            case 2:  // Register Volunteer
                clearScreen();
                registerVolunteer();
                saveAllData(allReports, allVolunteers, allCamps, DonationManager::getFunds());
                saveCNICHistory();
                break;
                
            case 3:  // Make Donation
                clearScreen();
                makeDonation();
                saveAllData(allReports, allVolunteers, allCamps, DonationManager::getFunds());
                saveCNICHistory();
                break;
                
            case 4:  // Emergency Numbers
                showEmergencyNumbers();
                break;
                
            case 5: {  // Admin Login
                clearScreen();
                string password;
                cout << "\nAdmin Password: ";
                cin >> password; cin.ignore();
                
                if (checkAdminPassword(password)) {
                    int adminChoice;
                    do {
                        clearScreen();
                        cout << "\n+================================+\n";
                        cout << "|        ADMIN PANEL             |\n";
                        cout << "+================================+\n";
                        cout << "| 1.  Dashboard Stats            |\n";
                        cout << "| 2.  View All Reports           |\n";
                        cout << "| 3.  View Resources             |\n";
                        cout << "| 4.  Impose Lockdown            |\n";
                        cout << "| 5.  Order Evacuation           |\n";
                        cout << "| 6.  Lift Evacuation            |\n";
                        cout << "| 7.  View Evacuations Log       |\n";
                        cout << "| 8.  Deploy Resources           |\n";
                        cout << "| 9.  Follow Up Resources        |\n";
                        cout << "| 10. Add Relief Camp            |\n";
                        cout << "| 11. Mark Report Resolved       |\n";
                        cout << "| 12. Save & Exit Admin          |\n";
                        cout << "+================================+\n";
                        adminChoice = getValidInt("Choice: ", 1, 12);
                        
                        if (adminChoice == 1) dashboard.showStats();
                        else if (adminChoice == 2) dashboard.showAllReports();
                        else if (adminChoice == 3) dashboard.showAllResources();
                        else if (adminChoice == 4) {
                            string area;
                            cout << "Area/City: "; getline(cin, area);
                            dashboard.imposeLockdown(area);
                        }
                        else if (adminChoice == 5) {
                            string city, area, dtype;
                            cout << "City: "; getline(cin, city);
                            cout << "Area/Sector: "; getline(cin, area);
                            cout << "Disaster Type: "; getline(cin, dtype);
                            int sev = getValidInt("Severity (1-10): ", 1, 10);
                            dashboard.orderEvacuation(city, area, dtype, sev);
                        }
                        else if (adminChoice == 6) dashboard.liftEvacuation();
                        else if (adminChoice == 7) dashboard.viewEvacuations();
                        else if (adminChoice == 8) dashboard.deployResources();
                        else if (adminChoice == 9) dashboard.followUpResources();
                        else if (adminChoice == 10) {
                            string name, city;
                            cout << "Camp Name: "; getline(cin, name);
                            cout << "City: "; getline(cin, city);
                            dashboard.addReliefCamp(name, city);
                        }
                        else if (adminChoice == 11) {
                            int rid = getValidInt("Report ID: ", 1000, 9999);
                            dashboard.markReportResolved(rid);
                        }
                        else if (adminChoice == 12) {
                            saveAllData(allReports, allVolunteers, allCamps, DonationManager::getFunds());
                            saveCNICHistory();
                            saveEvacuations();
                            cout << "Exiting Admin Panel...\n";
                            waitForEnter();
                        }
                        
                        if (adminChoice != 12) { cout << "\n"; waitForEnter(); }
                    } while (adminChoice != 12);
                } else {
                    cout << "Wrong password!\n";
                    waitForEnter();
                }
                break;
            }
            
            case 6:  // Exit
                saveAllData(allReports, allVolunteers, allCamps, DonationManager::getFunds());
                saveCNICHistory();
                saveEvacuations();
                clearScreen();
                cout << "\n+----------------------------------------------------------+\n";
                cout << "|         DISASTER RELIEF SYSTEM SHUTTING DOWN             |\n";
                cout << "+----------------------------------------------------------+\n";
                cout << "|  All data has been saved successfully.                   |\n";
                cout << "|                                                          |\n";
                cout << "|  Stay safe! Pakistan Zindabad!                           |\n";
                cout << "+----------------------------------------------------------+\n";
                break;
        }
    } while (choice != 6);
    
    return 0;
}