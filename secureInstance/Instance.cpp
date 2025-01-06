#include <iostream>
#include <thread>
#include <mutex>
#include <memory>


class Single2 {

private:
	Single2() {
	}

	Single2(const Single2& s) = delete;
	Single2& operator=(const Single2& s )= delete;

public:
	static Single2& getInstance() {

		static Single2 single;
		return single;
	}
};
//�����汾�ĵ���ģʽ��C++11 ��ǰ���ڶ��̲߳���ȫ����������������ܻ��ʼ�������̬������
//����C++11�Ƴ��Ժ󣬸������Ż����������ܱ�֤�̰߳�ȫ������Ϊ�˱�֤���а�ȫ��ȷ��ʹ��C++11���ϵı�׼

void test_single2() {

	std::cout << "s1 addr is " << &Single2::getInstance() << std::endl;
	std::cout << "s2 addr is " << &Single2::getInstance() << std::endl;

}

//c++11��ǰ�Ƴ��̰߳�ȫ�Ķ���ʽ
class Single2Hungry {

private:
	Single2Hungry() {
	}

	Single2Hungry(const Single2Hungry& s) = delete;
	Single2Hungry& operator=(const Single2Hungry& s) = delete;

public:
	static Single2Hungry* getInstance() {
		
		if (single == nullptr) {
			
			single=new Single2Hungry();
		}

		return single;
	}

private:
	static Single2Hungry* single;
};

Single2Hungry* Single2Hungry::single = Single2Hungry::getInstance();

void thread_func_s2(int i)
{
	std::cout << "this is thread " << i << std::endl;
	std::cout << "inst is " << Single2Hungry::getInstance() << std::endl;
}
void test_single2hungry()
{
	std::cout << "s1 addr is " << Single2Hungry::getInstance() << std::endl;
	std::cout << "s2 addr is " << Single2Hungry::getInstance() << std::endl;
	for (int i = 0; i < 3; i++)
	{
		std::thread tid(thread_func_s2, i);
		tid.join();
	}
}


//����ʽ
class SinglePointer {

private:
	SinglePointer(){}

	SinglePointer(const SinglePointer&) = delete;
	SinglePointer& operator = (const SinglePointer&) = delete;

public:
	static SinglePointer* getInstance() {

		if (single != nullptr) {
			return single;
		}

		mt.lock();
		if (single != nullptr) {
			mt.unlock();
			return single;
		}
		single = new SinglePointer();
		mt.unlock();
		return single;
	}

private:
	static SinglePointer* single;
	static std::mutex mt;

};

SinglePointer* SinglePointer::single = nullptr;
std::mutex SinglePointer::mt;

void thread_func_lazy(int i)
{
	std::cout << "this is lazy thread " << i << std::endl;
	std::cout << "inst is " << SinglePointer::getInstance() << std::endl;
}
void test_singlelazy()
{
	for (int i = 0; i < 3; i++)
	{
		std::thread tid(thread_func_lazy, i);
		tid.join();
	}
	//��ʱ�ͷ�new�Ķ�������ڴ�й©
}

//��������ָ�������ģʽ
class SingleAuto {
public:
	~SingleAuto() { std::cout << "������" << std::endl; }
private:
	SingleAuto() {}
	SingleAuto(const SingleAuto&) = delete;
	SingleAuto& operator = (const SingleAuto&) = delete;

public:
	static std::shared_ptr<SingleAuto> getInstance() {

		if (single != nullptr) {
			return single;
		}

		mt.lock();
		if (single != nullptr) {
			mt.unlock();
			return single;
		}
		single = std::shared_ptr<SingleAuto>(new SingleAuto());
		mt.unlock();
		return single;
	}

private:
	static std::shared_ptr<SingleAuto> single;
	static std::mutex mt;

};

std::shared_ptr<SingleAuto> SingleAuto::single = nullptr;
std::mutex SingleAuto::mt;
void test_singleauto()
{
	auto sp1 = SingleAuto::getInstance();
	auto sp2 = SingleAuto::getInstance();
	std::cout << "sp1  is  " << sp1 << std::endl;
	std::cout << "sp2  is  " << sp2 << std::endl;
	//��ʱ���������������ֶ�ɾ����ָ�룬��ɱ���
	// delete sp1.get();
}

//��������ָ��Ӱ�ȫ�ͷŵ�����ģʽ

class SingleAutoSafe;
class SafeDeletor {

	void operator()(SingleAutoSafe* sf){

		std::cout << "this is deletor operator()" << std::endl;
		delete sf;
	}
};

class SingleAutoSafe {

private:
	SingleAutoSafe() {}
	SingleAutoSafe(const SingleAutoSafe&) = delete;
	SingleAutoSafe& operator = (const SingleAutoSafe&) = delete;
	~SingleAutoSafe() { std::cout << "������" << std::endl; }
	//������Ԫ�࣬ͨ����Ԫ����ø�����������
	friend SafeDeletor;
public:
	static std::shared_ptr<SingleAutoSafe> getInstance() {

		if (single != nullptr) {
			return single;
		}

		mt.lock();
		if (single != nullptr) {
			mt.unlock();
			return single;
		}
		//����ָ��ɾ����
		//single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe,SafeDeletor());
		mt.unlock();
		return single;
	}

private:
	static std::shared_ptr<SingleAutoSafe> single;
	static std::mutex mt;

};

std::shared_ptr<SingleAutoSafe> SingleAutoSafe::single = nullptr;
std::mutex SingleAutoSafe::mt;
void test_singleautosafe()
{
	auto sp1 = SingleAutoSafe::getInstance();
	auto sp2 = SingleAutoSafe::getInstance();
	std::cout << "sp1  is  " << sp1 << std::endl;
	std::cout << "sp2  is  " << sp2 << std::endl;

}

class SingletonOnce {
private:
	SingletonOnce() = default;
	SingletonOnce(const SingletonOnce&) = delete;
	SingletonOnce& operator = (const SingletonOnce& st) = delete;
	static std::shared_ptr<SingletonOnce> _instance;
public:
	static std::shared_ptr<SingletonOnce> GetInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			_instance = std::shared_ptr<SingletonOnce>(new SingletonOnce);
			});
		return _instance;
	}
	void PrintAddress() {
		std::cout << _instance.get() << std::endl;
	}
	~SingletonOnce() {
		std::cout << "this is singleton destruct" << std::endl;
	}
};
std::shared_ptr<SingletonOnce> SingletonOnce::_instance = nullptr;

void TestSingle() {
	std::thread t1([]() {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		SingletonOnce::GetInstance()->PrintAddress();
		});
	std::thread t2([]() {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		SingletonOnce::GetInstance()->PrintAddress();
		});
	t1.join();
	t2.join();
}

//����ģ��
template<typename T>
class SingleTem {
private:
	SingleTem() = default;
	SingleTem(const SingleTem&) = delete;
	SingleTem& operator=(const SingleTem&) = delete;
	static std::shared_ptr<T> single;

public:
	std::shared_ptr<T>& getInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			single = std::shared_ptr<T>(new T);
			})
			return single;
	}

	void PrintAddress() {
		std::cout << _instance.get() << std::endl;
	}
	~Singleton() {
		std::cout << "this is singleton destruct" << std::endl;
	}
};

template<typename T>
std::shared_ptr<T> SingleTem<T>::single = nullptr;

int main() {

	//test_single2();
	//test_single2hungry();
	//test_singlelazy();
	//test_singleauto();
	//test_singleautosafe();
	TestSingle();
	return 0;
}