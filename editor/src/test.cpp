#include "test.h"

#include <iostream>

#include <moon.h>

int main()
{
    moon::log::init();
    MOON_TRACE("Test {0}", test(5));
    MOON_TRACE("Hello World!");
    return 0;
}
