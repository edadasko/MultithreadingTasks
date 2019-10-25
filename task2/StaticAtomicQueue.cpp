#include <stdio.h>
#include "Queue.cpp"

class StaticAtomicQueue : public Queue {
private:
    atomic<byte>* q;
    int size;
    atomic<int> pushIndex, popIndex;
    
public:
    StaticAtomicQueue(int s) {
        pushIndex = 0;
        popIndex = 0;
        size = s;
        q = new atomic<byte>[size];
        for (int i = 0; i < size; i ++)
            q[i] = 0;
    }
    
    void push(byte val) override {
        while (true) {
            int currentPushIndex = pushIndex.load();
            if (currentPushIndex == popIndex.load() + size) {
                continue;
            }
            
            byte x = q[currentPushIndex % size];
            if (x != 0) continue;
            
            if (pushIndex.compare_exchange_strong(currentPushIndex,
                                                  currentPushIndex + 1)){
                if (q[currentPushIndex % size].compare_exchange_strong(x, val))
                    return;
            }
        }
    }
    
    bool pop(byte& val) override {
        int currentPopIndex = popIndex.load();
        if (currentPopIndex  == pushIndex.load()) {
            return false;
        }
        
        byte x = q[currentPopIndex % size];
        if (x == 0) return false;
        if (popIndex.compare_exchange_strong(currentPopIndex,
                                             currentPopIndex + 1)) {
            
            if (q[currentPopIndex % size].compare_exchange_strong(x, 0)) {
                val = x;
                return true;
            }
        }
        return false;
    }
    
    ~StaticAtomicQueue() {
        delete[] q;
    }
};
