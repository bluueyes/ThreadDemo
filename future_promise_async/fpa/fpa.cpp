#include <iostream>
#include <future>
#include <chrono>
// ����һ���첽����
std::string fetchDataFromDB(std::string query) {
    // ģ��һ���첽���񣬱�������ݿ��л�ȡ����
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return "Data: " + query;
}
void use_async() {
    // ʹ�� std::async �첽���� fetchDataFromDB
    std::future<std::string> resultFromDB = std::async(std::launch::async, fetchDataFromDB, "Data");
    // �����߳�������������
    std::cout << "Doing something else..." << std::endl;
    // �� future �����л�ȡ����
    std::string dbData = resultFromDB.get();    //get����
    std::cout << dbData << std::endl;
   
}

int my_task() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "my task run 5 s" << std::endl;
    return 42;
}
void use_package() {
    // ����һ����װ������� std::packaged_task ����  
    std::packaged_task<int()> task(my_task);
    // ��ȡ����������� std::future ����  
    std::future<int> result = task.get_future();
    // ����һ���߳���ִ������  
    std::thread t(std::move(task));
    t.detach(); // ���߳������̷߳��룬�Ա����߳̿��Եȴ��������  
    // �ȴ�������ɲ���ȡ���  
    int value = result.get();
    std::cout << "The result is: " << value << std::endl;
}


void set_value(std::promise<int> prom) {
    // ���� promise ��ֵ
    prom.set_value(10);
    std::cout << "promise set Value success" << std::endl;
}

void use_promise() {
    // ����һ�� promise ����
    std::promise<int> prom;
    // ��ȡ�� promise ������� future ����
    std::future<int> fut = prom.get_future();
    // �����߳������� promise ��ֵ
    std::thread t(set_value, std::move(prom));
    // �����߳��л�ȡ future ��ֵ
    std::cout << "Waiting for the thread to set the value...\n";
    std::cout << "Value set by the thread: " << fut.get() << '\n';
    t.join();

}

void set_exception(std::promise<void> prom) {
    try {
        // �׳�һ���쳣
        throw std::runtime_error("An error occurred!");
    }
    catch (...) {
        // ���� promise ���쳣
        prom.set_exception(std::current_exception());
    }
}

void test_exception() {
    // ����һ�� promise ����
    std::promise<void> prom;
    // ��ȡ�� promise ������� future ����
    std::future<void> fut = prom.get_future();
    // �����߳������� promise ���쳣
    std::thread t(set_exception, std::move(prom));
    // �����߳��л�ȡ future ���쳣
    //���߳��������쳣һ��Ҫ�����߳�try catch�쳣����Ȼ���̻߳��
    try {
        std::cout << "Waiting for the thread to set the exception...\n";
        fut.get();
    }
    catch (const std::exception& e) {
        std::cout << "Exception set by the thread: " << e.what() << '\n';
    }
    t.join();
}

void use_promise_destruct() {
    std::thread t;
    std::future<int> fut;
    {
        // ����һ�� promise ����
        std::promise<int> prom;
        // ��ȡ�� promise ������� future ����
        fut = prom.get_future();
        // �����߳������� promise ��ֵ
        t = std::thread(set_value, std::move(prom));
    }
    // �����߳��л�ȡ future ��ֵ
    std::cout << "Waiting for the thread to set the value...\n";
    std::cout << "Value set by the thread: " << fut.get() << '\n';
    t.join();
}

void myFunction(std::promise<int>&& promise) {
    // ģ��һЩ����
    std::this_thread::sleep_for(std::chrono::seconds(1));
    promise.set_value(42); // ���� promise ��ֵ
}
void threadFunction(std::shared_future<int> future) {
    try {
        int result = future.get();
        std::cout << "Result: " << result << std::endl;
    }
    catch (const std::future_error& e) {
        std::cout << "Future error: " << e.what() << std::endl;
    }
}
void use_shared_future() {
    std::promise<int> promise;
    std::shared_future<int> future = promise.get_future();
    std::thread myThread1(myFunction, std::move(promise)); // �� promise �ƶ����߳���
    // ʹ�� share() ������ȡ�µ� shared_future ����  
    std::thread myThread2(threadFunction, future);
    std::thread myThread3(threadFunction, future);
    myThread1.join();
    myThread2.join();
    myThread3.join();
}

int main() {

    //use_async();
    //use_package();
    //use_promise();
    //test_exception();
    //use_promise_destruct();
    use_shared_future();
    return 0;
}