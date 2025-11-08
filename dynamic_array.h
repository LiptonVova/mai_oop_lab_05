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


template <class ArrayType>
class ArrayIterator {
    ArrayType* array_pointer; // указатель на массив
    size_t current_index; 
    size_t array_size;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = ArrayType::item_type;
    using difference_type = int;
    using pointer = ArrayType::item_type*;
    using reference = ArrayType::item_type&;

    ArrayIterator(ArrayType* array_pointer, size_t start_index, size_t array_size) :
        array_pointer(array_pointer), current_index(start_index), array_size(array_size) {}


    reference operator*() {
        if (current_index >= array_size) {
            throw std::out_of_range("Выход за границу");
        }
        return (*array_pointer)[current_index];
    }

    pointer operator->() {
        if (current_index >= array_size) {
            throw std::out_of_range("Выход за границу");
        }
        return (*array_pointer)[current_index];
    }

    bool operator!=(const ArrayIterator<ArrayType> &other) const {
        return (other.current_index != current_index) || (other.array_pointer != array_pointer);
    }

    ArrayIterator<ArrayType> &operator++() {
        ++current_index;
        return *this;
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
    using item_type = T;

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

    ArrayIterator<DynamicArray<T, allocator_type>> begin() {
        return ArrayIterator<DynamicArray<T, allocator_type>>(this, 0, size);
    }

    ArrayIterator<DynamicArray<T, allocator_type>> end() {
        return ArrayIterator<DynamicArray<T, allocator_type>>(this, size, size);
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