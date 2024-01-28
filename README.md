# cpuidex
CPUID demonstration program which also probes for defects in x86/x64 emulation

Contains code samples for "ARM64 Boot Camp" tutorials on www.emulators.com
specifically the tutorial on emulation at:

  http://www.emulators.com/docs/abc_exit_xta.htm

To build, install the latest Visual Studio 2022 17.8 and make sure to install
the ARM64/ARM64EC build tools in order to builds binaries for Windows on ARM.

To build 32-bit x86 version:
  - open a command prompt and type 'vcvars32.bat' to open x86 build window
  - run 'makeall.bat' to build the 4 demo binaries as x86
  - run CPUIDEX.EXE with no arguments to see the CPUID information in 32-bit mode
  - on Windows on ARM devices this will be emulated

To build 64-bit x64/AMD64 version:
  - open a command prompt and type 'vcvars64.bat' to open x64 build window
  - run 'makeall.bat' to build the 4 demo binaries as x64
  - run CPUIDEX.EXE with no arguments to see the CPUID information in 64-bit mode
  - on Windows on ARM devices this will be emulated

To build 64-bit ARM64EC version:
  - run the x64 build above ahead of time, do not delete the temporary .OBJs
  - open a command prompt and type 'vcvarsamd64_arm64.bat' to open ARM64 build window
  - run 'makeall.bat' to build the 4 demo binaries as ARM64EC
  - run CPUIDEX.EXE with no arguments to see the CPUID information as emulated
  - this build will only work on ARM64 devices

