
//
// CPUIDMAX.C
//
// Call CPUID and display the highest CPUID function supported.
//
// 2024-01-24 darekm
//

#include <stdint.h>
#include <stdio.h>
#include <intrin.h>

extern uint32_t CallCpuid(uint32_t EAX, uint32_t ECX);

int main()
{
    // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-170

    int CpuInfo[4];
    __cpuidex(CpuInfo, 0, 0);

    uint32_t Max = CpuInfo[0];

    printf("CPUID function 0 returned max functions = %u\n", Max);
    return Max;
}



