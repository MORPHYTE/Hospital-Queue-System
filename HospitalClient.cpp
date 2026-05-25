/**
 * HospitalClient.cpp - Client Interaktif (Windows/Winsock2)
 * Kompilasi: g++ -o client.exe HospitalClient.cpp -lws2_32 -std=c++17 -pthread
 */

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <sstream>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 8192

using namespace std; // Global namespace directive

atomic<bool> running(true);

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
// Helper: Tampilkan antrian dari JSON array
// ============================================================
void printQueue(const string& msg) {
    string total = jsonGet(msg, "total");

    cout << "\n+------------------------------------------------+\n";
    cout << "|          DAFTAR ANTRIAN PASIEN                 |\n";
    cout << "+------------------------------------------------+\n";
    cout << "| Total pasien: " << total << "\n";
    cout << "+------------------------------------------------+\n";

    // Parse array JSON data pasien secara manual
    size_t arrStart = msg.find("\"data\":[");
    if (arrStart == string::npos || msg.find("[]") != string::npos) {
        if (total == "0" || total.empty())
            cout << "| [Antrean kosong]\n";
    } else {
        arrStart = msg.find('[', arrStart + 7);
        size_t arrEnd = msg.rfind(']');
        string arr = msg.substr(arrStart + 1, arrEnd - arrStart - 1);

        int no = 1;
        size_t pos = 0;
        while (pos < arr.size()) {
            size_t objStart = arr.find('{', pos);
            if (objStart == string::npos) break;
            // Cari penutup objek (sederhana, tidak nested)
            size_t objEnd = arr.find('}', objStart);
            string obj = arr.substr(objStart, objEnd - objStart + 1);

            string name  = jsonGet(obj, "name");
            string type  = jsonGet(obj, "type");
            string id    = jsonGet(obj, "id");
            string score = jsonGet(obj, "priority_score");

            string label = (type == "Emergency") ? "[DARURAT]" : "[REGULER]";
            cout << "| " << no++ << ". " << label << " " << name
                      << " | ID:" << id << " | Skor:" << score << "\n";
            pos = objEnd + 1;
        }
    }
    cout << "+------------------------------------------------+\n";
}

// ============================================================
// Thread: Terima pesan dari server secara real-time
// ============================================================
void receiveLoop(SOCKET sock) {
    char buf[BUFFER_SIZE];
    string leftover;

    while (running) {
        memset(buf, 0, BUFFER_SIZE);
        int n = recv(sock, buf, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            if (running) cout << "\n[!] Koneksi ke server terputus.\n";
            running = false;
            break;
        }

        leftover += string(buf, n);
        size_t pos;
        while ((pos = leftover.find('\n')) != string::npos) {
            string msg = leftover.substr(0, pos);
            leftover.erase(0, pos + 1);
            if (msg.empty()) continue;

            string status = jsonGet(msg, "status");

            if (status == "QUEUE_UPDATE") {
                printQueue(msg);
                cout << "\nPilih menu: " << flush;

            } else if (status == "SUCCESS") {
                string message = jsonGet(msg, "message");
                string pid     = jsonGet(msg, "patient_id");
                cout << "\n[OK] " << message;
                if (!pid.empty()) cout << " (ID Pasien: " << pid << ")";
                cout << "\n";

            } else if (status == "CALLED") {
                // Dokter memanggil pasien
                size_t pStart = msg.find("\"patient\":{");
                string patObj = "";
                if (pStart != string::npos) {
                    pStart += 10;
                    size_t pEnd = msg.find('}', pStart);
                    patObj = msg.substr(pStart, pEnd - pStart + 1);
                }
                cout << "\n+------------------------------------------------+\n";
                cout << "|        DOKTER MEMANGGIL PASIEN                 |\n";
                cout << "+------------------------------------------------+\n";
                cout << "| Nama   : " << jsonGet(patObj, "name") << "\n";
                cout << "| ID     : " << jsonGet(patObj, "id") << "\n";
                cout << "| Tipe   : " << jsonGet(patObj, "type") << "\n";
                cout << "| Prioritas: " << jsonGet(patObj, "priority_score") << "\n";
                cout << "+------------------------------------------------+\n";
                cout << "\nPilih menu: " << flush;

            } else if (status == "FOUND") {
                size_t pStart = msg.find("\"patient\":{");
                string patObj = "";
                if (pStart != string::npos) {
                    pStart += 10;
                    size_t pEnd = msg.find('}', pStart);
                    patObj = msg.substr(pStart, pEnd - pStart + 1);
                }
                string method = jsonGet(msg, "method");
                cout << "\n[DITEMUKAN] Metode: " << method << "\n";
                cout << "  Nama  : " << jsonGet(patObj, "name") << "\n";
                cout << "  ID    : " << jsonGet(patObj, "id") << "\n";
                cout << "  Tipe  : " << jsonGet(patObj, "type") << "\n";
                cout << "  Skor  : " << jsonGet(patObj, "priority_score") << "\n";
                cout << "\nPilih menu: " << flush;

            } else if (status == "NOT_FOUND") {
                cout << "\n[!] " << jsonGet(msg, "message") << "\n";
                cout << "\nPilih menu: " << flush;

            } else if (status == "EMPTY") {
                cout << "\n[!] " << jsonGet(msg, "message") << "\n";
                cout << "\nPilih menu: " << flush;

            } else if (status == "ERROR") {
                cout << "\n[ERROR] " << jsonGet(msg, "message") << "\n";
                cout << "\nPilih menu: " << flush;
            }
        }
    }
}

// ============================================================
// Helper: Kirim JSON ke server dengan terminator '\n'
// ============================================================
void sendJson(SOCKET sock, const string& json) {
    string msg = json + "\n";
    send(sock, msg.c_str(), (int)msg.size(), 0);
}

// ============================================================
// Main: Menu interaktif
// ============================================================
int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cerr << "WSAStartup gagal\n"; return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket gagal\n"; WSACleanup(); return 1;
    }

    sockaddr_in sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &sAddr.sin_addr);

    cout << "Menghubungkan ke server " << SERVER_IP << ":" << PORT << "...\n";
    if (connect(sock, (sockaddr*)&sAddr, sizeof(sAddr)) == SOCKET_ERROR) {
        cerr << "[ERROR] Gagal terhubung ke server. Pastikan server sudah berjalan!\n";
        closesocket(sock); WSACleanup(); return 1;
    }
    cout << "[OK] Terhubung ke Hospital Queue Server!\n\n";

    // Mulai thread penerima pesan
    thread recvThread(receiveLoop, sock);
    recvThread.detach();

    // --------------------------------------------------------
    // Loop menu interaktif
    // --------------------------------------------------------
    while (running) {
        cout << "\n================================================\n";
        cout << "     DIGITAL HOSPITAL QUEUE SYSTEM\n";
        cout << "================================================\n";
        cout << "  1. [Loket] Daftarkan Pasien Reguler\n";
        cout << "  2. [Loket] Daftarkan Pasien Gawat Darurat\n";
        cout << "  3. [Dokter] Panggil Pasien Berikutnya\n";
        cout << "  4. [Monitor] Tampilkan Antrian\n";
        cout << "  5. [Cari] Cari Pasien by Nama (Linear Search)\n";
        cout << "  6. [Cari] Cari Pasien by ID (Binary Search)\n";
        cout << "  0. Keluar\n";
        cout << "================================================\n";
        cout << "Pilih menu: ";

        int choice;
        if (!(cin >> choice)) { running = false; break; }

        if (choice == 0) { running = false; break; }

        else if (choice == 1) {
            string name;
            cout << "Nama pasien: ";
            cin.ignore();
            getline(cin, name);
            sendJson(sock, "{\"action\":\"ADD_PATIENT\",\"type\":\"Regular\",\"name\":\"" + name + "\"}");
            Sleep(300);
        }

        else if (choice == 2) {
            string name;
            int lvl;
            cout << "Nama pasien: ";
            cin.ignore();
            getline(cin, name);
            cout << "Level cedera (1=ringan, 5=kritis): ";
            cin >> lvl;
            sendJson(sock, "{\"action\":\"ADD_PATIENT\",\"type\":\"Emergency\","
                           "\"name\":\"" + name + "\","
                           "\"injury_level\":" + to_string(lvl) + "}");
            Sleep(300);
        }

        else if (choice == 3) {
            sendJson(sock, "{\"action\":\"CALL_NEXT\"}");
            Sleep(300);
        }

        else if (choice == 4) {
            sendJson(sock, "{\"action\":\"GET_QUEUE\"}");
            Sleep(300);
        }

        else if (choice == 5) {
            string name;
            cout << "Masukkan nama (atau sebagian nama): ";
            cin.ignore();
            getline(cin, name);
            sendJson(sock, "{\"action\":\"SEARCH_NAME\",\"name\":\"" + name + "\"}");
            Sleep(300);
        }

        else if (choice == 6) {
            int id;
            cout << "Masukkan ID pasien: ";
            cin >> id;
            sendJson(sock, "{\"action\":\"SEARCH_ID\",\"id\":" + to_string(id) + "}");
            Sleep(300);
        }

        else {
            cout << "[!] Menu tidak valid.\n";
        }
    }

    closesocket(sock);
    WSACleanup();
    cout << "Sampai jumpa!\n";
    return 0;
}
