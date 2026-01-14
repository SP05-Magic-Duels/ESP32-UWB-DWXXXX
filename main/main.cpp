#include <stdio.h>
#include "DW1000.h"

#include <cmath>

extern "C"
{
    void app_main(void)
    {
        std::printf("%lf", floor(10.2));
    }
}