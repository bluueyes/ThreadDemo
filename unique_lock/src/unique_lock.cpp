#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <map>

//unique_lock �����÷�
std::mutex mtx;
int shared_data = 0;
void use_unique() {
    //lock���Զ�������Ҳ���ֶ�����
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "lock success" << std::endl;
    shared_data++;
    lock.unlock();
}

//ͨ��lock.owns_lock()�ж�unique_lock�Ƿ�����
void owns_lock() {
    //lock���Զ�������Ҳ���ֶ�����
    std::unique_lock<std::mutex> lock(mtx);
    shared_data++;
    if (lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    }
    else {
        std::cout << "doesn't own lock" << std::endl;
    }
    lock.unlock();
    if (lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    }
    else {
        std::cout << "doesn't own lock" << std::endl;
    }
}

//ͬʱʹ��owns��defer
void use_own_defer() {
    std::unique_lock<std::mutex>  lock(mtx);
    // �ж��Ƿ�ӵ����
    if (lock.owns_lock())
    {
        std::cout << "Main thread has the lock." << std::endl;
    }
    else
    {
        std::cout << "Main thread does not have the lock." << std::endl;
    }
    std::thread t([]() {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        // �ж��Ƿ�ӵ����
        if (lock.owns_lock())
        {
            std::cout << "Thread has the lock." << std::endl;
        }
        else
        {
            std::cout << "Thread does not have the lock." << std::endl;
        }
        // ����
        lock.lock();
        // �ж��Ƿ�ӵ����
        if (lock.owns_lock())
        {
            std::cout << "Thread has the lock." << std::endl;
        }
        else
        {
            std::cout << "Thread does not have the lock." << std::endl;
        }
        // ����
        lock.unlock();
        });
    t.join();
}

//ͬ��֧����������
void use_own_adopt() {
    mtx.lock();
    std::unique_lock<std::mutex> lock(mtx, std::adopt_lock);
    if (lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;
    }
    else {
        std::cout << "does not have the lock" << std::endl;
    }
    lock.unlock();
}

int a = 10;
int b = 99;
std::mutex  mtx1;
std::mutex  mtx2;
//����unique_lock֮������������Լ�����
void safe_swap() {
    std::lock(mtx1, mtx2);
    std::unique_lock<std::mutex> lock1(mtx1, std::adopt_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::adopt_lock);
    std::swap(a, b);
    //�����÷�
    //mtx1.unlock();
    //mtx2.unlock();
}

void safe_swap2() {
    std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);
    //����lock1,lock2����
    std::lock(lock1, lock2);
    //�����÷�
    //std::lock(mtx1, mtx2);
    std::swap(a, b);
}

//ת�ƻ���������Ȩ
//����������֧��move����������unique_lock֧��
// ���ﷵ��û�п��������������ƶ����췵��
//����unique_lock����һ����ʱ������������ֵ��lock�ƶ�������
std::unique_lock <std::mutex>  get_lock() {
    std::unique_lock<std::mutex>  lock(mtx);
    shared_data++;
    return lock;
}
void use_return() {
    std::unique_lock<std::mutex> lock(get_lock());
    shared_data++;
}

void precision_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    shared_data++;
    lock.unlock();
    //����ƹ������ݵĺ�ʱ������Ҫ��������ִ��
    std::this_thread::sleep_for(std::chrono::seconds(1));
    lock.lock();
    shared_data++;
} 

class DNService {
public:
    DNService() {}
    //���������ù�����
    std::string QueryDNS(std::string dnsname) {
        std::shared_lock<std::shared_mutex> shared_locks(_shared_mtx);
        auto iter = _dns_info.find(dnsname);
        if (iter != _dns_info.end()) {
            return iter->second;
        }
        return "";
    }
    //д�������ö�ռ��
    void AddDNSInfo(std::string dnsname, std::string dnsentry) {
        std::lock_guard<std::shared_mutex>  guard_locks(_shared_mtx);
        _dns_info.insert(std::make_pair(dnsname, dnsentry));
    }
private:
    std::map<std::string, std::string> _dns_info;
    mutable std::shared_mutex  _shared_mtx;
};

class RecursiveDemo {
public:
    RecursiveDemo() {}
    bool QueryStudent(std::string name) {
        std::lock_guard<std::recursive_mutex>  recursive_lock(_recursive_mtx);
        auto iter_find = _students_info.find(name);
        if (iter_find == _students_info.end()) {
            return false;
        }
        return true;
    }
    void AddScore(std::string name, int score) {
        std::lock_guard<std::recursive_mutex>  recursive_lock(_recursive_mtx);
        if (!QueryStudent(name)) {
            _students_info.insert(std::make_pair(name, score));
            return;
        }
        _students_info[name] = _students_info[name] + score;
    }
    //���Ƽ����õݹ�����ʹ�õݹ���˵�����˼·�������룬���Ż����
    //�Ƽ�����߼����������߼����Ϊͳһ�ӿ�
    void AddScoreAtomic(std::string name, int score) {
        std::lock_guard<std::recursive_mutex>  recursive_lock(_recursive_mtx);
        auto iter_find = _students_info.find(name);
        if (iter_find == _students_info.end()) {
            _students_info.insert(std::make_pair(name, score));
            return;
        }
        _students_info[name] = _students_info[name] + score;
        return;
    }
private:
    std::map<std::string, int> _students_info;
    std::recursive_mutex   _recursive_mtx;
};

int main() {

    //use_unique();
    //owns_lock();
    //use_own_defer();
    //use_own_adopt();
    return 0;
}