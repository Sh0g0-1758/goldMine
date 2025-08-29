#include <iostream>
#include <new>
#include <memory>
#include <type_traits>
#include <vector>

using namespace std;

class Arena {
public:
    explicit Arena(size_t size) : buffer(static_cast<char*>(::operator new(size))), capacity(size), offset(0) {}

    void* allocate(size_t size, size_t alignment) {
        char* current_ptr = buffer + offset;
        size_t space = capacity - offset;
        void* aligned_ptr = current_ptr;

        if(std::align(alignment, size, aligned_ptr, space) == nullptr) {
            throw bad_alloc();
        }

        offset = static_cast<char*>(aligned_ptr) - buffer + size;
        return aligned_ptr;
    }

    void reset() {offset = 0;}

    ~Arena() {::operator delete(buffer);}

    Arena(const Arena&) = delete;
    Arena(Arena&&) = delete;
private:
    char* buffer;
    size_t capacity;
    size_t offset;
};

class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t size) : arena(new Arena(size)) {}

    ~ArenaAllocator() {
        call_destructors();
        ::operator delete(arena);
    }

    template<typename T, typename... Args>
    T* create(Args... args) {
        auto ptr = arena->allocate(sizeof(T), alignof(T));
        T* obj = new (ptr) T(forward<Args>(args)...);
        if constexpr (!is_trivially_destructible_v<T>) {
            auto func = [](void* p) {
                static_cast<T*>(p)->~T();
            };
            dtors.push_back(make_pair(func, obj));
        }
        return obj;
    }

    void reset() {
        call_destructors();
        arena->reset();
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator(ArenaAllocator&&) = delete;
private:
    Arena* arena;
    vector<pair<void(*)(void*), void*>> dtors;

    void call_destructors() {
        for(auto it : dtors) {
            it.first(it.second);
        }
        dtors.clear();
    }
};

struct Bar {
    ~Bar() {
        std::cout << "non-trivial dtor" << std::endl;
    }
};

void func() {
    ArenaAllocator my_alloc(1024);
    Bar* bar = my_alloc.create<Bar>();
    std::cout << "End of scope" << std::endl;
}

int main() {
    func();
}
