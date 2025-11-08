#include <iostream>
#include <memory_resource>
#include <cstdlib>
#include <map>
#include <memory>


class CustomMemoryResource : public std::pmr::memory_resource {
    // использованная память хранится в мапе в виде:
    // ключ - начало выделенной памяти,
    // значение - размер выделенной памяти
    std::map <void*, size_t> used_blocks;
public:
    void *do_allocate(size_t bytes, size_t alignment) override {
        // Выравнивание должно быть степенью двойки
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
            std::cout << "Ошибка: выравнивание должно быть степенью двойки" << '\n';
            throw std::bad_alloc();
        }

        if (bytes == 0) bytes = 1;

        if (bytes % alignment != 0) {
            std::cout << "Ошибка: количество байтов должно нацело делится на выравниивание" << '\n';
            throw std::bad_alloc();
        }

        void *ptr = std::aligned_alloc(alignment, bytes);
        if (!ptr) throw std::bad_alloc();

        this->used_blocks[ptr] = bytes;
        return ptr;
    }

    void do_deallocate(void *ptr, size_t bytes, size_t alignment) override {
        if (ptr != nullptr) {
            std::free(ptr);
            auto it = this->used_blocks.find(ptr);
            this->used_blocks.erase(it);
            return;
        }
        throw std::logic_error("Попытка освобождения не выделенного памяти");
    }
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};


template <class T, class allocator_type> 
    requires std::is_default_constructible_v<T> && 
             std::is_same_v<allocator_type, std::pmr::polymorphic_allocator<T>>
class DynamicArray {
    struct PolymorphicDeleter {
        void operator()(T* ptr) const {
            //  память освобождается автоматически перед аллокатор
        }
    };

    allocator_type polymorphic_allocator; //  полиморфный аллокатор
    std::unique_ptr<T, PolymorphicDeleter> data_pointer; //  указатель на данные
    size_t size; // размер массива
    size_t capacity; // размерность массива


public:
    DynamicArray(size_t capacity, allocator_type alloc = {}) : polymorphic_allocator(alloc), capacity(capacity), size(0) {
        //  выделение памяти через полиморфный аллокатор
        T* raw_pointer = this->polymorphic_allocator.allocate(capacity);

        //  конструирование объектов в выделенной памяти
        for (size_t i = 0; i < capacity; ++i) {
            this->polymorphic_allocator.construct(raw_pointer + i);
        }

        //  создание unique_ptr с пользовательским deleter
        this->data_pointer = std::unique_ptr<T, PolymorphicDeleter>(raw_pointer, PolymorphicDeleter{});
    }

    void resize() {
        this->capacity *= 2;

        T* raw_pointer = this->polymorphic_allocator.allocate(this->capacity);
        
        for (size_t i = 0; i < this->capacity; ++i) {
            this->polymorphic_allocator.construct(raw_pointer + i);
        }

        for (size_t i = 0; i < size; ++i) {
            *(raw_pointer+i) = std::move(this->data_pointer.get()[i]);
        }

        this->data_pointer = std::unique_ptr<T, PolymorphicDeleter>(raw_pointer, PolymorphicDeleter{});
    }

    void push_back(T value) {
        if (this->size + 1 == this->capacity) {
            this->resize();            
        }
        auto ptr = (this->data_pointer.get() + this->size); 
        *ptr = value;
        ++this->size;
    }

    T& operator[](size_t index) {
        if (index < this->capacity) {
            return data_pointer.get()[index];
        }
        throw std::out_of_range("Выход за границы массива");
    }

    const T& operator[](size_t index) const {
        if (index < this->size) {
            return this->data_pointer.get()[index];
        }
        throw std::out_of_range("Выход за границы массива");
    }

    T* begin() {
        return this->data_pointer.get();
    }

    T* end() {
        return this->data_pointer.get() + this->capacity;
    }


    ~DynamicArray() {
        if constexpr (std::is_destructible_v<T>) {
            for (size_t i = 0; i < this->size; ++i) {
                std::allocator_traits<allocator_type>::destroy(polymorphic_allocator, this->data_pointer.get() + i);
            }
        }
        this->polymorphic_allocator.deallocate(this->data_pointer.get(), this->capacity);
    }
};



int main() {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);

    std::cout << "Демонстрация работы int\n";

    DynamicArray<int, std::pmr::polymorphic_allocator<int>> int_array(10, polymorphic_allocator);
    for (int i = 1; i <= 50; ++i) {
        int_array.push_back(i);
    }
    for (int i = 0; i < 50; ++i) {
        std::cout << int_array[i] << ' ';
    }
    std::cout << '\n';




    std::cout << "Демонстрация работы пользовательской структуры\n";

    struct TestStructure {
        int first_value;
        int second_value;
    };

    DynamicArray<TestStructure, std::pmr::polymorphic_allocator<TestStructure>> 
        structure_array(10, std::pmr::polymorphic_allocator<TestStructure>(&custom_memory_resource));



    structure_array[0] = {1, 1};
    structure_array[1] = {1, 2};

    std::cout << structure_array[0].first_value << ' ' << structure_array[0].second_value << '\n'; 
    std::cout << structure_array[1].first_value << ' ' << structure_array[1].second_value << '\n'; 

    return 0;
}