#include <gtest/gtest.h>
#include <memory_resource>
#include <map>
#include <memory>

// Включаем ваш исходный код
#include "../dynamic_array.h" // Замените на имя вашего файла

// Тесты для DynamicArray с int
TEST(DynamicArrayIntTest, BasicOperations) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);

    DynamicArray<int, std::pmr::polymorphic_allocator<int>> int_array(10, polymorphic_allocator);
    
    // Тестируем добавление элементов
    for (int i = 1; i <= 5; ++i) {
        int_array.push_back(i * 10);
    }
    
    // Проверяем, что элементы правильно добавлены
    EXPECT_EQ(int_array[0], 10);
    EXPECT_EQ(int_array[1], 20);
    EXPECT_EQ(int_array[2], 30);
    EXPECT_EQ(int_array[3], 40);
    EXPECT_EQ(int_array[4], 50);
}

TEST(DynamicArrayIntTest, ResizeTest) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);

    DynamicArray<int, std::pmr::polymorphic_allocator<int>> int_array(3, polymorphic_allocator);
    
    // Добавляем больше элементов, чем начальная capacity
    int_array.push_back(1);
    int_array.push_back(2);
    int_array.push_back(3);
    int_array.push_back(4); // Должен вызвать resize
    int_array.push_back(5);
    
    EXPECT_EQ(int_array[0], 1);
    EXPECT_EQ(int_array[1], 2);
    EXPECT_EQ(int_array[2], 3);
    EXPECT_EQ(int_array[3], 4);
    EXPECT_EQ(int_array[4], 5);
}

TEST(DynamicArrayIntTest, OutOfRangeTest) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);

    DynamicArray<int, std::pmr::polymorphic_allocator<int>> int_array(5, polymorphic_allocator);
    
    int_array.push_back(1);
    int_array.push_back(2);
    
    // Проверяем доступ в пределах
    EXPECT_NO_THROW(int_array[0]);
    EXPECT_NO_THROW(int_array[1]);
    
    // Проверяем выход за границы
    EXPECT_THROW(int_array[5], std::out_of_range);
    EXPECT_THROW(int_array[10], std::out_of_range);
}

TEST(DynamicArrayIntTest, IteratorTest) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<int> polymorphic_allocator(&custom_memory_resource);

    DynamicArray<int, std::pmr::polymorphic_allocator<int>> int_array(5, polymorphic_allocator);
    
    int_array.push_back(10);
    int_array.push_back(20);
    int_array.push_back(30);
    
    // Тестируем итераторы
    auto it = int_array.begin();
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_EQ(*it, 20);
    ++it;
    EXPECT_EQ(*it, 30);
    ++it;
    
    // Конец должен быть равен end()
    EXPECT_FALSE(it != int_array.end());
}

// Пользовательская структура для тестирования
struct TestStructure {
    int first_value;
    int second_value;
    
    bool operator==(const TestStructure& other) const {
        return first_value == other.first_value && second_value == other.second_value;
    }
};

// Тесты для DynamicArray с пользовательской структурой
TEST(DynamicArrayStructTest, StructBasicOperations) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<TestStructure> polymorphic_allocator(&custom_memory_resource);

    DynamicArray<TestStructure, std::pmr::polymorphic_allocator<TestStructure>> struct_array(5, polymorphic_allocator);
    
    // Добавляем структуры
    TestStructure s1{1, 10};
    TestStructure s2{2, 20};
    TestStructure s3{3, 30};
    
    struct_array.push_back(s1);
    struct_array.push_back(s2);
    struct_array.push_back(s3);
    
    // Проверяем поля структур
    EXPECT_EQ(struct_array[0].first_value, 1);
    EXPECT_EQ(struct_array[0].second_value, 10);
    EXPECT_EQ(struct_array[1].first_value, 2);
    EXPECT_EQ(struct_array[1].second_value, 20);
    EXPECT_EQ(struct_array[2].first_value, 3);
    EXPECT_EQ(struct_array[2].second_value, 30);
}

TEST(DynamicArrayStructTest, StructModification) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<TestStructure> polymorphic_allocator(&custom_memory_resource);

    DynamicArray<TestStructure, std::pmr::polymorphic_allocator<TestStructure>> struct_array(5, polymorphic_allocator);
    
    // Добавляем структуру
    TestStructure s1{100, 200};
    struct_array.push_back(s1);
    
    // Модифицируем поля
    struct_array[0].first_value = 999;
    struct_array[0].second_value = 888;
    
    // Проверяем изменения
    EXPECT_EQ(struct_array[0].first_value, 999);
    EXPECT_EQ(struct_array[0].second_value, 888);
}

TEST(DynamicArrayStructTest, StructIterator) {
    CustomMemoryResource custom_memory_resource;
    std::pmr::polymorphic_allocator<TestStructure> polymorphic_allocator(&custom_memory_resource);

    DynamicArray<TestStructure, std::pmr::polymorphic_allocator<TestStructure>> struct_array(5, polymorphic_allocator);
    
    TestStructure s1{5, 50};
    TestStructure s2{6, 60};
    
    struct_array.push_back(s1);
    struct_array.push_back(s2);
    
    // Тестируем итераторы со структурами
    auto it = struct_array.begin();
    EXPECT_TRUE(*it == s1);
    ++it;
    EXPECT_TRUE(*it == s2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}