#include "stm32f10x.h"
#include "robot_control.h"

int main(void)
{
    Robot_Init();

    while (1)
    {
        Robot_Poll();
    }
}
