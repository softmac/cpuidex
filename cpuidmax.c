
//
// CPUIDMAX.C
//
// Call CPUID and display the highest CPUID function supported.
//
// 2024-01-24 darekm
//

#include <stdint.h>
#include <stdio.h>

extern uint32_t CallCpuid(uint32_t EAX, uint32_t ECX);

int main()
{
    uint32_t Max = CallCpuid(0, 0);

    printf("CPUID function 0 returned max functions = %u\n", Max);
    return Max;
}



