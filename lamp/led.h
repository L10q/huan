#ifndef LED_H
#define LED_H

#include <fcntl.h>
#include <unistd.h>

class Led
{
public:
    Led(const char *path = "/sys/devices/platform/led/leds/led1/brightness");    //打开路径

    void on();
    void off();

private:
    int fd;
};

#endif // LED_H
