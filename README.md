# libsrc

Common C and C++ source files used by LTRData's native Windows code. This collection contains implementations for support routines and classes declared in [LTRData/include](https://github.com/LTRData/include), along with older compatibility code and experiments.

The repository contains 58 source files. Its Git history begins with a repository conversion in May 2024; the code includes substantially older designs and compatibility paths, including a Windows 95 workaround. The conversion date is not the original development date.

## Contents

| Area | Representative files and purpose |
| --- | --- |
| Errors and logging | [waerror.c](waerror.c), [wwerror.c](wwerror.c), the `*perror.c` files, [wapdherror.c](wapdherror.c), and `*errlog.cpp` / [wmsglog.cpp](wmsglog.cpp): Windows, NTSTATUS, socket and PDH error text, printing, and timestamped logging. |
| Console and formatting | [wconsole.c](wconsole.c), [wconsole.cpp](wconsole.cpp), [wconwin.c](wconwin.c), [wconmsg.cpp](wconmsg.cpp), [wconmsgw.cpp](wconmsgw.cpp), [woemprnf.c](woemprnf.c), and `wmprintf*.c`: console operations, message prompts, and allocated/formatted output. |
| Strings and character conversion | [wstring.c](wstring.c), [wmemstr.c](wmemstr.c), [wmemwcs.c](wmemwcs.c), `*towide.c`, `wwideto*.c`, and [wmsgoem.cpp](wmsgoem.cpp): trimming, searching, and ANSI/OEM/wide-character conversion. |
| Stream I/O | [wio.cpp](wio.cpp) and `wol*.cpp`: completion routines, buffered and line-oriented I/O through `WOverlapped` and `WOverlappedIOC`, and command-line input. [wreadpwd.c](wreadpwd.c) and [wreadpwdw.c](wreadpwdw.c) provide password input helpers. |
| Networking | [wsocket.cpp](wsocket.cpp): IPv4 TCP connection helpers. [tcproute.cpp](tcproute.cpp): HTTP CONNECT and SOCKS proxy connection routines. |
| Native files and processes | [ntfileio.cpp](ntfileio.cpp): native file/directory creation with optional parent-directory creation. [loadremotelib.cpp](loadremotelib.cpp): DLL loading in another process through a remote thread. |
| Privileges and scheduling | [waenpriv.c](waenpriv.c) / [wwenpriv.c](wwenpriv.c): token privilege helpers. [wthread.c](wthread.c), [wmsleep.c](wmsleep.c), and [spsleep.cpp](spsleep.cpp): thread creation, sleep, and x86 processor-yield support. |
| Mapping compatibility | [wmmap.c](wmmap.c) and [wmsync.c](wmsync.c): Windows-backed implementations of `mmap`, `munmap`, and `msync`, with a limited set of supported flags. |
| Other support and experiments | [wtime.c](wtime.c) / [wtimeflt.c](wtimeflt.c): date, time, and duration output; [wimath.c](wimath.c): integer arithmetic helpers; [winstrct.cpp](winstrct.cpp): older wrapper-object definitions; [wcslen.c](wcslen.c): x86 assembly implementation with a test entry point. |

## Building and integration

There is no checked-in solution, project, makefile, package definition, or automated test suite. Build integration belongs to the consuming project or support-library build.

- Obtain the corresponding headers from [LTRData/include](https://github.com/LTRData/include) and add them to the compiler's include path. Common dependencies include `winstrct.h`, `wio.h`, `wsocket.hpp`, `ntfileio.hpp`, and `ntdll.h`.
- Select the required source files and their dependencies explicitly. The collection includes legacy alternatives and experiments; in particular, `wcslen.c` contains its own `wmain` and unguarded x86 inline assembly.
- Use a Windows C/C++ toolchain and SDK compatible with those files, with the appropriate Windows/CRT import libraries. Some code uses MSVC-specific extensions or architecture conditions. The POSIX-named mapping and sleep functions are implemented using Windows APIs.
- Check the shared headers' library directives. `winstrct.h` normally requests `winstrct.lib` and `winstrcp.lib`; defining `NO_WINSTRCT_LIB_IMPORT` suppresses those requests when supplying the implementations another way, but does not supply missing symbols. `wcslen.c` also requests `minwcrt` when `_DLL` is defined.
- Match source and header declarations, character-set settings, and target architecture for the chosen build. The current repository does not provide a configuration that compiles every file together.

[LTRData/props](https://github.com/LTRData/props) contains related Visual C++ property sheets for shared include/library paths and legacy build environments. Those sheets do not define a build for this source collection or provide compiled libraries.

## Scope and limitations

The files preserve different generations of Windows support code. For example, `wsocket.cpp` uses IPv4 and older name-resolution APIs, `spsleep.cpp` has an x86-only implementation, and `msync` rejects asynchronous and invalidate flags. Select and verify the routines needed by the consuming application; the collection has no repository-wide platform or compiler compatibility matrix.

## Licensing

This repository currently has no root license file or explicit license notices in its source files. Contact the maintainer to clarify licensing for reuse. The companion header repository contains separate notices that should be checked for the headers used.
