
;
;  CPUID64.ASM
;
;  32-/64-bit test code for CPUID
;
;  2024-01-24   darekm
;  2025-07-26   darekm
;

    .RADIX  16t
    IFDEF _X86_
    .MODEL  FLAT,C
    ENDIF
    OPTION  SCOPED
    OPTION  CASEMAP:NOTPUBLIC

    .CODE

    align   4   ;; required for ARM64EC compatibility

CallCpuid PROC C
    mov     eax, ecx
    mov     ecx, edx
    cpuid
    ret
CallCpuid ENDP

    align   4   ;; required for ARM64EC compatibility

CallXgetbv PROC C
    IFDEF _X86_
    mov   ecx,dword ptr [esp+4]
    ENDIF
    xgetbv
    ret
CallXgetbv ENDP

    END

