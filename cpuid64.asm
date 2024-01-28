
;
;  CPUID64.ASM
;
;  64-bit test code for CPUID
;
;  2024-01-24   darekm
;

    .RADIX  16t
    OPTION  SCOPED
    OPTION  CASEMAP:NOTPUBLIC

    .CODE

CallCpuid PROC
    mov     eax, ecx
    mov     ecx, edx
    cpuid
    ret
CallCpuid ENDP

    END

