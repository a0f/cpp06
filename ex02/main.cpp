#include "Base.hpp"
#include "A.hpp"
#include "B.hpp"
#include "C.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>

Base* generate(void)
{
    int r = std::rand() % 3;

    if (r == 0)
        return new A();
    if (r == 1)
        return new B();
    return new C();
}

void identify(Base* p)
{
    if (dynamic_cast<A*>(p) != NULL)
        std::cout << "A\n";
    else if (dynamic_cast<B*>(p) != NULL)
        std::cout << "B\n";
    else if (dynamic_cast<C*>(p) != NULL)
        std::cout << "C\n";
}

void identify(Base& r)
{
    try
    {
        dynamic_cast<A&>(r);
        std::cout << "A\n";
        return;
    }
    catch (std::exception& e) {}
    try
    {
        dynamic_cast<B&>(r);
        std::cout << "B\n";
        return;
    }
    catch (std::exception& e) {}
    std::cout << "C\n";
}

int main(void)
{
    std::srand(std::time(NULL));

    for (int i = 0; i < 6; i++)
    {
        Base* obj = NULL;

        try {
            obj = generate();
        }
        catch (std::bad_alloc& e) {
            std::cerr << e.what() << '\n';
            return 1;
        }

        std::cout << "------------------------------------\n";
        std::cout << "Test " << i + 1 << '\n';
        std::cout << "------------------------------------\n";
        std::cout << "Pointer identify: ";
        identify(obj);
        std::cout << "Reference identify: ";
        identify(*obj);

        delete obj;
    }

    return 0;
}
