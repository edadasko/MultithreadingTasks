#include <stdio.h>
#include "Queue.cpp"

class DynamicMutexQueue : public Queue {
private:
    queue<byte> q;
    mutex commonMutex, pushMutex, popMutex;
    
public:
    void push(byte val) override {
        commonMutex.lock();
        pushMutex.lock();
        q.push(val);
        pushMutex.unlock();
        commonMutex.unlock();
    }
    
    bool pop(byte& val) override {
        popMutex.lock();
        commonMutex.lock();
        if (q.empty()){
            commonMutex.unlock();
            this_thread::sleep_for(chrono::milliseconds(1));
            if (q.empty()) {
                popMutex.unlock();
                return false;
            }
        }
        val = q.front();
        q.pop();
        popMutex.unlock();
        commonMutex.unlock();
        return true;
    }
};

class DynamicMutexQueueTask {
    static void startTask(int producerNum, int consumerNum, int taskNum) {
        DynamicMutexQueue q;
        atomic<int> sum(0);
        
        auto producer = [&]() {
            for (int i = 0; i < taskNum; i ++) {
                q.push(1);
            }
            return 0;
        };
        auto consumer = [&]() {
            for (int i = 0; i < taskNum * producerNum / consumerNum; i ++) {
                byte poppedValue = 0;
                while(!q.pop(poppedValue));
                sum += poppedValue;
            }
        };
        
        vector<thread> threads;
        for (int i = 0; i < producerNum; i++)
            threads.push_back(thread(producer));
        for (int i = 0; i < consumerNum; i++)
            threads.push_back(thread(consumer));
        for (int i = 0; i < consumerNum + producerNum; i++)
            threads[i].join();
        
        
        cout << "consumerNum: " << consumerNum << " producerNum: " << producerNum << endl;
        assert(sum == taskNum * producerNum);
        
    }
public:
    static void task() {
        vector<int> consumerNums = {1, 2, 4};
        vector<int> produserNums = {1, 2, 4};
        int taskNum = 4 * 1024 * 1024;
        
        for (auto consumer: consumerNums) {
            for (auto producer: produserNums) {
                auto start = chrono::high_resolution_clock::now();
                startTask(producer, consumer, taskNum);
                auto end = chrono::high_resolution_clock::now();
                auto time = chrono::duration_cast<chrono::milliseconds>(end - start);
                cout << "Time: " << (double)time.count() / 1000 << "\n\n";
            }
        }
    }
};
