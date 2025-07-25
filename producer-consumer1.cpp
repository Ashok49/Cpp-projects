#include <iostream>
using namespace std;
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>


std::mutex mtx;
std::queue<int> buffer;
const unsigned int BUFFER_SIZE = 50;
std::condition_variable cv;
bool done = false;

void producer(){
	for(int i=0;i < 100 ; i++){
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] { return buffer.size() < BUFFER_SIZE; });
		buffer.push(i);
		std::cout<< "Produced: " << i << std::endl;
		lock.unlock();
		cv.notify_one();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	// Signal consumer to exit by pushing a sentinel value
	{
		std::unique_lock<std::mutex> lock(mtx);
		done = true;
		cv.notify_all();
	}
}


void consumer(){
	while(true){
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] { return !buffer.empty() || done; });
		if (buffer.empty() && done) {
			// No more items and producer is done
			break;
		}
		if (!buffer.empty()) {
			int item = buffer.front();
			buffer.pop();
			std::cout << "Consumed: " << item << std::endl;
			lock.unlock();
			cv.notify_one();
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
		}
	}
}


int main() {
	//create producer and consumer threads and join
	thread producer_thread(producer);
	thread consumer_thread(consumer);
	producer_thread.join();
	consumer_thread.join();
	cout << "Producer and Consumer threads finished." << endl;
}
