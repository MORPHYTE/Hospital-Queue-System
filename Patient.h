#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <iostream>
using namespace std;

class Patient {
protected:
    string name;
    int patientId;
    int priorityScore;
    string patientType; 

public:
    Patient(const string& name, int id, const string& type)
        : name(name), patientId(id), priorityScore(0), patientType(type) {}

    virtual ~Patient() {}

    virtual void calculatePriority() = 0;  
    virtual void displayInfo() = 0;  
    virtual string toJson() = 0;  

    int getPriorityScore() const { return priorityScore; }
    int getPatientId() const { return patientId; }
    string getName() const { return name; }
    string getPatientType() const { return patientType; }
};

class EmergencyPatient : public Patient {
private:
    int injuryLevel; 
public:
    EmergencyPatient(const string& name, int id, int injuryLevel) : Patient(name, id, "Emergency"), injuryLevel(injuryLevel) {
        calculatePriority(); 
    }

    void calculatePriority() override {
        priorityScore = 1000 + (injuryLevel * 100);
    }

    void displayInfo() override {
        cout << "[DARURAT] ID: " << patientId << " | Nama: " << name << " | Level Cedera: " << injuryLevel << "/5 | Prioritas: "  << priorityScore << "\n";
    }

    string toJson() override {
        return "{\"id\":" + to_string(patientId) + ",\"name\":\"" + name + "\"" + ",\"type\":\"Emergency\"" + ",\"injury_level\":" + to_string(injuryLevel) + ",\"priority_score\":" + to_string(priorityScore) + "}";
    }

    int getInjuryLevel() const { return injuryLevel; }
};

class RegularPatient : public Patient {
private:
    int arrivalOrder; 

public:
    RegularPatient(const string& name, int id, int arrivalOrder) : Patient(name, id, "Regular"), arrivalOrder(arrivalOrder) {
        calculatePriority();
    }
    void calculatePriority() override {
        priorityScore = 500 - arrivalOrder;
    }

    void displayInfo() override {
        cout << "[REGULER] ID: " << patientId << " | Nama: "      << name << " | No. Antrian: " << arrivalOrder << " | Prioritas: "   << priorityScore << "\n";
    }

    string toJson() override {
        return "{\"id\":" + to_string(patientId) + ",\"name\":\"" + name + "\"" + ",\"type\":\"Regular\"" + ",\"arrival_order\":" + to_string(arrivalOrder) + ",\"priority_score\":" + to_string(priorityScore) + "}";
    }

    int getArrivalOrder() const { return arrivalOrder; }
};

#endif 
