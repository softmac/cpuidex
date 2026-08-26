
//
// IsEmulated.c
//
// Answers the question, am I running emulated on Windows on ARM?
// Using 3 different methods!
//

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

//
// Method #1
//
// This CPU detection code derived from:
// https://github.com/softmac/xformer10/blob/master/src/gemul8r.c
//
// Also see:
// https://learn.microsoft.com/en-us/windows/win32/api/wow64apiset/nf-wow64apiset-iswow64process2
//

const char *SzFromCpu(USHORT machine)
{
    switch (machine)
    {
        default:
        case IMAGE_FILE_MACHINE_UNKNOWN:     return "Unknown";
        case IMAGE_FILE_MACHINE_ARM64:       return "ARM64";
        case IMAGE_FILE_MACHINE_THUMB:       return "ARM";
        case IMAGE_FILE_MACHINE_ARMNT:       return "ARM";
        case IMAGE_FILE_MACHINE_AMD64:       return "X64";
        case IMAGE_FILE_MACHINE_I386:        return "X86";
        case IMAGE_FILE_MACHINE_TARGET_HOST: return "Same as host";
    }
}

// Returns true if guest!=host implying emulation and/or Wow64

bool IsEmulatedWow2()
{
#ifdef _M_AMD64
    USHORT guestCPU = IMAGE_FILE_MACHINE_AMD64;
    USHORT hostCPU  = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_IX86)
    USHORT guestCPU = IMAGE_FILE_MACHINE_I386;
    USHORT hostCPU  = IMAGE_FILE_MACHINE_I386;
#elif defined(_M_ARM)
    USHORT guestCPU = IMAGE_FILE_MACHINE_ARMNT;
    USHORT hostCPU  = IMAGE_FILE_MACHINE_ARMNT;
#elif defined(_M_ARM64)
    USHORT guestCPU = IMAGE_FILE_MACHINE_ARM64;
    USHORT hostCPU  = IMAGE_FILE_MACHINE_ARM64;
#endif

    USHORT Dummy;

    // IsWow64Process2 requires Windows 10 or later

    IsWow64Process2(GetCurrentProcess(), &Dummy, &hostCPU);

    printf("Process CPU architecture: %s\n", SzFromCpu(guestCPU));
    printf("Native  CPU architecture: %s\n", SzFromCpu(hostCPU));

    return (guestCPU != hostCPU);
}

#if defined(_M_AMD64) || defined(_M_IX86)

//
// Method #2
//
// CPUID based detection derived from:
// https://github.com/softmac/cpuidex/blob/main/cpuidex.c
//
// Relies on Prism failing to expose certain CPUID bits that real AMD and Intel hardware does
// and specific CPUID signatures that Prism exposes
//

typedef enum CPUID_REGS
{
    CPUID_EAX = 0,
    CPUID_EBX = 1,
    CPUID_ECX = 2,
    CPUID_EDX = 3,
} CPUID_REGS;

extern unsigned __int64 CallXgetbv(unsigned int ECX);

uint32_t LookUpReg(uint32_t Function, uint32_t Sub, CPUID_REGS Reg)
{
    int CpuInfo[4] = { };

    __cpuidex(CpuInfo, Function, Sub);

    return CpuInfo[Reg];
}

uint32_t LookUpRegBit(int Function, int Sub, CPUID_REGS Reg, int Bit)
{
    return (LookUpReg(Function, Sub, Reg) >> Bit) & 1;
}

bool HasVME()      { return LookUpRegBit(1, 0, CPUID_EDX,  1); }
bool HasDE()       { return LookUpRegBit(1, 0, CPUID_EDX,  2); }
bool HasPSE()      { return LookUpRegBit(1, 0, CPUID_EDX,  3); }
bool HasMSR()      { return LookUpRegBit(1, 0, CPUID_EDX,  5); }
bool HasPAE()      { return LookUpRegBit(1, 0, CPUID_EDX,  6); }
bool HasAPIC()     { return LookUpRegBit(1, 0, CPUID_EDX,  9); }
bool HasPAT()      { return LookUpRegBit(1, 0, CPUID_EDX, 16); }
bool HasHTT()      { return LookUpRegBit(1, 0, CPUID_EDX, 28); }

bool IsEmulatedCpuid()
{
    // look for missing feature which all modern AMD and Intel CPUs always expose

    return !(HasVME() && HasDE() && HasPSE() && HasMSR() && HasPAE() && HasAPIC && HasPAT() && HasHTT());
}

#endif

#if defined(_M_AMD64)

//
// Method #3
//
// IsProcessorFeaturePresent() based detection derived from:
// https://github.com/softmac/cpuidex/blob/main/IsProcFeatPresent/IsProcFeatPresent.c
//
// Relies on the fact that Prism exposes both the X64 and ARM64 features in emulated processes.
// This does not work with x86 emulation thus why just the ifdef _M_AMD64
//

bool IsEmulatedIsProc()
{
    return IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE) != FALSE;

}

#endif

int __cdecl main(int argc, char **argv)
{
    // Method #1
    printf("Am I emulated, IsWow2 method returns: %s\n", IsEmulatedWow2() ? "YES" : "NO");

    // Method #2
#if defined(_M_AMD64) || defined(_M_IX86)
    printf("Am I emulated, CPUID  method returns: %s\n", IsEmulatedCpuid() ? "YES" : "NO");
#endif

    // Method #3
#if defined(_M_AMD64)
    printf("Am I emulated, IsFeat method returns: %s\n", IsEmulatedIsProc() ? "YES" : "NO");
#endif

}


