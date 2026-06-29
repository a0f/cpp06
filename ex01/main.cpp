#include "Serializer.hpp"
#include "Data.hpp"
#include <iostream>

int main(void)
{
    Data data;
    data.id = 1337;
    data.name = "really sick data";

    std::cout << "------------------------------------\n";
    std::cout << "Original pointer\n";
    std::cout << "------------------------------------\n";
    std::cout << "address: " << &data << '\n';
    std::cout << "id:      " << data.id << '\n';
    std::cout << "name:    " << data.name << '\n';

    uintptr_t serialized = Serializer::serialize(&data);

    std::cout << "------------------------------------\n";
    std::cout << "Serialized\n";
    std::cout << "------------------------------------\n";
    std::cout << "value: " << serialized << '\n';

    Data* deserialized = Serializer::deserialize(serialized);

    std::cout << "------------------------------------\n";
    std::cout << "Deserialized pointer\n";
    std::cout << "------------------------------------\n";
    std::cout << "address: " << deserialized << '\n';
    std::cout << "id:      " << deserialized->id << '\n';
    std::cout << "name:    " << deserialized->name << '\n';

    std::cout << "------------------------------------\n";
    std::cout << "Round trip check\n";
    std::cout << "------------------------------------\n";
    if (deserialized == &data)
        std::cout << "Pointers match\n";
    else
        std::cout << "Pointers do not match\n";

    return 0;
}
