
//
// CPUIDMAX-INDIRECT.C
//
// Call CPUID and display the highest CPUID function supported.
//
// 2024-01-24 darekm
//

#include <stdint.h>
#include <stdio.h>

extern uint32_t CallCpuid(uint32_t EAX, uint32_t ECX);
typedef uint32_t (AsmProc)(uint32_t, uint32_t);
AsmProc *pfn = CallCpuid;

int main()
{
    uint32_t Max = (*pfn)(0, 0);

    printf("CPUID function 0 returned max functions = %u\n", Max);
    return Max;
}



