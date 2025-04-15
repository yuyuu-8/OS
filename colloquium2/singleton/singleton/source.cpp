#include <mutex>
#include <atomic>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

// Стратегии создания
template <typename T>
struct DefaultCreation {
    static T* Create() {
        return new T();
    }

    static void Destroy(T* p) {
        delete p;
    }
};

template <typename T, typename... Args>
struct CreateWithArgs {
    static T* Create(Args... args) {
        return new T(std::forward<Args>(args)...);
    }

    static void Destroy(T* p) {
        delete p;
    }
};

// Шаблонные стратегии времени жизни
template <typename T>
class DefaultLifetime {
public:
    static void ScheduleDestruction(T*, void (*pFun)()) {
        std::atexit(pFun);
    }

    static void OnDeadReference() {
        throw std::runtime_error("Dead Reference Detected");
    }
};

template <typename T>
class PhoenixSingleton {
public:
    static void ScheduleDestruction(T*, void (*pFun)()) {
        std::atexit(pFun);
    }

    static void OnDeadReference() {
        // Разрешаем "восстановление" объекта
    }
};

// Шаблонные стратегии потокобезопасности
template <typename T>
class SingleThreaded {
public:
    struct Lock { Lock() {} };
    using AtomicInt = int;
};

template <typename T>
class ClassLevelLockable {
public:
    using Mutex = std::mutex;
    using Lock = std::lock_guard<Mutex>;
    using AtomicInt = std::atomic<int>;

    static inline Mutex mtx_;
};

// Ядро SingletonHolder
template <
    typename T,
    template <typename> class CreationPolicy = DefaultCreation,
    template <typename> class LifetimePolicy = DefaultLifetime,
    template <typename> class ThreadingModel = ClassLevelLockable
>
class SingletonHolder {
public:
    using InstanceType = T;
    using Threading = ThreadingModel<T>;

    static T& Instance() {
        typename Threading::Lock guard(Threading::mtx_);
        (void)guard;

        if (!pInstance_) {
            MakeInstance();
        }

        return *pInstance_;
    }

private:
    static void MakeInstance() {
        if (destroyed_) {
            LifetimePolicy<T>::OnDeadReference();
            destroyed_ = false;
        }

        pInstance_ = CreationPolicy<T>::Create();
        LifetimePolicy<T>::ScheduleDestruction(pInstance_, &DestroySingleton);
    }

    static void DestroySingleton() {
        CreationPolicy<T>::Destroy(pInstance_);
        pInstance_ = nullptr;
        destroyed_ = true;
    }

    inline static T* pInstance_ = nullptr;
    inline static typename Threading::AtomicInt destroyed_{ false };

    SingletonHolder() = delete;
    ~SingletonHolder() = delete;
    SingletonHolder(const SingletonHolder&) = delete;
    SingletonHolder& operator=(const SingletonHolder&) = delete;
};

// Пример использования
class DatabaseConnection {
public:
    DatabaseConnection() { std::cout << "DB Connected\n"; }
    ~DatabaseConnection() { std::cout << "DB Disconnected\n"; }

    void query(const std::string& sql) {
        std::cout << "Executing: " << sql << "\n";
    }
};

// Конфигурация Singleton
using DBSingleton = SingletonHolder<
    DatabaseConnection,
    DefaultCreation,
    PhoenixSingleton,
    ClassLevelLockable
>;

int main() {
    auto& db1 = DBSingleton::Instance();
    db1.query("SELECT * FROM users");

    auto& db2 = DBSingleton::Instance();
    std::cout << "Same instance: " << (&db1 == &db2) << "\n";

    // Тестирование "феникса"
    DBSingleton::Instance().~DatabaseConnection(); // Имитация деструкции
    auto& db3 = DBSingleton::Instance();     // Должен восстановиться
    db3.query("SELECT * FROM logs");

    return 0;
}