#include <stdio.h>

int myfunc1();

int intermediate() {
    int i = 0;

    getchar();

    i = myfunc1();
    printf("myfunc1 returned: %d\n", i);

    return i;
}
