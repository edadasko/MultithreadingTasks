#include <stdio.h>
#include "Queue.cpp"

class StaticMutexQueue : public Queue {
private:
    byte* q;
    mutex m;
    int size;
    condition_variable conditionPush, conditionPop;
    atomic<int> popIndex, pushIndex, elementsCount;
    
public:
    StaticMutexQueue(int s) {
        size = s;
        q = new byte[size];
        for (int i = 0; i < size; i ++)
            q[i] = 0;
        elementsCount = 0;
        popIndex = 0;
        pushIndex = 0;
    }
    
    void push(byte val) override {
        unique_lock<mutex> lock(m);
        while (elementsCount == size)
            conditionPush.wait(lock);
        q[pushIndex % size] = val;
        pushIndex ++;
        elementsCount ++;
        conditionPop.notify_one();
    }
    
    bool pop(byte& val) override {
        unique_lock<mutex> lock(m);
        while (elementsCount == 0)
            conditionPop.wait(lock);
        val = q[popIndex % size];
        q[popIndex % size] = 0;
        popIndex ++;
        elementsCount --;
        conditionPush.notify_one();
        return true;
    }
};
