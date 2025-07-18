#include "led.h"

Led::Led(const char *path)
{
    fd = open(path,O_WRONLY);
}

void Led::on()
{
    write(fd,"1",1);
}

void Led::off()
{
    write(fd,"0",1);
}
