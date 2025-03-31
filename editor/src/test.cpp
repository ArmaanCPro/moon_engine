#include <iostream>
#include <core/log.h>
#include <test.h>

int main()
{
    moon::log::init();
    MOON_TRACE("Test {0}", test(5));
    MOON_TRACE("Hello World!");
    return 0;
}
