/**
 * =============================================================================
 * Patient.h - Hierarki Kelas Pasien
 * =============================================================================
 * PILAR OOP YANG DIIMPLEMENTASIKAN:
 * 1. Abstraksi    : Patient adalah pure abstract class (ada pure virtual func)
 * 2. Enkapsulasi  : Atribut protected/private, diakses via getter/setter publik
 * 3. Pewarisan    : EmergencyPatient & RegularPatient mewarisi Patient
 * 4. Polimorfisme : calculatePriority(), displayInfo(), toJson() di-override
 * =============================================================================
 */

#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <iostream>
using namespace std;

// ============================================================
// PILAR 1 - ABSTRAKSI: Abstract Base Class
// Tidak bisa di-instantiate langsung karena ada pure virtual function
// ============================================================
class Patient {
protected:
    // PILAR 2 - ENKAPSULASI: Atribut disembunyikan, hanya bisa diakses
    // oleh subclass (protected) atau via getter publik
    string name;
    int patientId;
    int priorityScore;
    string patientType; // "Emergency" atau "Regular"

public:
    Patient(const string& name, int id, const string& type)
        : name(name), patientId(id), priorityScore(0), patientType(type) {}

    virtual ~Patient() {}

    // Pure virtual functions → menjadikan Patient sebagai abstract class
    virtual void calculatePriority() = 0;  // PILAR 1: Abstraksi
    virtual void displayInfo() = 0;  // PILAR 4: Polimorfisme
    virtual string toJson() = 0;  // PILAR 4: Polimorfisme

    // PILAR 2 - ENKAPSULASI: Getter publik (read-only access)
    int getPriorityScore() const { return priorityScore; }
    int getPatientId() const { return patientId; }
    string getName() const { return name; }
    string getPatientType() const { return patientType; }
};

// ============================================================
// PILAR 3 - PEWARISAN & PILAR 4 - POLIMORFISME:
// EmergencyPatient mewarisi Patient, override semua pure virtual
// ============================================================
class EmergencyPatient : public Patient {
private:
    // PILAR 2 - ENKAPSULASI: atribut spesifik private
    int injuryLevel; // Skala 1-5 (5 = paling kritis)

public:
    EmergencyPatient(const string& name, int id, int injuryLevel) : Patient(name, id, "Emergency"), injuryLevel(injuryLevel) {
        calculatePriority(); // Hitung skor saat konstruksi
    }

    /**
     * PILAR 4 - POLIMORFISME: Override calculatePriority()
     * Rumus: 1000 + (injuryLevel * 100)
     * Skor UGD (min 1100) selalu lebih tinggi dari Reguler (max 499)
     * Kompleksitas Waktu: O(1) | Kompleksitas Ruang: O(1)
     */
    void calculatePriority() override {
        priorityScore = 1000 + (injuryLevel * 100);
    }

    /**
     * PILAR 4 - POLIMORFISME: Override displayInfo()
     * Kompleksitas Waktu: O(1) | Kompleksitas Ruang: O(1)
     */
    void displayInfo() override {
        cout << "[DARURAT] ID: " << patientId << " | Nama: " << name << " | Level Cedera: " << injuryLevel << "/5 | Prioritas: "  << priorityScore << "\n";
    }

    /**
     * PILAR 4 - POLIMORFISME: Override toJson()
     * Serialisasi manual ke format JSON string
     * Kompleksitas Waktu: O(1) | Kompleksitas Ruang: O(1)
     */
    string toJson() override {
        return "{\"id\":" + to_string(patientId) + ",\"name\":\"" + name + "\"" + ",\"type\":\"Emergency\"" + ",\"injury_level\":" + to_string(injuryLevel) + ",\"priority_score\":" + to_string(priorityScore) + "}";
    }

    // Getter spesifik EmergencyPatient
    int getInjuryLevel() const { return injuryLevel; }
};

// ============================================================
// PILAR 3 - PEWARISAN & PILAR 4 - POLIMORFISME:
// RegularPatient mewarisi Patient, override semua pure virtual
// ============================================================
class RegularPatient : public Patient {
private:
    // PILAR 2 - ENKAPSULASI: atribut spesifik private
    int arrivalOrder; // Urutan kedatangan (auto-increment dari server)

public:
    RegularPatient(const string& name, int id, int arrivalOrder) : Patient(name, id, "Regular"), arrivalOrder(arrivalOrder) {
        calculatePriority();
    }

    /**
     * PILAR 4 - POLIMORFISME: Override calculatePriority()
     * Rumus: 500 - arrivalOrder
     * Semakin awal datang (arrivalOrder kecil) → skor lebih tinggi
     * Kompleksitas Waktu: O(1) | Kompleksitas Ruang: O(1)
     */
    void calculatePriority() override {
        priorityScore = 500 - arrivalOrder;
    }

    /**
     * PILAR 4 - POLIMORFISME: Override displayInfo()
     * Kompleksitas Waktu: O(1) | Kompleksitas Ruang: O(1)
     */
    void displayInfo() override {
        cout << "[REGULER] ID: " << patientId << " | Nama: "      << name << " | No. Antrian: " << arrivalOrder << " | Prioritas: "   << priorityScore << "\n";
    }

    /**
     * PILAR 4 - POLIMORFISME: Override toJson()
     * Serialisasi manual ke format JSON string
     * Kompleksitas Waktu: O(1) | Kompleksitas Ruang: O(1)
     */
    string toJson() override {
        return "{\"id\":" + to_string(patientId) + ",\"name\":\"" + name + "\"" + ",\"type\":\"Regular\"" + ",\"arrival_order\":" + to_string(arrivalOrder) + ",\"priority_score\":" + to_string(priorityScore) + "}";
    }

    int getArrivalOrder() const { return arrivalOrder; }
};

#endif // PATIENT_H
