
//
// FILETYPE.C
//

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int __cdecl wmain(int argc, wchar_t **argv)
{
    if (argc < 2)
    {
        printf("Usage: %ws binary\n", argv[0]);
    }

    // https://learn.microsoft.com/en-us/cpp/c-language/using-wmain?view=msvc-170

    IMAGE_FILE_MACHINES Machines;

    NTSTATUS Status = RtlGetImageFileMachines(argv[1], &Machines);

    printf("%ws supports machine types:", argv[1]);

    if (Machines.MachineX86)     printf("  X86");
    if (Machines.MachineAmd64)   printf("  X64/AMD64");
    if (Machines.MachineArm)     printf("  ARM32");
    if (Machines.MachineArm64)   printf("  ARM64");
    if (Machines.MachineArm64EC) printf("  ARM64EC");

    printf("\n");
}



