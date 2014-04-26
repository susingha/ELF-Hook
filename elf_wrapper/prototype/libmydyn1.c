

int myfunc1() {
    return puts("Running myfunc1() from libmydyn1.c");
}


#if 1
int myfunc2() {
    return puts("Running myfunc2() from libmydyn1.c HEARTBLEED");
}
#endif
