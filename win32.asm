;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2018. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE GUI 4.0
entry start

include 'win32a.inc'
include 'cpuid.inc'

ID_STATIC_VENDOR_TITLE = 100
ID_STATIC_VENDOR_VALUE = 101
ID_STATIC_BRAND_TITLE = 102
ID_STATIC_BRAND_VALUE = 103
ID_STATIC_CODE_TITLE = 104
ID_STATIC_CODE_VALUE = 105

section '.text' code readable executable

  start:
        invoke  GetProcessHeap
        test    eax, eax
        jz      exit
        mov     [cur_heap], eax
        ; how much memory need
        stdcall CpuidDump, 0, 0
        ; allocate it
        invoke  HeapAlloc, [cur_heap], 0, eax
        test    eax, eax
        jz      exit
        mov     [dump], eax
        stdcall CpuidDump, [dump], 0
        mov     [dump_size], eax

        ; open file dor dump
        invoke  CreateFile, dump_file, GENERIC_WRITE, 0, NULL, \
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
        cmp     eax, INVALID_HANDLE_VALUE
        jz      .free
        mov     [dump_handle], eax

        mov     esi, [dump]
  @@:
        call    foutput
        invoke  lstrlen, output
        invoke  WriteFile, [dump_handle], output, eax, dwtemp, NULL
        add     esi, 24
        mov     edi, [dump]
        add     edi, [dump_size]
        cmp     esi, edi
        jl      @b

        invoke  CloseHandle, [dump_handle]

  .free:
        invoke  HeapFree, [cur_heap], 0, [dump]

        lea     eax, [ci]
        stdcall CpuidInit, eax

        invoke  GetModuleHandle, 0
        invoke  DialogBoxParam, eax, 37, HWND_DESKTOP, DialogProc, 0
        or      eax, eax
        jz      exit

  exit:
        invoke  ExitProcess, 0

  eax2hex:
        push    eax ebx edx
        mov     byte [edi + ecx], 0 ; zero terminate
        mov     ebx, 16
  .l:
        xor     edx, edx
        div     ebx
        add     edx, "0"
        cmp     edx, "9"
        jbe     @f
        add     edx, "A" - "9" - 1
  @@:
        mov     [edi + ecx - 1], dl
        sub     ecx, 1
        jne     .l
        pop     edx ebx eax
        ret

  foutput:
        pusha
        mov     edi, output
        xor     eax, eax
        stosb

        invoke  lstrcat, output, cpuid_line

        lodsd ; eax arg
        mov     edi, hex_str
        mov     ecx, 8
        call    eax2hex
        invoke  lstrcat, output, hex_str
        invoke  lstrcat, output, colon

        lodsd ; ecx arg
        mov     edi, hex_str
        mov     ecx, 2
        call    eax2hex
        invoke  lstrcat, output, hex_str
        invoke  lstrcat, output, equal

        mov     ecx, 4
  @@:
        lodsd
        push    ecx
        mov     edi, hex_str
        mov     ecx, 8
        call    eax2hex
        invoke  lstrcat, output, hex_str
        invoke  lstrcat, output, space
        pop     ecx
        loop    @b

        invoke  lstrcat, output, crlf
        popa
        ret

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
  hex_str rb 10

section '.idata' import data readable writeable

  library kernel32, 'kernel32.dll', \
          user32, 'user32.dll'

  include 'api\kernel32.inc'
  include 'api\user32.inc'
  include 'common.inc'
  include 'intel.inc'
  include 'amd.inc'

  dump_file db 'cpuid.txt', 0
  cpuid_line db 'CPUID ', 0
  crlf db 13, 10, 0
  colon db ":", 0
  equal db " = ", 0
  space db " ", 0

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
