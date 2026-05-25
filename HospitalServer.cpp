/**
 * HospitalServer.cpp - Server Multithreaded (Windows/Winsock2)
 * Kompilasi: g++ -o server.exe HospitalServer.cpp -lws2_32 -std=c++17 -pthread
 *
 * FITUR:
 * - Socket Programming Winsock2 (Client-Server)
 * - BONUS: Multithreading - tiap client di thread terpisah
 * - BONUS: Mutex sinkronisasi shared data
 * - JSON manual untuk pertukaran data
 */

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include "Patient.h"
#include "ManualLinkedList.h"

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT        8080
#define BUFFER_SIZE 8192

// ============================================================
// Shared state (dilindungi mutex - BONUS multithreading)
// ============================================================
mutex queueMutex;
mutex clientsMutex;
ManualLinkedList<Patient*> patientQueue;
vector<SOCKET> clients;
int idCounter = 1000;
int arrivalCounter = 0;

// ============================================================
// Helper: Ekstrak nilai dari JSON string secara manual
// ============================================================
string jsonGet(const string& js, const string& key) {
    string k = "\"" + key + "\"";
    size_t p = js.find(k);
    if (p == string::npos) return "";
    p = js.find(':', p) + 1;
    while (p < js.size() && js[p] == ' ') p++;
    if (js[p] == '"') {
        p++;
        size_t e = js.find('"', p);
        return js.substr(p, e - p);
    }
    size_t e = p;
    while (e < js.size() && (isdigit(js[e]) || js[e] == '-')) e++;
    return js.substr(p, e - p);
}

// ============================================================
// Broadcast antrian ke semua client yang terhubung
// Kompleksitas: O(n * c) → n pasien, c client
// ============================================================
void broadcastQueue() {
    string data;
    {
        lock_guard<mutex> lk(queueMutex);
        data = "{\"status\":\"QUEUE_UPDATE\",\"total\":"
             + to_string(patientQueue.getSize())
             + ",\"data\":" + patientQueue.toJsonArray() + "}\n";
    }
    cout << "\n[JSON DIKIRIM KE CLIENT]\n";
    cout << data << endl;
    lock_guard<mutex> lk(clientsMutex);
    for (SOCKET s : clients)
        send(s, data.c_str(), (int)data.size(), 0);
}

// ============================================================
// Handler: Tambah pasien
// ============================================================
void doAddPatient(SOCKET sock, const string& req) {
    string type = jsonGet(req, "type");
    string name = jsonGet(req, "name");
    if (name.empty()) {
        string e = "{\"status\":\"ERROR\",\"message\":\"Nama kosong\"}\n";
        send(sock, e.c_str(), (int)e.size(), 0); return;
    }
    Patient* p = nullptr;
    int newId;
    {
        lock_guard<mutex> lk(queueMutex);
        newId = ++idCounter;
        if (type == "Emergency") {
            int lvl = jsonGet(req, "injury_level").empty() ? 3
                      : stoi(jsonGet(req, "injury_level"));
            lvl = max(1, min(5, lvl));
            p = new EmergencyPatient(name, newId, lvl);
        } else {
            p = new RegularPatient(name, newId, ++arrivalCounter);
        }
        patientQueue.add(p);
    }
    cout << "[+] Tambah: " << name << " ID=" << newId
              << " Prioritas=" << p->getPriorityScore() << "\n";

    string ok = "{\"status\":\"SUCCESS\",\"message\":\"Pasien " + name
               + " didaftarkan\",\"patient_id\":" + to_string(newId) + "}\n";
    send(sock, ok.c_str(), (int)ok.size(), 0);
    broadcastQueue();
}

// ============================================================
// Handler: Panggil pasien berikutnya (prioritas tertinggi)
// ============================================================
void doCallNext(SOCKET sock) {
    Patient* p = nullptr;
    {
        lock_guard<mutex> lk(queueMutex);
        p = patientQueue.popFront(); // O(1)
    }
    string resp;
    if (p) {
        cout << "[~] Panggil: " << p->getName() << " ID=" << p->getPatientId() << "\n";
        resp = "{\"status\":\"CALLED\",\"patient\":" + p->toJson() + "}\n";
        delete p;
    } else {
        resp = "{\"status\":\"EMPTY\",\"message\":\"Antrean kosong\"}\n";
    }
    send(sock, resp.c_str(), (int)resp.size(), 0);
    broadcastQueue();
}

// ============================================================
// Handler: Cari berdasarkan nama - Linear Search O(n)
// ============================================================
void doSearchName(SOCKET sock, const string& req) {
    string name = jsonGet(req, "name");
    lock_guard<mutex> lk(queueMutex);
    Patient* found = patientQueue.linearSearchByName(name); // O(n)
    string resp;
    if (found)
        resp = "{\"status\":\"FOUND\",\"method\":\"Linear Search O(n)\","
               "\"patient\":" + found->toJson() + "}\n";
    else
        resp = "{\"status\":\"NOT_FOUND\",\"message\":\"'" + name + "' tidak ditemukan\"}\n";
    send(sock, resp.c_str(), (int)resp.size(), 0);
}

// ============================================================
// Handler: Cari berdasarkan ID - Binary Search O(log n)
// ============================================================
void doSearchId(SOCKET sock, const string& req) {
    string idStr = jsonGet(req, "id");
    if (idStr.empty()) {
        string e = "{\"status\":\"ERROR\",\"message\":\"ID tidak valid\"}\n";
        send(sock, e.c_str(), (int)e.size(), 0); return;
    }
    lock_guard<mutex> lk(queueMutex);
    Patient* found = patientQueue.binarySearchById(stoi(idStr)); // O(n log n)
    string resp;
    if (found)
        resp = "{\"status\":\"FOUND\",\"method\":\"Binary Search O(log n)\"," "\"patient\":" + found->toJson() + "}\n";
    else
        resp = "{\"status\":\"NOT_FOUND\",\"message\":\"ID " + idStr + " tidak ada\"}\n";
    send(sock, resp.c_str(), (int)resp.size(), 0);
}

// ============================================================
// Thread per client (BONUS: Multithreading)
// ============================================================
void handleClient(SOCKET sock) {
    char buf[BUFFER_SIZE];
    string leftover;

    while (true) {
        memset(buf, 0, BUFFER_SIZE);
        int n = recv(sock, buf, BUFFER_SIZE - 1, 0);
        if (n <= 0) { cout << "[-] Client disconnect\n"; break; }

        leftover += string(buf, n);
        size_t pos;
        while ((pos = leftover.find('\n')) != string::npos) {
            string msg = leftover.substr(0, pos);
            leftover.erase(0, pos + 1);
            if (msg.empty()) continue;

            string action = jsonGet(msg, "action");
            if (action == "ADD_PATIENT") doAddPatient(sock, msg);
            else if (action == "CALL_NEXT") doCallNext(sock);
            else if (action == "GET_QUEUE"){
                string q;
                {
                    lock_guard<mutex> lk(queueMutex);
                    q = "{\"status\":\"QUEUE_UPDATE\",\"total\":"
                      + to_string(patientQueue.getSize())
                      + ",\"data\":" + patientQueue.toJsonArray() + "}\n";
                }
                send(sock, q.c_str(), (int)q.size(), 0);
            }
            else if (action == "SEARCH_NAME") doSearchName(sock, msg);
            else if (action == "SEARCH_ID")   doSearchId(sock, msg);
        }
    }

    { lock_guard<mutex> lk(clientsMutex);
      clients.erase(remove(clients.begin(), clients.end(), sock), clients.end()); }
    closesocket(sock);
}

// ============================================================
// Main
// ============================================================
int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cerr << "WSAStartup gagal\n"; return 1;
    }

    SOCKET srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srv == INVALID_SOCKET) {
        cerr << "Socket gagal\n"; WSACleanup(); return 1;
    }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "Bind gagal: " << WSAGetLastError() << "\n";
        closesocket(srv); WSACleanup(); return 1;
    }
    listen(srv, 10);

    cout << "================================================\n"
              << "  HOSPITAL QUEUE SYSTEM - SERVER\n"
              << "  Port: " << PORT << " | Menunggu client...\n"
              << "================================================\n";

    while (true) {
        sockaddr_in cAddr; int cLen = sizeof(cAddr);
        SOCKET cSock = accept(srv, (sockaddr*)&cAddr, &cLen);
        if (cSock == INVALID_SOCKET) continue;

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cAddr.sin_addr, ip, INET_ADDRSTRLEN);
        cout << "[+] Client terhubung: " << ip << "\n";

        { lock_guard<mutex> lk(clientsMutex); clients.push_back(cSock); }

        // Kirim state antrian saat ini ke client baru
        string q;
        { lock_guard<mutex> lk(queueMutex);
          q = "{\"status\":\"QUEUE_UPDATE\",\"total\":" + to_string(patientQueue.getSize()) + ",\"data\":" + patientQueue.toJsonArray() + "}\n"; }
        send(cSock, q.c_str(), (int)q.size(), 0);

        // BONUS: Thread baru per client dengan sinkronisasi mutex
        thread(handleClient, cSock).detach();
    }

    closesocket(srv);
    WSACleanup();
    return 0;
}
