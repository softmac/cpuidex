
//
// CPUIDEX.C
//
// Display common CPUID functions using __cpuidex intrinsic.
//
// Copyright (C) 2024 by Darek Mihocka.  All rights reserved.
//
// 2024-01-26 darekm
//

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <intrin.h>
#include <windows.h>

// https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-170

uint32_t BaseFunc = 0;
uint32_t BaseFuncHyp = 0x40000000;
uint32_t BaseFuncExt = 0x80000000;

uint32_t MaxFunc = 0;
uint32_t MaxFuncHyp = 0;
uint32_t MaxFuncExt = 0;

uint32_t Warnings = 0;

typedef enum CPU_VENDOR
{
    CPU_UNKNOWN = 0,
    CPU_AMD     = 1,  // AuthenticAMD
    CPU_INTEL   = 2,  // GenuineIntel
    CPU_TMTA    = 3,  // Transmeta
    CPU_VPC     = 4,  // Virtual CPU    (Microsoft XTA, or Connectix Virtual PC)
} CPU_VENDOR;

CPU_VENDOR CpuVendor = CPU_UNKNOWN;

typedef enum CPUID_REGS
{
    CPUID_EAX = 0,
    CPUID_EBX = 1,
    CPUID_ECX = 2,
    CPUID_EDX = 3,
} CPUID_REGS;

extern unsigned __int64 CallXgetbv(unsigned int ECX);

//
// Helper functions to probe CPUID by register, bit, bitfield, or string.
//

uint32_t LookUpReg(uint32_t Function, uint32_t Sub, CPUID_REGS Reg)
{
    int CpuInfo[4] = { };

    if ((Function > MaxFunc) && (Function < BaseFuncHyp))
    {
        printf("Function %08X out of range for basic functions limit %08X\n", Function, MaxFunc);
        return 0;
    }

    if (MaxFuncHyp && (Function > MaxFuncHyp) && (Function < BaseFuncExt))
    {
        printf("Function %08X out of range for hyper functions limit %08X\n", Function, MaxFuncHyp);
        return 0;
    }

    if (MaxFuncExt && (Function > MaxFuncExt))
    {
        printf("Function %08X out of range for extended functions limit %08X\n", Function, MaxFuncExt);
        return 0;
    }

    __cpuidex(CpuInfo, Function, Sub);

    return CpuInfo[Reg];
}

uint32_t LookUpRegBit(int Function, int Sub, CPUID_REGS Reg, int Bit)
{
    return (LookUpReg(Function, Sub, Reg) >> Bit) & 1;
}

uint32_t LookUpRegBits(int Function, int Sub, CPUID_REGS Reg, int Bit, int Width)
{
    return (LookUpReg(Function, Sub, Reg) >> Bit) & ((1 << Width) - 1);
}

const char * LookUpVendorString()
{
    int CpuInfo[4];
    __cpuidex(CpuInfo, 0, 0);

    static char sz[16] = { };
    *(int32_t *)(&sz[0]) = CpuInfo[CPUID_EBX];
    *(int32_t *)(&sz[4]) = CpuInfo[CPUID_EDX];
    *(int32_t *)(&sz[8]) = CpuInfo[CPUID_ECX];
    sz[12] = '\0';

    if (!strncmp("AuthenticAMD", sz, 12))
        CpuVendor = CPU_AMD;
    if (!strncmp("GenuineIntel", sz, 12))
        CpuVendor = CPU_INTEL;
    if (!strncmp("GenuineTMx86", sz, 12))
        CpuVendor = CPU_TMTA;
    if (!strncmp("Virtual CPU ", sz, 12))
        CpuVendor = CPU_VPC;

    return sz;
}

const char * LookUpModelString()
{
    static int CpuInfo[4][4];

    __cpuidex(&CpuInfo[0][0], 0x80000002, 0);
    __cpuidex(&CpuInfo[1][0], 0x80000003, 0);
    __cpuidex(&CpuInfo[2][0], 0x80000004, 0);

    return (char *)&CpuInfo[0];
}

//
// Helper functions for identifying common CPUID feature bits.
//
// See: https://www.felixcloutier.com/x86/cpuid
//

// These are the old school 1990's 486 and 32-bit Pentium bits:

bool HasX87()      { return LookUpRegBit(1, 0, CPUID_EDX,  0); }
bool HasVME()      { return LookUpRegBit(1, 0, CPUID_EDX,  1); }
bool HasDE()       { return LookUpRegBit(1, 0, CPUID_EDX,  2); }
bool HasPSE()      { return LookUpRegBit(1, 0, CPUID_EDX,  3); }
bool HasTSC()      { return LookUpRegBit(1, 0, CPUID_EDX,  4); }
bool HasMSR()      { return LookUpRegBit(1, 0, CPUID_EDX,  5); }
bool HasPAE()      { return LookUpRegBit(1, 0, CPUID_EDX,  6); }
bool HasCX8()      { return LookUpRegBit(1, 0, CPUID_EDX,  8); }
bool HasAPIC()     { return LookUpRegBit(1, 0, CPUID_EDX,  9); }
bool HasSEP()      { return LookUpRegBit(1, 0, CPUID_EDX, 11); }  // SYSENTER SYSEXIT
bool HasCMOV()     { return LookUpRegBit(1, 0, CPUID_EDX, 15); }
bool HasFCMOV()    { return HasX87() && HasCMOV();             }
bool HasPAT()      { return LookUpRegBit(1, 0, CPUID_EDX, 16); }
bool HasPSE36()    { return LookUpRegBit(1, 0, CPUID_ECX, 17); }
bool HasCLFLUSH()  { return LookUpRegBit(1, 0, CPUID_EDX, 19); }
bool HasACPI()     { return LookUpRegBit(1, 0, CPUID_EDX, 22); }
bool HasMMX()      { return LookUpRegBit(1, 0, CPUID_EDX, 23); }
bool HasFXSR()     { return LookUpRegBit(1, 0, CPUID_EDX, 24); }
bool HasSSE()      { return LookUpRegBit(1, 0, CPUID_EDX, 25); }
bool HasSSE2()     { return LookUpRegBit(1, 0, CPUID_EDX, 26); }
bool HasHTT()      { return LookUpRegBit(1, 0, CPUID_EDX, 28); }

// Windows Vista and Win7 era SSE/AVX Intel Core and AMD64 bits:

bool HasSSE3()     { return LookUpRegBit(1, 0, CPUID_ECX,  0); }
bool HasPCLMUL()   { return LookUpRegBit(1, 0, CPUID_ECX,  1); }
bool HasMONITOR()  { return LookUpRegBit(1, 0, CPUID_ECX,  3); }
bool HasVTX()      { return LookUpRegBit(1, 0, CPUID_ECX,  5); }
bool HasSMX()      { return LookUpRegBit(1, 0, CPUID_ECX,  6); }
bool HasEIST()     { return LookUpRegBit(1, 0, CPUID_ECX,  7); }  // enhanced SpeedStep
bool HasSSSE3()    { return LookUpRegBit(1, 0, CPUID_ECX,  9); }
bool HasFMA()      { return LookUpRegBit(1, 0, CPUID_ECX, 12); }
bool HasCX16()     { return LookUpRegBit(1, 0, CPUID_ECX, 13); }
bool HasSSE41()    { return LookUpRegBit(1, 0, CPUID_ECX, 19); }
bool HasSSE42()    { return LookUpRegBit(1, 0, CPUID_ECX, 20); }
bool HasMOVBE()    { return LookUpRegBit(1, 0, CPUID_ECX, 22); }
bool HasPOPCNT()   { return LookUpRegBit(1, 0, CPUID_ECX, 23); }
bool HasAES()      { return LookUpRegBit(1, 0, CPUID_ECX, 25); }
bool HasXSAVE()    { return LookUpRegBit(1, 0, CPUID_ECX, 26); }
bool HasOSXSAVE()  { return LookUpRegBit(1, 0, CPUID_ECX, 27); }
bool HasAVX()      { return LookUpRegBit(1, 0, CPUID_ECX, 28); }
bool HasF16C()     { return LookUpRegBit(1, 0, CPUID_ECX, 29); }
bool HasRDRAND()   { return LookUpRegBit(1, 0, CPUID_ECX, 30); }

bool HasLAHF64()   { return LookUpRegBit(0x80000001, 0, CPUID_ECX,  0); }
bool HasABM()      { return LookUpRegBit(0x80000001, 0, CPUID_ECX,  5); }
bool HasSSE4A()    { return LookUpRegBit(0x80000001, 0, CPUID_ECX,  6); }
bool Has3DPREF()   { return LookUpRegBit(0x80000001, 0, CPUID_ECX,  8); }
bool HasXOP()      { return LookUpRegBit(0x80000001, 0, CPUID_ECX, 11); }
bool HasLWP()      { return LookUpRegBit(0x80000001, 0, CPUID_ECX, 15); }
bool HasFMA4()     { return LookUpRegBit(0x80000001, 0, CPUID_ECX, 16); }
bool HasTBM()      { return LookUpRegBit(0x80000001, 0, CPUID_ECX, 21); }
bool HasMONITORX() { return LookUpRegBit(0x80000001, 0, CPUID_ECX, 29); }

bool HasRDTSCP()   { return LookUpRegBit(0x80000001, 0, CPUID_EDX, 27); }
bool HasTSCINV()   { return LookUpRegBit(0x80000007, 0, CPUID_EDX,  8); }

// Windows 11 modern AVX2+ Intel Core and AMD Zen bits:

bool HasFSGSBASE() { return LookUpRegBit(7, 0, CPUID_EBX,  0); }
bool HasBMI1()     { return LookUpRegBit(7, 0, CPUID_EBX,  3); }
bool HasHLE()      { return LookUpRegBit(7, 0, CPUID_EBX,  4); }
bool HasAVX2()     { return LookUpRegBit(7, 0, CPUID_EBX,  5); }
bool HasSMEP()     { return LookUpRegBit(7, 0, CPUID_EBX,  7); }
bool HasBMI2()     { return LookUpRegBit(7, 0, CPUID_EBX,  8); }
bool HasREPMOVSB() { return LookUpRegBit(7, 0, CPUID_EBX,  9); }
bool HasRTM()      { return LookUpRegBit(7, 0, CPUID_EBX, 11); }
bool HasDEPRFPU()  { return LookUpRegBit(7, 0, CPUID_EBX, 13); }
bool HasRDSEED()   { return LookUpRegBit(7, 0, CPUID_EBX, 18); }
bool HasADX()      { return LookUpRegBit(7, 0, CPUID_EBX, 19); }
bool HasRDPID()    { return LookUpRegBit(7, 0, CPUID_EBX, 22); }
bool HasCLFLSHOP() { return LookUpRegBit(7, 0, CPUID_EBX, 23); }
bool HasSHANI()    { return LookUpRegBit(7, 0, CPUID_EBX, 29); }

bool HasCETSS()    { return LookUpRegBit(7, 0, CPUID_ECX,  7); }
bool HasVAES()     { return LookUpRegBit(7, 0, CPUID_ECX,  9); }
bool HasVPCLMUL()  { return LookUpRegBit(7, 0, CPUID_ECX, 10); }

bool HasAVXVNNI()  { return LookUpRegBit(7, 1, CPUID_EAX,  4); }
bool HasAVX10()    { return LookUpRegBit(7, 1, CPUID_EAX, 19); }

bool HasXSAVEOPT() { return LookUpRegBit(13, 1, CPUID_EAX, 0); }
bool HasXSAVEC()   { return LookUpRegBit(13, 1, CPUID_EAX, 1); }
bool HasXGETBV()   { return LookUpRegBit(13, 1, CPUID_EAX, 2); }
bool HasXSAVES()   { return LookUpRegBit(13, 1, CPUID_EAX, 3); }

bool HasAVX512F()  { return LookUpRegBit(7, 0, CPUID_EBX, 16); }
bool HasAVX512CD() { return LookUpRegBit(7, 0, CPUID_EBX, 28); }
bool HasAVX512VNNI(){ return LookUpRegBit(7,0, CPUID_ECX, 11); }

#define ShowIsFeaturePresent(S,F) \
    printf ( "%s ", Has ## F () ? S : &"----------------"[16 - strlen(S)]);

#define WarnIfFeatureMissing(S,F) \
    if (0 == Has ## F ()) { printf ("Warning: feature %s is missing\n", S); Warnings++; }

//
// Return a string indicating the instruction set and mode of this process.
//

const char *GetGuestArchString()
{

#if _M_IX86

    static const char *SzX86 = "32-bit x86";
    return SzX86;

#elif _M_ARM64

    static char *SzA64 = "64-bit classic ARM64";
    return SzA64;

#elif _M_ARM64EC

    // check for ARM64EC before AMD64 because AMD64 will implicitly be defined here too.

    static char *SzAEC = "64-bit emulation compatible ARM64EC";
    return SzAEC;

#elif _M_AMD64

    static const char *SzX64 = "64-bit x64/AMD64";
    return SzX64;

#elif

#error Unknown ISA!  Let's hope it is RISC-V!

#endif

}

//
// Return a string indicating the actual instruction set of the host hardware or VM.
//

USHORT GuestCPU = 1;
USHORT HostCPU = 0;

const char *GetHostArchString()
{

    //
    // This code is copied from host ISA detection code in Xformer 10.
    // It uses a dynamic DLL load for compatibility with older Windows 7 systems
    // as the IsWow64Process2() function is only available in Windows 10 and later.
    //

    static BOOL (__stdcall *pfnGetWowProc2)(HANDLE, USHORT *, USHORT*) = NULL;
    HMODULE hMod;
    BOOL IsWow = 0;

    if (pfnGetWowProc2 == NULL)
    {
        hMod = LoadLibrary("kernel32.dll");
        pfnGetWowProc2 = (void *)GetProcAddress(hMod, "IsWow64Process2");
    }

    if (pfnGetWowProc2 && (*pfnGetWowProc2)(GetCurrentProcess(), &GuestCPU, &HostCPU))
    {
        // the new function successfully populated the GuestCPU and HostCPU variables.
    }
    else if (IsWow64Process(GetCurrentProcess(), &IsWow) && IsWow)
    {
        // the function failed, so fall back to the classic API and assume x64/AMD64 host.

        HostCPU = IMAGE_FILE_MACHINE_AMD64;
    }
    else
    {
        // IsWowProcess() failed, which indicates this process is not running under WOW.
        // That means one two things:
        // - the host kernel is a 32-bit Windows kernel (Windows XP, 7, 8.x, or 10)
        // - the host kernel is 64-bit and we are running the x64 build of this binary.

#if defined(_M_IX86)

        HostCPU = IMAGE_FILE_MACHINE_I386;

#elif defined(_M_AMD64)

        HostCPU = IMAGE_FILE_MACHINE_AMD64;

#endif

    }

    //
    // Let's see what got returned!  Possible outcomes are x86, x64, and ARM64.
    // Note that ARM64EC cannot be a host CPU architecture.
    // Ignore past NT platforms like Alpha, Itanium, PowerPC, they don't run Windows 11.
    //

    switch (HostCPU)
        {
    default:
        static const char *SzUnknown = "Unknown host ISA!! Hopefully RISC-V!!";
        printf("Unknown HostCPU %X\n", HostCPU);
        return SzUnknown;

    case IMAGE_FILE_MACHINE_I386:
        static const char *SzX86 = "32-bit x86";
        return SzX86;

    case IMAGE_FILE_MACHINE_AMD64:
        static const char *SzX64 = "64-bit x64/AMD64";
        return SzX64;

    case IMAGE_FILE_MACHINE_ARM64:
        static const char *SzA64 = "64-bit ARM64";
        return SzA64;
        }
}

int __cdecl main(int argc, char **argv)
{
    // Assume first function is available, then populate limits with actual lookups

    MaxFuncHyp = BaseFuncHyp;
    MaxFuncExt = BaseFuncExt;

    MaxFunc    = LookUpReg(BaseFunc,    0, CPUID_EAX);

    // hypervisor may not be present so verify that something valid got returned

    MaxFuncHyp = LookUpReg(BaseFuncHyp, 0, CPUID_EAX);
    bool HasHypervisor = (MaxFuncHyp >= BaseFuncHyp) && ((MaxFuncHyp - BaseFuncHyp) < 0x1000);

    MaxFuncHyp = HasHypervisor ? MaxFuncHyp : 0;

    // check for extended functions, which are primarily used and defined by AMD

    MaxFuncExt = LookUpReg(BaseFuncExt, 0, CPUID_EAX);
    bool HasExtFuncs   = (MaxFuncExt >= BaseFuncExt) && ((MaxFuncExt - BaseFuncExt) < 0x1000);

    MaxFuncExt = HasExtFuncs ? MaxFuncExt : 0;

    // now we can look up arbitrary functions
    // check if we're looking up a specific function

    if (argc > 1)
    {
        uint32_t Function = strtol(argv[1], NULL, 0);
        uint32_t SubFunc = (argc > 2) ? strtol(argv[2], NULL, 0) : 0;

        printf("Function %08X[%08X]: ", Function, SubFunc);
        printf("%08X %08X %08X %08X\n",
            LookUpReg(Function, SubFunc, CPUID_EAX),
            LookUpReg(Function, SubFunc, CPUID_EBX),
            LookUpReg(Function, SubFunc, CPUID_ECX),
            LookUpReg(Function, SubFunc, CPUID_EDX));

        return 0;
    }

    printf("\nCPUIDEX 1.07b - CPUID examination utility. October 2025 release.\n");
    printf("Developed by Darek Mihocka for emulators.com.\n");

    printf("\nRunning as a %s process on a %s host architecture.\n", GetGuestArchString(), GetHostArchString());

    printf("\nMaximum Intel function number = %08X\n", MaxFunc);
    printf("Maximum AMD64 function number = %08X\n", MaxFuncExt);

    printf("\nHypervisor functions %s present.\n", HasHypervisor ? "are" : "are not");

    if (HasHypervisor)
        printf("Maximum hyper function number = %08X\n", MaxFuncHyp);

    const uint32_t ProcessorSignature = LookUpReg(1, 0, CPUID_EAX);
    const uint32_t ProcessorFamily    = (ProcessorSignature >> 8) & 15;
    const uint32_t ProcessorExtFamily = ((ProcessorSignature >> 20) & 255) + ProcessorFamily;
    const uint32_t ProcessorModel     = (ProcessorSignature >> 4) & 15;
    const uint32_t ProcessorExtModel  = ((ProcessorSignature >> 12) & 0xF0) + ProcessorModel;
    const uint32_t ProcessorStepping  = ProcessorSignature & 15;

    printf("\n");
    printf("Processor signature  = %08X\n", ProcessorSignature);
    printf("Processor family     = %8u\n", ProcessorFamily);
    printf("Processor ext family = %8u\n", ProcessorExtFamily);
    printf("Processor model      = %8u\n", ProcessorModel);
    printf("Processor ext model  = %8u\n", ProcessorExtModel);
    printf("Processor stepping   = %8u\n", ProcessorStepping);
    printf("Processor vendor     = '%s'\n", LookUpVendorString());
    printf("Processor model      = '%s'\n", LookUpModelString());

    const uint32_t ProcessorEbxBits = LookUpReg(1, 0, CPUID_EBX);
    const uint32_t ProcessorEcxBits = LookUpReg(1, 0, CPUID_ECX);
    const uint32_t ProcessorEdxBits = LookUpReg(1, 0, CPUID_EDX);

    printf("Processor brand index= %8u\n", ProcessorEbxBits & 0xFF);
    printf("CLFLUSH line size    = %8u bytes\n", (ProcessorEbxBits >> 5) & 0x7F8);
    printf("Processor max IDs    = %8u\n", (ProcessorEbxBits >> 16) & 0xFF);

    printf("Basic features ECX   = %08X\n", ProcessorEcxBits);
    printf("Basic features EDX   = %08X\n", ProcessorEdxBits);


#if 0

    if (HostCPU == IMAGE_FILE_MACHINE_ARM64)
    {
        printf("Emulated CPU hosted on ARM64.\n");
    }

    else if (CpuVendor == CPU_VPC)
    {
        printf("Emulated CPU hosted by Microsoft's Virtual PC.\n");
    }

    else if ((ProcessorExtFamily == 15) && (CpuVendor == CPU_INTEL))
    {
        printf("Intel Pentium 4 or Intel Atom era CPU.\n");
    }

    else if ((ProcessorExtFamily == 6) && (CpuVendor == CPU_INTEL))
    {
        printf("Intel Core i3/i5/i7/i9 CPU.\n");
    }

    else if ((ProcessorExtFamily == 15) && (CpuVendor == CPU_AMD))
    {
        printf("This appears to be an older AMD CPU.\n");
    }

    else if ((ProcessorExtFamily >= 16) && (CpuVendor == CPU_AMD))
    {
        printf("This appears to be a newer AMD CPU.\n");
    }

    else if (CpuVendor == CPU_TMTA))
    {
        printf("Emulated CPU hosted on Transmeta.\n");
    }

    else
    {
        printf("Unabled to determine the host CPU architecture.\n");
    }

#endif

    printf("\nBasic 32-bit CPU features (first row should all be present on Windows 7):\n");

    ShowIsFeaturePresent("X87",     X87);
    ShowIsFeaturePresent("TSC",     TSC);
    ShowIsFeaturePresent("CMOV",    CMOV);
    ShowIsFeaturePresent("FCMOV",   FCMOV);
    ShowIsFeaturePresent("CX8",     CX8);
    ShowIsFeaturePresent("MMX",     MMX);
    ShowIsFeaturePresent("FXSAVE",  FXSR);
    ShowIsFeaturePresent("SSE",     SSE);
    ShowIsFeaturePresent("SSE2",    SSE2);
    ShowIsFeaturePresent("HTT",     HTT);
    ShowIsFeaturePresent("CLFLUSH", CLFLUSH);
    printf("\n");
    ShowIsFeaturePresent("SSE3",    SSE3);   // Core Duo was a 32-bit only CPU with SSE3
    ShowIsFeaturePresent("VME",     VME);
    ShowIsFeaturePresent("DE",      DE);
    ShowIsFeaturePresent("PSE",     PSE);
    ShowIsFeaturePresent("MSR",     MSR);
    ShowIsFeaturePresent("PAE",     PAE);
    ShowIsFeaturePresent("APIC",    APIC);
    ShowIsFeaturePresent("SEP",     SEP);
    ShowIsFeaturePresent("PAT",     PAT);
//  ShowIsFeaturePresent("PSE36",   PSE36);
//  ShowIsFeaturePresent("ACPI",    ACPI);
    printf("\n");

    printf("\nExtended 64-bit CPU features (first row should all be present on Windows 10):\n");

    ShowIsFeaturePresent("CX16",    CX16);
    ShowIsFeaturePresent("SSSE3",   SSSE3);
    ShowIsFeaturePresent("SSE41",   SSE41);
    ShowIsFeaturePresent("SSE42",   SSE42);
    ShowIsFeaturePresent("POPCNT",  POPCNT);
    ShowIsFeaturePresent("AES",     AES);
    ShowIsFeaturePresent("PCLMUL",  PCLMUL);
    ShowIsFeaturePresent("XSAVE",   XSAVE);
    ShowIsFeaturePresent("OSXSAVE", OSXSAVE);
    ShowIsFeaturePresent("RDTSCP",  RDTSCP);
    printf("\n");
    ShowIsFeaturePresent("MOVBE",   MOVBE);
    ShowIsFeaturePresent("MWAIT",   MONITOR);
    ShowIsFeaturePresent("EIST",    EIST);
    ShowIsFeaturePresent("VT-x",    VTX);
    ShowIsFeaturePresent("SMX",     SMX);
    ShowIsFeaturePresent("AVX",     AVX);
    ShowIsFeaturePresent("F16C",    F16C);
    ShowIsFeaturePresent("FMA",     FMA);
    ShowIsFeaturePresent("RDRAND",  RDRAND);
    ShowIsFeaturePresent("DEPRFPU", DEPRFPU);
    ShowIsFeaturePresent("TSCINV",  TSCINV);
    printf("\n");
    ShowIsFeaturePresent("LAHF64",  LAHF64);
    ShowIsFeaturePresent("ABM",     ABM);
    ShowIsFeaturePresent("SSE4A",   SSE4A);
    ShowIsFeaturePresent("PREFETCH",3DPREF);
    ShowIsFeaturePresent("XOP",     XOP);
    ShowIsFeaturePresent("LWP",     LWP);
    ShowIsFeaturePresent("FMA4",    FMA4);
    ShowIsFeaturePresent("TBM",     TBM);
    ShowIsFeaturePresent("MWAITX",  MONITORX);
    printf("\n");

    printf("\nModern features since 2013 (first row should all be present on Windows 11):\n");

    ShowIsFeaturePresent("FSGSBASE",FSGSBASE);
    ShowIsFeaturePresent("RDSEED",  RDSEED);
    ShowIsFeaturePresent("SMEP",    SMEP);
    ShowIsFeaturePresent("FASTSTR", REPMOVSB);
    ShowIsFeaturePresent("CLFLUSHOPT",CLFLSHOP);
    ShowIsFeaturePresent("XSAVEOPT",XSAVEOPT);
    ShowIsFeaturePresent("XSAVEC",  XSAVEC);
    ShowIsFeaturePresent("XGETBV",  XGETBV);
    ShowIsFeaturePresent("XSAVES",  XSAVES);
    printf("\n");
//  ShowIsFeaturePresent("RDPID",   RDPID);
    ShowIsFeaturePresent("BMI1",    BMI1);
    ShowIsFeaturePresent("BMI2",    BMI2);
    ShowIsFeaturePresent("AVX2",    AVX2);
    ShowIsFeaturePresent("ADX",     ADX);
    ShowIsFeaturePresent("HLE",     HLE);
    ShowIsFeaturePresent("RTM",     RTM);
    ShowIsFeaturePresent("CET_SS",  CETSS);
    ShowIsFeaturePresent("SHANI",   SHANI);
    ShowIsFeaturePresent("VAES",    VAES);
    ShowIsFeaturePresent("VPCLMUL", VPCLMUL);
    ShowIsFeaturePresent("AVXVNNI", AVXVNNI);
    printf("\n");

    ShowIsFeaturePresent("AVX512F", AVX512F);
    ShowIsFeaturePresent("AVX512CD",AVX512CD);
    ShowIsFeaturePresent("AVX512VNNI",AVX512VNNI);
    ShowIsFeaturePresent("AVX10",   AVX10);
    printf("\n");

    printf("\nChecking for possible missing features:\n");

    WarnIfFeatureMissing("CLFLUSH", CLFLUSH);
    WarnIfFeatureMissing("PAE",     PAE);

#if _M_AMD64
    WarnIfFeatureMissing("CX16",    CX16);
    WarnIfFeatureMissing("HTT",     HTT);     // all 64-bit CPUs are multi-core
    WarnIfFeatureMissing("RDTSCP",  RDTSCP);
#endif

    if (HasAES() || HasAVX())
    {
        // All CPUs with hardware AES support SSSE3 SSE4.2 and XSAVE
        WarnIfFeatureMissing("SSSE3",   SSSE3);
        WarnIfFeatureMissing("SSE41",   SSE41);
        WarnIfFeatureMissing("SSE42",   SSE42);
        WarnIfFeatureMissing("OSXSAVE", OSXSAVE);
        WarnIfFeatureMissing("XSAVE",   XSAVE);
        WarnIfFeatureMissing("TSCINV",  TSCINV);
    }

    if (HasAVX() || HasAVX2())
    {
        // All CPUs with hardware AVX/AVX2 should support XGETBV
        WarnIfFeatureMissing("OSXSAVE", OSXSAVE);
    }

    // Make sure the model string starts with real text
    if (' ' == LookUpModelString()[0])
    {
        printf("Warning: model string should not start with spaces\n");
        Warnings++;
    }

    if (HasXSAVE() && HasOSXSAVE())
    {
        printf("\nChecking for XGETBV constistency:\n");

        printf("xgetbv(0) intrinsic  = %08llX\n", _xgetbv(0));
        printf("xgetbv(0) asm func   = %08llX\n", CallXgetbv(0));

        if (_xgetbv(0) != CallXgetbv(0))
        {
            printf("Warning: XGETBV return value mismatch\n");
            Warnings++;
        }
    }

    printf("\nChecking for TSC constistency:\n");

    int tsc_tries_left = 0;

    for (tsc_tries_left = 1000000; tsc_tries_left > 0; tsc_tries_left--)
    {
        if (__rdtsc() == __rdtsc())
        {
            printf("Warning: RDTSC returned the same value, needs to be monotonically increasing\n");
            Warnings++;
            break;
        }

        uint32_t Proc1 = 0, Proc2 = 0;
        if (__rdtscp(&Proc1) == __rdtscp(&Proc2))
        {
            printf("Warning: RDTSCP returned the same value, needs to be monotonically increasing\n");
            Warnings++;
            break;
        }
    }

    if (tsc_tries_left == 0)
            printf("TSC consistency checks passed\n");

    if (Warnings == 0)
        printf("\nNo checks failed!\n");
    else
        printf("\n%u checks failed!\n", Warnings);

    return MaxFunc;
}


