#ifndef MANUALLINKEDLIST_H
#define MANUALLINKEDLIST_H

#include "Patient.h"
#include <stdexcept>
using namespace std;

// ============================================================
// Node: Unit dasar dari Linked List
// ============================================================
template <typename T>
struct Node {
    T data;
    Node* next;
    explicit Node(T d) : data(d), next(nullptr) {}
};

// ============================================================
// ManualLinkedList: Singly Linked List tanpa library apapun
// BONUS: Implementasi Linked List secara manual
// ============================================================
template <typename T>
class ManualLinkedList {
private:
    Node<T>* head;
    int size;

    // ----------------------------------------------------------
    // MERGE SORT - Helper: Gabung dua sublist terurut (descending)
    // Kompleksitas: O(n) untuk setiap level merge
    // ----------------------------------------------------------
    Node<T>* merge(Node<T>* left, Node<T>* right) {
        if (!left)  return right;
        if (!right) return left;

        // Urutkan descending berdasarkan priorityScore
        // EmergencyPatient (skor ~1100-1500) selalu duluan dari Regular (~499--)
        if (left->data->getPriorityScore() >= right->data->getPriorityScore()) {
            left->next = merge(left->next, right);
            return left;
        } else {
            right->next = merge(left, right->next);
            return right;
        }
    }

    // ----------------------------------------------------------
    // MERGE SORT - Helper: Belah list menjadi dua bagian
    // Menggunakan teknik fast/slow pointer (Floyd's algorithm)
    // Kompleksitas: O(n)
    // ----------------------------------------------------------
    void splitList(Node<T>* source, Node<T>** front, Node<T>** back) {
        Node<T>* slow = source;
        Node<T>* fast = source->next;

        // fast bergerak 2x lebih cepat dari slow
        // Ketika fast sampai akhir, slow ada di tengah
        while (fast != nullptr) {
            fast = fast->next;
            if (fast != nullptr) {
                slow = slow->next;
                fast = fast->next;
            }
        }

        *front = source;
        *back  = slow->next;
        slow->next = nullptr; // Putus koneksi di tengah
    }

    // ----------------------------------------------------------
    // MERGE SORT - Fungsi Rekursif Utama
    // Algoritma Sorting Manual (tidak menggunakan fungsi bawaan library)
    //
    // Big O Analysis:
    //   - Best Case    : O(n log n) → selalu divide & conquer
    //   - Average Case : O(n log n)
    //   - Worst Case   : O(n log n)
    //   - Space        : O(log n)   → rekursi stack
    // ----------------------------------------------------------
    void mergeSortHelper(Node<T>** headRef) {
        Node<T>* h = *headRef;
        if (!h || !h->next) return; // Base case: 0 atau 1 elemen

        Node<T>* a;
        Node<T>* b;
        splitList(h, &a, &b); // Bagi dua

        mergeSortHelper(&a);  // Rekursi kiri
        mergeSortHelper(&b);  // Rekursi kanan

        *headRef = merge(a, b); // Gabung hasil
    }

public:
    ManualLinkedList() : head(nullptr), size(0) {}

    ~ManualLinkedList() {
        Node<T>* cur = head;
        while (cur) {
            Node<T>* next = cur->next;
            delete cur->data; // Bebaskan memori objek Patient
            delete cur;
            cur = next;
        }
    }

    // ----------------------------------------------------------
    // add(): Tambah pasien dan auto-sort berdasarkan prioritas
    // Kompleksitas: O(n log n) → karena sort dipanggil setelah insert
    // ----------------------------------------------------------
    void add(T data) {
        Node<T>* newNode = new Node<T>(data);
        newNode->next = head;
        head = newNode;
        size++;
        mergeSortHelper(&head); // Auto-sort setelah insert
    }

    // ----------------------------------------------------------
    // popFront(): Ambil pasien dengan prioritas tertinggi (head)
    // Kompleksitas: O(1)
    // ----------------------------------------------------------
    T popFront() {
        if (!head) return nullptr;
        Node<T>* temp = head;
        T data = temp->data;
        head = head->next;
        delete temp; // Hapus node, tapi TIDAK hapus data (dikembalikan)
        size--;
        return data;
    }

    // ----------------------------------------------------------
    // ALGORITMA SEARCHING MANUAL 1: Linear Search by Name
    // Mencari pasien berdasarkan nama (substring, case-insensitive)
    //
    // Big O Analysis:
    //   - Best Case  : O(1)  → elemen pertama cocok
    //   - Average    : O(n/2) ≈ O(n)
    //   - Worst Case : O(n)  → elemen terakhir atau tidak ada
    //   - Space      : O(1)
    // ----------------------------------------------------------
    T linearSearchByName(const string& name) const {
        Node<T>* cur = head;

        // Konversi keyword ke lowercase untuk case-insensitive search
        string lowerName = name;
        for (char& c : lowerName) c = tolower(c);

        while (cur) {
            string patName = cur->data->getName();
            for (char& c : patName) c = tolower(c);

            // Cek apakah name adalah substring dari nama pasien
            if (patName.find(lowerName) != string::npos) {
                return cur->data;
            }
            cur = cur->next;
        }
        return nullptr; // Tidak ditemukan
    }

    // ----------------------------------------------------------
    // ALGORITMA SEARCHING MANUAL 2: Binary Search by ID
    // List sudah terurut by priorityScore (desc), bukan by ID.
    // Oleh karena itu kita konversi ke array, sort by ID,
    // lalu lakukan Binary Search.
    //
    // Big O Analysis:
    //   - Konversi ke array : O(n)
    //   - Insertion sort by ID: O(n^2) worst (umumnya data kecil)
    //   - Binary Search     : O(log n)
    //   - Total Worst Case  : O(n^2) → didominasi sorting
    //   - Space             : O(n)   → array sementara
    // ----------------------------------------------------------
    T binarySearchById(int targetId) const {
        if (!head) return nullptr;

        // Langkah 1: Salin ke array sementara
        int arrSize = size;
        T* arr = new T[arrSize];
        Node<T>* cur     = head;
        for (int i = 0; i < arrSize; i++) {
            arr[i] = cur->data;
            cur = cur->next;
        }

        // Langkah 2: Insertion Sort array by ID (ascending) - O(n^2)
        // (Diperlukan agar Binary Search bisa bekerja)
        // 
        for (int i = 1; i < arrSize; i++) {
            T key = arr[i];
            int j = i - 1;
            while (j >= 0 && arr[j]->getPatientId() > key->getPatientId()) {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }

        // Langkah 3: Binary Search - O(log n)
        // 
        int lo = 0, hi = arrSize - 1;
        T result = nullptr;
        while (lo <= hi) {
            int mid    = lo + (hi - lo) / 2;
            int midId  = arr[mid]->getPatientId();

            if (midId == targetId) {
                result = arr[mid]; // Ditemukan
                break;
            } else if (midId < targetId) {
                lo = mid + 1;     // Cari di kanan
            } else {
                hi = mid - 1;     // Cari di kiri
            }
        }

        delete[] arr;
        return result;
    }

    // ----------------------------------------------------------
    // removeById(): Hapus pasien berdasarkan ID
    // Kompleksitas: O(n) → linear traversal
    // ----------------------------------------------------------
    T removeById(int id) {
        Node<T>* cur  = head;
        Node<T>* prev = nullptr;

        while (cur && cur->data->getPatientId() != id) {
            prev = cur;
            cur  = cur->next;
        }

        if (!cur) return nullptr; // Tidak ditemukan

        T data = cur->data;
        if (!prev) {
            head = cur->next;
        } else {
            prev->next = cur->next;
        }
        delete cur;
        size--;
        return data;
    }

    // ----------------------------------------------------------
    // toJsonArray(): Konversi seluruh list ke JSON array string
    // Kompleksitas: O(n) → traversal semua node
    // ----------------------------------------------------------
    string toJsonArray() const {
        string result = "[";
        Node<T>* cur = head;
        bool first  = true;
        while (cur) {
            if (!first) result += ",";
            result += cur->data->toJson();
            first = false;
            cur = cur->next;
        }
        result += "]";
        return result;
    }

    // ----------------------------------------------------------
    // Utilitas
    // ----------------------------------------------------------
    int  getSize() const { return size; }
    bool isEmpty() const { return head == nullptr; }

    void displayAll() const {
        if (!head) {
            cout << "  [Antrean kosong]\n";
            return;
        }
        int pos = 1;
        Node<T>* cur = head;
        while (cur) {
            cout << "  " << pos++ << ". ";
            cur->data->displayInfo();
            cur = cur->next;
        }
    }
};

#endif // MANUALLINKEDLIST_H
