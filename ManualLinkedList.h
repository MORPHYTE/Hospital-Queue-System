#ifndef MANUALLINKEDLIST_H
#define MANUALLINKEDLIST_H

#include "Patient.h"
#include <stdexcept>
using namespace std;

template <typename T>
struct Node {
    T data;
    Node* next;
    explicit Node(T d) : data(d), next(nullptr) {}
};

template <typename T>
class ManualLinkedList {
private:
    Node<T>* head;
    int size;

    Node<T>* merge(Node<T>* left, Node<T>* right) {
        if (!left)  return right;
        if (!right) return left;

        if (left->data->getPriorityScore() >= right->data->getPriorityScore()) {
            left->next = merge(left->next, right);
            return left;
        } else {
            right->next = merge(left, right->next);
            return right;
        }
    }

    void splitList(Node<T>* source, Node<T>** front, Node<T>** back) {
        Node<T>* slow = source;
        Node<T>* fast = source->next;

 
        while (fast != nullptr) {
            fast = fast->next;
            if (fast != nullptr) {
                slow = slow->next;
                fast = fast->next;
            }
        }

        *front = source;
        *back  = slow->next;
        slow->next = nullptr; 
    }


    void mergeSortHelper(Node<T>** headRef) {
        Node<T>* h = *headRef;
        if (!h || !h->next) return; 

        Node<T>* a;
        Node<T>* b;
        splitList(h, &a, &b); 

        mergeSortHelper(&a);  
        mergeSortHelper(&b);  

        *headRef = merge(a, b); 
    }

public:
    ManualLinkedList() : head(nullptr), size(0) {}

    ~ManualLinkedList() {
        Node<T>* cur = head;
        while (cur) {
            Node<T>* next = cur->next;
            delete cur->data;
            delete cur;
            cur = next;
        }
    }

    void add(T data) {
        Node<T>* newNode = new Node<T>(data);
        newNode->next = head;
        head = newNode;
        size++;
        mergeSortHelper(&head);
    }

    T popFront() {
        if (!head) return nullptr;
        Node<T>* temp = head;
        T data = temp->data;
        head = head->next;
        delete temp;
        size--;
        return data;
    }

    T linearSearchByName(const string& name) const {
        Node<T>* cur = head;

        string lowerName = name;
        for (char& c : lowerName) c = tolower(c);

        while (cur) {
            string patName = cur->data->getName();
            for (char& c : patName) c = tolower(c);

            if (patName.find(lowerName) != string::npos) {
                return cur->data;
            }
            cur = cur->next;
        }
        return nullptr;
    }

    T binarySearchById(int targetId) const {
        if (!head) return nullptr;

        int arrSize = size;
        T* arr = new T[arrSize];
        Node<T>* cur     = head;
        for (int i = 0; i < arrSize; i++) {
            arr[i] = cur->data;
            cur = cur->next;
        }

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


        int lo = 0, hi = arrSize - 1;
        T result = nullptr;
        while (lo <= hi) {
            int mid    = lo + (hi - lo) / 2;
            int midId  = arr[mid]->getPatientId();

            if (midId == targetId) {
                result = arr[mid]; 
                break;
            } else if (midId < targetId) {
                lo = mid + 1;    
            } else {
                hi = mid - 1;    
            }
        }

        delete[] arr;
        return result;
    }

    T removeById(int id) {
        Node<T>* cur  = head;
        Node<T>* prev = nullptr;

        while (cur && cur->data->getPatientId() != id) {
            prev = cur;
            cur  = cur->next;
        }

        if (!cur) return nullptr; 

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
