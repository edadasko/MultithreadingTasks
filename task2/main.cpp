#include "DynamicMutexQueue.cpp"
#include "StaticMutexQueue.cpp"
#include "StaticAtomicQueue.cpp"

void startTask(Queue &q, int producerNum, int consumerNum, int taskNum) {
    atomic<int> sum(0);
    
    auto producer = [&]() {
        for (int i = 0; i < taskNum; i ++) {
            q.push(1);
        }
    };
    
    auto consumer = [&]() {
        for (int i = 0; i < taskNum * producerNum / consumerNum; i ++) {
            byte poppedValue = 0;
            while(!q.pop(poppedValue));
            sum += poppedValue;
        }
    };
    
    vector<thread> threads;;
    for (int i = 0; i < producerNum; i++)
        threads.push_back(thread(producer));
    for (int i = 0; i < consumerNum; i++)
        threads.push_back(thread(consumer));
    for (int i = 0; i < consumerNum + producerNum; i++)
        threads[i].join();
    
    cout << "consumerNum: " << consumerNum << " producerNum: " << producerNum << endl;
    assert(sum == taskNum * producerNum);
}

void measureTime(Queue &q) {
    vector<int> consumerNums = {1, 2, 4};
    vector<int> produserNums = {1, 2, 4};
    int taskNum = 4 * 1024 * 1024;
    
    for (auto consumer: consumerNums) {
        for (auto producer: produserNums) {
            auto start = chrono::high_resolution_clock::now();
            startTask(q, producer, consumer, taskNum);
            auto end = chrono::high_resolution_clock::now();
            auto time = chrono::duration_cast<chrono::milliseconds>(end - start);
            cout << "time: " << (double)time.count() / 1000 << "\n\n";
        }
    }
}
    

int main() {
    DynamicMutexQueue a;
    measureTime(a);
    
    vector<int> sizes = {1, 4, 16};
    for (auto size: sizes) {
        StaticMutexQueue b (size);
        cout << "size: " << size << endl;
        measureTime(b);
    }
    
    for (auto size: sizes) {
        StaticAtomicQueue c (size);
        cout << "size: " << size << endl;
        measureTime(c);
    }
    
    return 0;
}
