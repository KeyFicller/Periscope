#include "periscope_printer.h"

#include <iostream>

class A
{
public:
    std::string to_string() const
    {
        return "A";
    }
};

int main()
{
    A a;
    std::cout << periscope::format_printer()("[0]-[1]-[0]", A(), &a) << std::endl;
}