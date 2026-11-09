#include <stdio.h>
#include "mymemory.h"

int main(void)
{
    mymemory_t *memory = mymemory_init(1000);

    int *v = mymemory_alloc(memory, sizeof(int) * 32);
    for (int i = 0; i < 32; i++)
        v[i] = i + 1;

    int *v2 = mymemory_alloc(memory, sizeof(int) * 16);
    for (int i = 0; i < 16; i++)
        v2[i] = (i + 1) * 10;

    mymemory_display(memory);
    mymemory_stats(memory);

    mymemory_free(memory, v);

    char *str = mymemory_alloc(memory, sizeof(char) * 16);
    snprintf(str, 16, "ola mundo");
    printf("\nstr = \"%s\" (reaproveitou o espaco de v: %s)\n\n",
           str, (void *)str == (void *)v ? "sim" : "nao");

    mymemory_display(memory);
    mymemory_stats(memory);

    mymemory_cleanup(memory);
    return 0;
}
