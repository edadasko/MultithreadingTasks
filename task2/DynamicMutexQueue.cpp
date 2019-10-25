#include <stdio.h>
#include "Queue.cpp"

class DynamicMutexQueue : public Queue {
private:
    queue<byte> q;
    mutex m;
    
public:
    void push(byte val) override {
        m.lock();
        q.push(val);
        m.unlock();
    }
    
    bool pop(byte& val) override {
        m.lock();
        if (q.empty()){
            m.unlock();
            this_thread::sleep_for(chrono::milliseconds(1));
                return false;
        }
        val = q.front();
        q.pop();
        m.unlock();
        return true;
    }
};
