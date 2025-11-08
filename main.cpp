#include <iostream>

#include "dynamic_array.h"



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

    std::cout << "Вывод через итераторы: \n";

    for (const auto& value : int_array ) {
        std::cout << value << ' ';
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