# Hospital Queue System — Ringkasan Proyek Final

## Struktur File

```
HospitalQueueSystem/
├── Patient.h              ← OOP: Abstract class + subclass
├── ManualLinkedList.h     ← Struktur data + algoritma manual
├── HospitalServer.cpp     ← Server multithreaded (Winsock2)
├── HospitalClient.cpp     ← Client interaktif (Winsock2)
└── Makefile               ← Kompilasi MinGW
```

## Cara Download & Setup

### 1. Install MinGW (compiler C++ untuk Windows)
Download dari: https://winlibs.com/ atau https://www.mingw-w64.org/
- Pilih versi: Win64, POSIX threads, UCRT runtime
- Tambahkan `C:\mingw64\bin` ke PATH Windows

### 2. Download nlohmann/json.hpp (TIDAK diperlukan)
> Project ini menggunakan parsing JSON **manual** (tanpa library eksternal)

### 3. Kompilasi
Buka **cmd** di folder `HospitalQueueSystem/`:
```cmd
mingw32-make
```
Atau kompilasi manual:
```cmd
g++ -o server.exe HospitalServer.cpp -lws2_32 -std=c++17 -pthread
g++ -o client.exe HospitalClient.cpp -lws2_32 -std=c++17 -pthread
```

### 4. Jalankan
- Terminal 1: `server.exe`
- Terminal 2: `client.exe`
- Terminal 3 (opsional): `client.exe` (multi-client)

---

## Pemenuhan Syarat Wajib

| Syarat | Implementasi | File |
|--------|-------------|------|
| **Struktur Data Dinamis** | Manual Linked List (`Node<T>`) tanpa std::list/vector | `ManualLinkedList.h` |
| **Sorting Manual** | **Merge Sort** – O(n log n), diimplementasikan dari nol | `ManualLinkedList.h` |
| **Searching Manual** | **Linear Search** by nama – O(n) | `ManualLinkedList.h` |
| **Searching Manual** | **Binary Search** by ID – O(log n) | `ManualLinkedList.h` |
| **OOP – Abstraksi** | `Patient` adalah pure abstract class | `Patient.h` |
| **OOP – Enkapsulasi** | Atribut `protected`/`private`, getter publik | `Patient.h` |
| **OOP – Pewarisan** | `EmergencyPatient`, `RegularPatient` ← `Patient` | `Patient.h` |
| **OOP – Polimorfisme** | `calculatePriority()`, `displayInfo()`, `toJson()` di-override | `Patient.h` |
| **Networking** | TCP Socket Programming via Winsock2, Client-Server | `Server.cpp`, `Client.cpp` |
| **JSON** | Serialisasi/deserialisasi data manual (format JSON) | Semua file |

## Pemenuhan Bonus (+10%)

| Bonus | Implementasi |
|-------|-------------|
| ✅ **Manual Linked List** | `ManualLinkedList<T>` tanpa `std::list`/`std::vector` |
| ✅ **Analisis Big O** | Komentar di setiap fungsi algoritma |
| ✅ **Multithreading** | `std::thread` per client + `std::mutex` sinkronisasi |

---

## Analisis Big O (Ringkasan)

| Operasi | Kompleksitas Waktu | Kompleksitas Ruang |
|---------|-------------------|-------------------|
| `add()` (insert + sort) | O(n log n) | O(log n) |
| `popFront()` | O(1) | O(1) |
| `linearSearchByName()` | O(n) | O(1) |
| `binarySearchById()` | O(n log n) | O(n) |
| `mergeSort()` | O(n log n) | O(log n) |
| `toJsonArray()` | O(n) | O(n) |
| `removeById()` | O(n) | O(1) |

---

## Arsitektur Sistem

```
┌──────────────────────────┐         JSON via TCP Socket
│  CLIENT (client.exe)      │ ──────────────────────────► ┌──────────────────────────┐
│                           │                              │  SERVER (server.exe)      │
│  Menu:                    │ ◄────────────────────────── │                           │
│  1. Daftar Reguler        │      JSON Response          │  ManualLinkedList         │
│  2. Daftar Darurat        │                              │  (Merge Sort auto-sort)   │
│  3. Panggil Pasien        │                              │                           │
│  4. Lihat Antrian         │                              │  Thread per Client        │
│  5. Cari by Nama          │                              │  (Mutex-protected)        │
│  6. Cari by ID            │                              │                           │
└──────────────────────────┘                              └──────────────────────────┘
         CLIENT 2 ─────────────────────────────────────────────────────────────────┘
         CLIENT 3 ─────────────────────────────────────────────────────────────────┘
```

## Hierarki OOP

```
Patient  (Abstract Class — ABSTRAKSI)
│  + name, patientId, priorityScore  (protected — ENKAPSULASI)
│  + getName(), getId(), getScore()  (getter publik — ENKAPSULASI)
│  + calculatePriority() = 0         (pure virtual — ABSTRAKSI)
│  + displayInfo() = 0               (pure virtual — ABSTRAKSI)
│  + toJson() = 0                    (pure virtual — ABSTRAKSI)
│
├── EmergencyPatient  (PEWARISAN)
│       + injuryLevel                (private)
│       + calculatePriority()        ← override (POLIMORFISME)
│       + displayInfo()              ← override (POLIMORFISME)
│       + toJson()                   ← override (POLIMORFISME)
│       Skor = 1000 + injuryLevel*100  (min 1100, selalu prioritas)
│
└── RegularPatient  (PEWARISAN)
        + arrivalOrder               (private)
        + calculatePriority()        ← override (POLIMORFISME)
        + displayInfo()              ← override (POLIMORFISME)
        + toJson()                   ← override (POLIMORFISME)
        Skor = 500 - arrivalOrder    (max 499, setelah Emergency)
```

## Contoh Alur Penggunaan

1. Jalankan `server.exe` → server mendengarkan port 8080
2. Buka 3 terminal, masing-masing jalankan `client.exe`
3. Terminal 1 (Loket): daftarkan pasien reguler dan darurat
4. Terminal 2 (Dokter): panggil pasien selanjutnya
5. Terminal 3 (Monitor): lihat antrian real-time (update otomatis)
