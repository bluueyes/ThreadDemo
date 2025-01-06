#include <iostream>
#include <mutex>
#include <memory>

template<typename T,size_t Cap>
class CirQueue:private std::allocator<T> {

public:
	CirQueue() :_cap(Cap + 1), _data(std::allocator<T>::allocate(_cap)), _tail(0), _head(0) {
	}
	CirQueue(const CirQueue&) = delete;
	CirQueue& operator=(const CirQueue&) = delete;

	~CirQueue() {

		std::lock_guard<std::mutex> lock(_mt);
		while (_head != _tail) {

			std::allocator<T>::destroy(_data+_head);
			_head = (_head + 1) % _cap;
		}

		//回收内存
		std::allocator<T>::deallocate(_data, _cap);
	}


	template<class ...Args>
	bool emplace(Args&&... args) {
		
		std::lock_guard<std::mutex> lock(_mt);

		if ((_tail + 1) % _cap == _head) {

			std::cout << "push error,queue is full" << std::endl;
			return false;
		}

		std::allocator<T>::construct(_data + _tail, std::forward<Args>(args)...);
		_tail = (_tail + 1) % _cap;
		return true;
	}


	bool push(const T& value) {
		
		std::cout << "call push cosnt T&" << std::endl;
		return emplace(value);
	}

	bool push(T&& value) {

		std::cout << "call push T&&" << std::endl;
		return emplace(std::move(value));
		return true;
	}

	bool pop(T& value) {

		std::lock_guard<std::mutex> lock(_mt);
		if (_tail == _head) {
			std::cout << "pop error,queue is null" << std::endl;
			return false;
		}
		
		value = std::move(_data[_head]);
		_head = (_head + 1) % _cap;

		return true;
	}



private:
	size_t _cap;
	T* _data;
	std::mutex _mt;
	size_t _tail;
	size_t _head;

};

class MyClass {
public:
	MyClass(const std::string& st=" ") :_st(st) {}
	MyClass(const MyClass& c) = default;
	MyClass& operator=(const MyClass& c)=default;
	MyClass(MyClass&& c) {
		_st = std::move(c._st);
		std::cout << "move"<<std::endl;
	}
	MyClass& operator=(MyClass&& c) {
		if (&c == this) return *this;
		_st = std::move(c._st);
		return *this;
	}
	~MyClass() {}

	friend std::ostream& operator<<(std::ostream& os, const MyClass& c);

private:
	std::string _st;
};

std::ostream& operator<<(std::ostream& os,const MyClass& c) {

	os << c._st;
	return os;
}


void TestCircularQue() {
	//最大容量为10
	CirQueue<MyClass, 5> cq_lk;
	MyClass mc1("hh");
	MyClass mc2("2");
	cq_lk.push(mc1);
	cq_lk.push(std::move(mc2));
	for (int i = 3; i <= 5; i++) {
		MyClass mc("3");
		auto res = cq_lk.push(mc);
		if (res == false) {
			break;
		}
	}
	cq_lk.push(mc2);
	for (int i = 0; i < 5; i++) {
		MyClass mc1;
		auto res = cq_lk.pop(mc1);
		if (!res) {
			break;
		}
		std::cout << "pop success, " << mc1 << std::endl;
	}
	auto res = cq_lk.pop(mc1);
}



int main() {

	TestCircularQue();


}
