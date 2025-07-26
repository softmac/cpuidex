echo on

@rem Builds 64-bit versions of the IsProcFeatPresent test binaries for x64 and ARM64EC.
@rem Run the Visual Studio vcvars32.bat _or_ vcvars64.bat / vcvarsarm.bat scripts ahead of time.

@if "%VSCMD_ARG_TGT_ARCH%" == "" (
    echo Visual Studio build environment not initialized.
    echo Make sure to run vcvars32.bat vcvars64.bat or vcvarsamd64_arm64.bat
    goto end
    )

@if "%VSCMD_ARG_TGT_ARCH%" == "x64" (
    cl -Zi -W4 -FAsc -O2 -Oi -Ob2          IsProcFeatPresent.c -link -release -debug -incremental:no -out:IsProcFeatPresent_x64.exe  -nodefaultlib ucrt.lib vcruntime.lib libcmt.lib mincore.lib
    move /y IsProcFeatPresent.cod IsProcFeatPresent_x64.cod
    goto end
    )

@if "%VSCMD_ARG_TGT_ARCH%" == "x86" (
    cl -Zi -W4 -FAsc -O2 -Oi -Ob2          IsProcFeatPresent.c -link -release -debug -incremental:no -out:IsProcFeatPresent_x86.exe  -nodefaultlib ucrt.lib vcruntime.lib libcmt.lib mincore.lib msvcrt.lib
    move /y IsProcFeatPresent.cod IsProcFeatPresent_x86.cod
    goto end
    )

@if not "%VSCMD_ARG_TGT_ARCH%" == "arm64" (
    @echo Unknown target ISA!
    goto end
    )

cl -Zi -W4 -FAsc -O2 -Oi -Ob2          IsProcFeatPresent.c -link -release -debug -incremental:no -out:IsProcFeatPresent_aa64.exe -nodefaultlib ucrt.lib vcruntime.lib libcmt.lib mincore.lib
move /y IsProcFeatPresent.cod IsProcFeatPresent_aa64.cod

cl -Zi -W4 -FAsc -O2 -Oi -Ob2 -arm64EC IsProcFeatPresent.c -link -release -debug -incremental:no -out:IsProcFeatPresent_ec.exe   -nodefaultlib ucrt.lib vcruntime.lib libcmt.lib mincore.lib
move /y IsProcFeatPresent.cod IsProcFeatPresent_ec.cod

:end

