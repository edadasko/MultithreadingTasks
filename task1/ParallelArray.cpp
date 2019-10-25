#ifndef UPDATE_H
#define UPDATE_H

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <queue>

typedef long long ll;
typedef uint8_t byte;

using namespace std;

class ParallelArray {
public:
    const int NumTasks = 1024 * 1024;
    byte* byteArray;
    
    ParallelArray() {};
    
    void print() {
        for (int i = 0; i < NumTasks; i++)
            cout << static_cast<int>(byteArray[i]) << " ";
    }
    
    void initArray() {
        byteArray = new byte[NumTasks];
        for (int i = 0; i < NumTasks; i++)
            byteArray[i] = 0;
    }
    
    void check() {
        for (int i = 0; i < NumTasks; i++)
            assert(byteArray[i] == 1);
    }
    
    virtual void init() = 0;
    virtual void increment() = 0;
    
    ~ParallelArray() {
        delete[] byteArray;
    }
};

#endif
