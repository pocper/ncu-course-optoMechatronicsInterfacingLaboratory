#include "c4mlib.h"

int main()
{
    C4M_DEVICE_set();

    REGFPT(&DDRD, 0x10, 0, 0);
    char data;

    while (1)
    {
        REGFGT(&PIND, 0x10, 4, &data);
        printf("Sensor = %d\n", data);
        _delay_ms(100);
    }
    return 0;
}