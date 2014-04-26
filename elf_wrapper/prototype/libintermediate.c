#include <stdio.h>

int myfunc1();
int myfunc2();

int intermediate() {
    int i = 0;

    getchar();

    i = myfunc1();
    printf("myfunc1 returned: %d\n", i);
    i = myfunc2();
    printf("myfunc2 returned: %d\n", i);

    return i;
}
