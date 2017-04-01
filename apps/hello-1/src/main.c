/*
 * Copyright 2015, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

/*
 * seL4 tutorial part 1:  simple printf
 */


#include <stdio.h>

#include <utils/zf_log.h>
#include <sel4utils/sel4_zf_logif.h>

/* TODO 1: add a main function to print a message */
/* hint: printf */
int main(int argc, char* argv[])
{
    *(char*) 0x0 = 'X';
    printf("Hello seL4 microkernel\n");
    printf("Hello seL4 microkernel\n");
    printf("Hello seL4 microkernel\n");

    return 0;
}
