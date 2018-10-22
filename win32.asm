;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2018. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE GUI 4.0
entry start

include 'win32a.inc'
include '../../string.inc'
include 'cpuid.inc'

ID_STATIC_VENDOR_TITLE = 100
ID_STATIC_VENDOR_VALUE = 101
ID_STATIC_BRAND_TITLE = 102
ID_STATIC_BRAND_VALUE = 103
ID_STATIC_CODE_TITLE = 104
ID_STATIC_CODE_VALUE = 105

section '.text' code readable executable

start:
        ; bind to first core
        invoke  GetCurrentProcess
        invoke  SetProcessAffinityMask, eax, 1
        invoke  Sleep, 0

        ; initialize heap
        invoke  GetProcessHeap
        test    eax, eax
        jz      exit
        mov     [cur_heap], eax
        ; obtain command line arguments
        invoke  GetCommandLineW
        invoke  CommandLineToArgvW, eax, argc
        test    eax, eax
        jz      exit
        mov     [argv], eax
        mov     eax, [argc]
        cmp     eax, 2
        jz      from_file
        cmp     eax, 3
        jz      to_file
        xor     eax, eax
        mov     [dump], eax
        jmp     dlg

from_file:
        mov     eax, [argv]
        mov     eax, [eax + 4]
        stdcall DumpFromFile, eax
        test    eax, eax
        jnz     exit
        jmp     dlg

to_file:
        mov     esi, [argv]
        mov     esi, [esi + 4]
        lodsd
        cmp     eax, 0x0064002D ; utf-16 "-d"
        jz      @f
        cmp     eax, 0x0044002D ; utf-16 "-D"
        jz      @f
        jmp     exit
@@:
        mov     esi, [argv]
        mov     esi, [esi + 8]
        stdcall DumpToFile, esi
        jmp     exit

dlg:
        lea     eax, [ci]
        stdcall CpuidInit, eax, [dump]
        invoke  GetModuleHandle, 0
        invoke  DialogBoxParam, eax, 37, HWND_DESKTOP, DialogProc, 0
        or      eax, eax
        jz      exit

exit:
        invoke  ExitProcess, 0

proc DumpToFile file:DWORD
        ; how much memory need
        stdcall CpuidDump, 0, 0
        ; allocate it
        invoke  HeapAlloc, [cur_heap], 0, eax
        test    eax, eax
        jz      .return
        mov     [dump], eax
        stdcall CpuidDump, [dump], 0
        mov     [dump_size], eax

        ; open file for dump
        mov     eax, [file]
        invoke  CreateFileW, eax, GENERIC_WRITE, 0, NULL, \
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
        cmp     eax, INVALID_HANDLE_VALUE
        jz      .free
        mov     [dump_handle], eax

        push    esi edi
        mov     esi, [dump]
  @@:
        stdcall DumpToStr, esi, output
        mov     esi, eax
        invoke  lstrlen, output
        invoke  WriteFile, [dump_handle], output, eax, dwtemp, NULL
        mov     edi, [dump]
        add     edi, [dump_size]
        cmp     esi, edi
        jl      @b
        pop     edi esi
        invoke  CloseHandle, [dump_handle]

.free:
        invoke  HeapFree, [cur_heap], 0, [dump]
.return:
        ret
endp

proc DumpFromFile file:DWORD
locals
        hfile   dd ?
        fsize   dd ?
        buf     dd ?
        rdbytes dd ?
endl
        ; open file and read it contents
        invoke  CreateFileW, [file], GENERIC_READ, FILE_SHARE_READ, NULL, \
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
        cmp     eax, INVALID_HANDLE_VALUE
        jz      .err
        mov     [hfile], eax

        invoke  GetFileSize, eax, 0
        cmp     eax, -1
        jz      .close
        mov     [fsize], eax

        invoke  HeapAlloc, [cur_heap], 0, eax
        test    eax, eax
        jz      .close
        mov     [buf], eax

        lea     eax, [fsize]
        invoke  ReadFile, [hfile], [buf], [fsize], eax, 0
        test    eax, eax
        jz      .free

        stdcall DumpParse, [buf], 0
        test    eax, eax
        jz      .free

        mov     ecx, 24
        mul     ecx
        invoke  HeapAlloc, [cur_heap], 0, eax
        test    eax, eax
        jz      .free

        mov     [dump], eax
        stdcall DumpParse, [buf], [dump]

        invoke  HeapFree, [cur_heap], 0, [buf]
        invoke  CloseHandle, [hfile]
        xor     eax, eax
        ret

.free:
        invoke  HeapFree, [cur_heap], 0, [buf]
.close:
        invoke  CloseHandle, [hfile]
.err:
        or      eax, -1
        ret
endp

proc DialogProc hwnddlg, msg, wparam, lparam
        push    ebx esi edi
        cmp     [msg], WM_INITDIALOG
        je      .wminitdialog
        cmp     [msg], WM_COMMAND
        je      .wmcommand
        cmp     [msg], WM_CLOSE
        je      .wmclose
        xor     eax, eax
        jmp     .finish

  .wminitdialog:
        invoke  SetWindowTextA, [hwnddlg], winmain_title
        invoke  SetDlgItemText, [hwnddlg], ID_STATIC_VENDOR_TITLE, vendor_title
        lea     eax, [ci + cpuid_info.name]
        invoke  SetDlgItemText, [hwnddlg], ID_STATIC_VENDOR_VALUE, eax
        invoke  SetDlgItemText, [hwnddlg], ID_STATIC_BRAND_TITLE, brand_title
        lea     eax, [ci + cpuid_info.brand]
        invoke  SetDlgItemText, [hwnddlg], ID_STATIC_BRAND_VALUE, eax
        invoke  SetDlgItemText, [hwnddlg], ID_STATIC_CODE_TITLE, codename_title
        mov     eax, [ci + cpuid_info.codename]
        invoke  SetDlgItemText, [hwnddlg], ID_STATIC_CODE_VALUE, eax

        jmp     .processed

.wmcommand:
        jmp     .processed

.wmclose:
        mov     eax, [dump]
        test    eax, eax
        jz      @f
        invoke  HeapFree, [cur_heap], 0, [dump]
@@:
        invoke  EndDialog, [hwnddlg], 0

.processed:
        mov     eax, 1

.finish:
        pop     edi esi ebx
        ret
endp

section '.bss' readable writeable
  ci cpuid_info
  cur_heap dd ?
  dump dd ?
  dump_size dd ?
  dump_handle dd ?
  dwtemp dd ?
  output rb 128
  argc dd ?
  argv dd ?

section '.idata' import data readable writeable

  library kernel32, 'kernel32.dll', \
          user32, 'user32.dll', \
          shell32, 'shell32.dll'

  include 'api\kernel32.inc'
  include 'api\user32.inc'

  import shell32, \
         CommandLineToArgvW, 'CommandLineToArgvW'

  include 'common.inc'
  include 'intel.inc'
  include 'amd.inc'

section '.rsrc' resource data readable

  directory RT_DIALOG, dialogs

  resource dialogs, 37, LANG_ENGLISH + SUBLANG_DEFAULT, maindlg

  dialog maindlg, '', 70, 70, 190, 175, WS_CAPTION + WS_POPUP + WS_SYSMENU + DS_MODALFRAME
    dialogitem 'STATIC', '', ID_STATIC_VENDOR_TITLE, 10, 10, 50, 8, WS_VISIBLE
    dialogitem 'STATIC', '', ID_STATIC_VENDOR_VALUE, 50, 10, 70, 8, WS_VISIBLE
    dialogitem 'STATIC', '', ID_STATIC_BRAND_TITLE, 10, 24, 50, 8, WS_VISIBLE
    dialogitem 'STATIC', '', ID_STATIC_BRAND_VALUE, 50, 24, 150, 8, WS_VISIBLE
    dialogitem 'STATIC', '', ID_STATIC_CODE_TITLE, 10, 38, 50, 8, WS_VISIBLE
    dialogitem 'STATIC', '', ID_STATIC_CODE_VALUE, 50, 38, 150, 8, WS_VISIBLE
  enddialog
