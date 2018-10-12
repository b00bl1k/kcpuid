;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2018. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

use32
        org     0x0

        db      'MENUET01'
        dd      0x1
        dd      START
        dd      I_END
        dd      IM_END + 0x1000
        dd      IM_END + 0x1000
        dd      0, 0

include '../../proc32.inc'
include '../../macros.inc'
include '../../struct.inc'
include '../../KOSfuncs.inc'
include 'cpuid.inc'

TITLES_X = 15
VALUES_X = 70

START:
        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        lea     eax, [ci]
        stdcall CpuidInit, eax

red_win:
        call    draw_window

mainloop:
        mcall   SF_WAIT_EVENT

        dec     eax
        jz      red_win

        dec     eax
        jz      key

        dec     eax
        jz      button

        jmp     mainloop

button:
        mcall   SF_GET_BUTTON

        test    ah, ah
        jz      mainloop
exit:
        mcall   SF_TERMINATE_PROCESS

key:
        mcall   SF_GET_KEY
        jmp     mainloop

draw_window:
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mov     edx, [sc.work]
        or      edx, 0x34000000
        xor     esi, esi
        mov     edi, winmain_title
        mcall   SF_CREATE_WINDOW, 50 shl 16 + 350, 30 shl 16 + 400

        ; Draw titles
        mov     ebx, TITLES_X shl 16 + 15
        mov     ecx, 0x80000000
        or      ecx, [sc.work_text]
        mov     edx, vendor_title
        mcall   SF_DRAW_TEXT
        mov     ebx, TITLES_X shl 16 + 35
        mov     edx, brand_title
        mcall   SF_DRAW_TEXT
        mov     ebx, TITLES_X shl 16 + 55
        mov     edx, codename_title
        mcall   SF_DRAW_TEXT

        ; Draw values
        mov     ebx, VALUES_X shl 16 + 15
        lea     edx, [ci + cpuid_info.name]
        mcall   SF_DRAW_TEXT
        mov     ebx, VALUES_X shl 16 + 35
        lea     edx, [ci + cpuid_info.brand]
        mcall   SF_DRAW_TEXT
        mov     ebx, VALUES_X shl 16 + 55
        mov     edx, [ci + cpuid_info.codename]
        mcall   SF_DRAW_TEXT

        mcall   SF_REDRAW, SSF_END_DRAW
        ret

include 'common.inc'
include 'intel.inc'
include 'amd.inc'

I_END:
sc      system_colors
ci      cpuid_info

IM_END: