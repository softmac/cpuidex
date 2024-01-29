
//
// VA2.C
//
// Sample showing use of VirtualAlloc2() to allocate dynamic ARM64EC code pages
//
// As discussed on the tutorial page:
// http://www.emulators.com/docs/abc_exit_xta.htm
//
// Based on documentation at:
// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc2
//
// 2024-01-28 darekm
//

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// pull in the import lib which exports VirtualAlloc2()
#pragma comment(linker, "/defaultlib:onecore")

void *AllocateReadExecute(size_t numBytesToAllocate)
{
    return VirtualAlloc(NULL, numBytesToAllocate, MEM_COMMIT, PAGE_EXECUTE_READ);
}

void *AllocateReadExecuteEc(size_t numBytesToAllocate, bool IsEC)
{
    MEM_EXTENDED_PARAMETER Parameter = { 0 };
    Parameter.Type = MemExtendedParameterAttributeFlags;

#if _M_AMD64 || _M_ARM64EC
    Parameter.ULong64 = IsEC ? MEM_EXTENDED_PARAMETER_EC_CODE : 0;
#else
    IsEC = false;
#endif

    // Looks like we use MEM_COMMIT directly without a MEM_RESERVE and VirtualProtect() step
    ULONG allocationType = MEM_COMMIT; // ULONG allocationType = MEM_RESERVE;

    DWORD protection = PAGE_EXECUTE_READ | PAGE_TARGETS_INVALID;

    void *Address = VirtualAlloc2 (
        GetCurrentProcess(),
        NULL,
        numBytesToAllocate,
        allocationType,
        protection,
        &Parameter,
        1);

    return Address;
}

typedef uint32_t (PFN)(uint32_t);

#define ROUNDS (10)

uint32_t RunAndTimeFunction(PFN *pfn)
{
    DWORD StartTick = GetTickCount();
    uint32_t Total = 0;

    for (unsigned round = 0; round < ROUNDS; round++)
    {
        // Reset the total each round
        Total = 0;

        for (unsigned i = 0; i < 3000000; i++)
        {
            Total = (*pfn)(Total);
        //  FlushInstructionCache(GetCurrentProcess(), pfn, 4096);
        }
    } 

    printf("%5u milliseconds elapsed per round\n", (GetTickCount() - StartTick) / ROUNDS);
    printf("final result total = %u\n", Total);

    return Total;
}

int __cdecl main(int argc, char **argv)
{
    SetLastError(0);
    void *AddressRXEC= AllocateReadExecuteEc(8*64*1024, true);
    printf("GetLastError = %X %u\n", GetLastError(), GetLastError());
    printf("Allocated EC code RX address %p\n", AddressRXEC);

    SetLastError(0);
    void *AddressRX2= AllocateReadExecuteEc(8*64*1024, false);
    printf("GetLastError = %X %u\n", GetLastError(), GetLastError());
    printf("Allocated native  RX address %p\n", AddressRX2);

    SetLastError(0);
    void *AddressRX1 = AllocateReadExecute(8*64*1024);
    printf("GetLastError = %X %u\n", GetLastError(), GetLastError());
    printf("Allocated classic RX address %p\n", AddressRX1);

    if (AddressRXEC)
    {
        DWORD OldProtect = 0;
        SetLastError(0);
        BOOL Status = VirtualProtect(AddressRXEC, 8*64*1024, PAGE_EXECUTE_READWRITE, &OldProtect);

        printf("GetLastError = %X %u\n", GetLastError(), GetLastError());
        printf("VirtualProtect returned %d\n", Status);

        if (Status != 0) *(uint32_t *)AddressRXEC = 0x12345678;

        printf("EC code page start with the value %08X\n", *(uint32_t *)AddressRXEC);
    }

    if (AddressRX2)
    {
        DWORD OldProtect = 0;
        SetLastError(0);
        BOOL Status = VirtualProtect(AddressRX2, 8*64*1024, PAGE_EXECUTE_READWRITE, &OldProtect);

        printf("GetLastError = %X %u\n", GetLastError(), GetLastError());
        printf("VirtualProtect returned %d\n", Status);

        if (Status != 0) *(uint32_t *)AddressRX2 = 0x12345678;
    }

    if (AddressRX1)
    {
        DWORD OldProtect = 0;
        SetLastError(0);
        BOOL Status = VirtualProtect(AddressRX1, 8*64*1024, PAGE_EXECUTE_READWRITE, &OldProtect);

        printf("GetLastError = %X %u\n", GetLastError(), GetLastError());
        printf("VirtualProtect returned %d\n", Status);

        if (Status != 0) *(uint32_t *)AddressRX1 = 0x12345678;
    }

#if _M_AMD64 || _M_ARM64EC

    if (AddressRX2 && AddressRXEC)
    {

        // both X64 and ARM64EC code pages successfully allocated
        // JIT some x64 code first

        uint8_t *X64Code = (uint8_t *)AddressRX2;

        int Option = (argc > 1) ? argv[1][0] & 15 : 0;

        switch (Option)
            {

        default:
        case 0:

            // emit x64 ADD + RET

            X64Code[0] = 0x8B; X64Code[1] = 0xC1;                     // MOV EAX,ECX
            X64Code[2] = 0x83; X64Code[3] = 0xC0; X64Code[4] = 0x01;  // ADD EAX,1
            X64Code[5] = 0xC3;                                        // RET
            break;

        case 1:

            // emit x64 ADD + JNO + RET

            X64Code[0] = 0x8B; X64Code[1] = 0xC1;                     // MOV EAX,ECX
            X64Code[2] = 0x83; X64Code[3] = 0xC0; X64Code[4] = 0x01;  // ADD EAX,1
            X64Code[5] = 0x71; X64Code[6] = 0xFB;                     // JNO (to the ADD)
            X64Code[7] = 0xC3;                                        // RET
            break;

        case 2:

            // emit x64 ADD + JNO + RET

            X64Code[0] = 0x8B; X64Code[1] = 0xC1;                     // MOV EAX,ECX
            X64Code[2] = 0x66;
            X64Code[3] = 0x83; X64Code[4] = 0xC0; X64Code[5] = 0x01;  // ADD AX,1
            X64Code[6] = 0x71; X64Code[7] = 0xFA;                     // JNO (to the ADD)
            X64Code[8] = 0xC3;                                        // RET
            break;

        case 3:

            // emit x64 INC + JNO + RET

            X64Code[0] = 0x8B; X64Code[1] = 0xC1;                     // MOV EAX,ECX
            X64Code[2] = 0x66;
            X64Code[3] = 0xFF; X64Code[4] = 0xC0;                     // INC AX
            X64Code[5] = 0x71; X64Code[6] = 0xFB;                     // JNO (to the ADD)
            X64Code[7] = 0xC3;                                        // RET
            break;

        case 5:

            // emit x64 SUB + JNO + RET

            X64Code[0] = 0x8B; X64Code[1] = 0xC1;                     // MOV EAX,ECX
            X64Code[2] = 0x83; X64Code[3] = 0xE8; X64Code[4] = 0x01;  // SUB EAX,1
            X64Code[5] = 0x71; X64Code[6] = 0xFB;                     // JNO (to the ADD)
            X64Code[7] = 0xC3;                                        // RET
            break;

        case 6:

            // emit x64 SUB + JNO + RET

            X64Code[0] = 0x8B; X64Code[1] = 0xC1;                     // MOV EAX,ECX
            X64Code[2] = 0x66;
            X64Code[3] = 0x83; X64Code[4] = 0xE8; X64Code[5] = 0x01;  // SUB AX,1
            X64Code[6] = 0x71; X64Code[7] = 0xFA;                     // JNO (to the ADD)
            X64Code[8] = 0xC3;                                        // RET
            break;

        case 7:

            // emit x64 DEC + JNO + RET

            X64Code[0] = 0x8B; X64Code[1] = 0xC1;                     // MOV EAX,ECX
            X64Code[2] = 0x66;
            X64Code[3] = 0xFF; X64Code[4] = 0xC8;                     // DEC AX
            X64Code[5] = 0x71; X64Code[6] = 0xFB;                     // JNO (to the ADD)
            X64Code[7] = 0xC3;                                        // RET
            break;

            }

        printf("emitted option %X, x64 buffer is at %p\n\n", Option, X64Code);
        printf("    echo !address; q | cdb -pv -pn va2.exe | findstr EXECUTE_READWRITE\n");
        printf("    echo .effmach amd64 ; u %p ; q | cdb -pv -pn va2.exe\n", X64Code);
        printf("    cdb -pv -pn va2.exe\n", X64Code);

        // Write-protect and flush the x64 JIT buffer

        DWORD OldProtect = 0;
        VirtualProtect(X64Code, 4096, PAGE_EXECUTE_READ, &OldProtect);
        FlushInstructionCache(GetCurrentProcess(), X64Code, 4096);

        // native ARM64 portion

        uint32_t *A64Code = (uint32_t *)AddressRXEC;

        // emit the ARM64 ADD + RET

        A64Code[0] = 0x11000400;                                  // ADD W0,W0,#1
        A64Code[1] = 0xD65F03C0;                                  // RET

        // Write-protect and flush the ARM64 JIT buffer

        VirtualProtect(A64Code, 4096, PAGE_EXECUTE_READ, &OldProtect);
        FlushInstructionCache(GetCurrentProcess(), A64Code, 4096);

        PFN *pfnX = (PFN *)(void *)X64Code;
        PFN *pfnA = (PFN *)(void *)A64Code;

        RunAndTimeFunction(pfnX);
        RunAndTimeFunction(pfnA);

    }

#endif

}

