#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>


template<typename T>
class safe_queue {

private:
	struct node {
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
	};

	mutable std::mutex _mthead;
	mutable std::mutex _mttail;
	std::condition_variable cv;
	node* _tail;
	std::unique_ptr<node> _head;

private:
	std::unique_ptr<node> pop_head() {

		std::unique_ptr<node> old = std::move(_head);
		_head = std::move(old->next);
		return old;
		
	}

	std::unique_lock<std::mutex> wait_for_data() {
		std::unique_lock<std::mutex> lock(_mthead);
		cv.wait(lock, [&]() {
			return _head.get()!=_tail;
			});

		return lock;
		
	}

	std::unique_ptr<node> wait_pop_head() {

		std::unique_lock<std::mutex> lock(wait_for_data());
		
		return pop_head();

	}




public:
	safe_queue() :_head(new node) { _tail = _head.get(); }
	safe_queue(const safe_queue&) = delete;
	safe_queue& operator=(safe_queue&) = delete;
	//safe_queue(const safe_queue&& val) :_head(std::move(val._head)),_tail(std::move(val._tail)) {}

	std::shared_ptr<T> pop_wait() {


		std::unique_ptr<node> ret = wait_pop_head();

		return ret->data;
	}

	void push(T value) {
		std::lock_guard<std::mutex> lock(_mttail);

		std::unique_ptr<node> newnode(new node);
		node* const next = newnode.get();
		std::shared_ptr<T> newvalue(std::make_shared<T>(std::move(value)));
		_tail->data = newvalue;
		_tail->next = std::move(newnode);
		_tail = next;

		cv.notify_one();


	}

	bool empty() {

		std::lock_guard<std::mutex> lock(_mthead);
		if (_head.get() == _tail) return true;
		return false;
	}

};

class MyClass {

public:
	MyClass(int value) :_value(value) {}
	MyClass(const MyClass& mc) { _value = mc._value; }
	MyClass(const MyClass&& mc):_value(std::move(mc._value)){ }

	friend std::ostream& operator<<(std::ostream& os, const MyClass& c) {

		std::cout << c._value;
		return os;
	}
private:
	int _value;
};


std::mutex mt;
void PrintCosumer(const std::string& str, std::shared_ptr<MyClass> mc) {

	std::lock_guard<std::mutex> lock(mt);
	std::cout << str << *mc << std::endl;
}

void testThread() {

	safe_queue<MyClass> st;

	std::thread consumer1([&]() {
		for (;;) {
			std::shared_ptr<MyClass> data = st.pop_wait();
			PrintCosumer("consumer1: ", data);
		}
		});

	std::thread consumer2([&]() {
		for (;;) {
			std::shared_ptr<MyClass> data = st.pop_wait();
			PrintCosumer("consumer2: ", data);
		}
		});

	std::thread producer([&]() {
		for (int i = 0;i < 100;i++) {

			MyClass mc(i);
			st.push(mc);

		}

		});

	consumer1.join();
	consumer2.join();
	producer.join();
}


int main() {



	testThread();
}