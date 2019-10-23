#ifndef UPDATE_H
#define UPDATE_H

#include <stdio.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <future>
#include <atomic>
#include <chrono>
#include <queue>

typedef uint8_t byte;
using namespace std;

class Queue {
public:
    virtual void push(byte val) = 0;
    virtual bool pop(byte& val) = 0;
};


#endif
