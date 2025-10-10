
@rem Builds 32-bit versions of the CPUID test binaries for x86.
@rem Builds 64-bit versions of the CPUID test binaries for x64 and ARM64EC.
@rem Run the Visual Studio vcvars32.bat _or_ vcvars64.bat / vcvarsarm.bat scripts ahead of time.

@if "%VSCMD_ARG_TGT_ARCH%" == "" (
    echo Visual Studio build environment not initialized.
    echo Make sure to run vcvars32.bat vcvars64.bat or vcvarsamd64_arm64.bat
    goto end
    )

@if "%VSCMD_ARG_TGT_ARCH%" == "x64" (
  @ml64.exe 1>nul 2>nul
  echo %errorlevel%

  @if not "%errorlevel%" == "0" (
    echo 64-bit assembler not found.
    echo Make sure to run vcvars64.bat
    goto end
    )

  @rem assemble the ASM file
  ml64 -c -Zd -Zi -D_X64_ cpuid64.asm
  )

@if "%VSCMD_ARG_TGT_ARCH%" == "x86" (
  @ml.exe 1>nul 2>nul
  echo %errorlevel%

  @if not "%errorlevel%" == "0" (
    echo 32-bit assembler not found.
    echo Make sure to run vcvars32.bat
    goto end
    )

  @rem assemble the ASM file
  ml -c -Zd -Zi -D_X86_ cpuid64.asm
  )

@if not exist cpuid64.obj (
  echo ASM failed.
  @if "%VSCMD_ARG_TGT_ARCH%" == "arm64" (
    echo Make sure to run the x64 build first!
    )
  goto end
  )

@setlocal

@rem maximum warnings, debug symbols, compile for size, generate listings.
@rem for better debugging emit frame pointers and spill homeparams.
@rem link against the static C runtime library for better backward compat.
set CL_ARGS=-W4 -Zi -FAsc -O1 -Oy- -MT

@if "%VSCMD_ARG_TGT_ARCH%" == "x64" (
    set CL_ARGS=%CL_ARGS% -homeparams
    )

@if "%VSCMD_ARG_TGT_ARCH%" == "arm64" (
    set CL_ARGS=%CL_ARGS% -arm64EC
    )

cl -c %CL_ARGS% cpuidex.c
cl -c %CL_ARGS% cpuidmax.c
cl -c %CL_ARGS% cpuidmax-indirect.c
cl -c %CL_ARGS% cpuidmax-intrin.c

@rem maximum debug information, remove dead code
set LINK_ARGS=-debug -release -opt:ref -incremental:no

@rem x64 supports emitting unwind information for all functions.
@rem This is useful for code discovery for offline x64-to-ARM64 binary rewriting.
@if "%VSCMD_ARG_TGT_ARCH%" == "x64" (
    set OUTFILE=-out:CpuidEx_x64.exe
    set LINK_ARGS=%LINK_ARGS% -allpdata
    )

@if "%VSCMD_ARG_TGT_ARCH%" == "x86" (
    set OUTFILE=-out:CpuidEx_x86.exe
    )

@rem force an ARM64X binary format
@if "%VSCMD_ARG_TGT_ARCH%" == "arm64" (
    set OUTFILE=-out:CpuidEx_a64.exe
    set LINK_ARGS=%LINK_ARGS% -machine:arm64ec
    )

@rem the libraries that are generally needed for command line (using UCRT not MSVCRT)
set LINK_LIBS=libucrt.lib onecore.lib kernel32.lib user32.lib ntdll.lib softintrin.lib

@if "%VSCMD_ARG_TGT_ARCH%" == "x64" (
    set LINK_LIBS=%LINK_LIBS% cpuid64.obj
    )

@if "%VSCMD_ARG_TGT_ARCH%" == "x86" (
    set LINK_LIBS=%LINK_LIBS% cpuid64.obj
    )

@if "%VSCMD_ARG_TGT_ARCH%" == "arm64" (
    set LINK_LIBS=%LINK_LIBS% cpuid64.obj
    )

link %LINK_ARGS% cpuidex.obj %OUTFILE% %LINK_LIBS%

@if "%VSCMD_ARG_TGT_ARCH%" == "x64" (
link %LINK_ARGS% cpuidmax.obj          %LINK_LIBS%
link %LINK_ARGS% cpuidmax-indirect.obj %LINK_LIBS%
)

link %LINK_ARGS% cpuidmax-intrin.obj   %LINK_LIBS%

@endlocal

:end

