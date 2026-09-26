#include "app_main.h"

int main(void)
{
    RVM_App_Init();

    for (;;)
    {
        RVM_App_Run();
    }
}
