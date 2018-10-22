;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2018. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CPU_INTEL = 'ntel'
CPU_AMD = 'cAMD'

struct cpuid_regs
        r_eax   dd ?
        r_ebx   dd ?
        r_ecx   dd ?
        r_edx   dd ?
ends

struct cpuid_info
        id      dd ? ; vendor
        step    db ?
        model   db ?
        family  db ?
        type    db ?
        emodel  db ?
        efamily db ?
        name    rb 13
        brand   rb 49
        codename dd ?
ends

; __stdcall u32 CpuidCall(const u32 * dump, u32 leaf, u32 subleaf, cpuid_regs * regs)
proc CpuidCall uses ebx esi, dump:DWORD, leaf:DWORD, subleaf:DWORD, regs:DWORD
        mov     esi, [dump]
        test    esi, esi
        mov     ecx, 50 ; TODO fix me
        jnz     .from_dump
        mov     eax, [leaf]
        mov     ecx, [subleaf]
        cpuid
        jmp     .result

.from_dump:
        lodsd
        cmp     eax, [leaf]
        jz      @f
        add     esi, 20
        jmp     .from_dump
@@:
        lodsd
        cmp     eax, [subleaf]
        jz      @f
        add     esi, 16
        loop    .from_dump
        mov     eax, 0x40000000
        jmp     .result ; TODO fix me
@@:
        lodsd
        push    eax
        lodsd
        mov     ebx, eax
        lodsd
        mov     ecx, eax
        lodsd
        mov     edx, eax
        pop     eax

.result:
        mov     esi, [regs]
        mov     [esi + cpuid_regs.r_eax], eax
        mov     [esi + cpuid_regs.r_ebx], ebx
        mov     [esi + cpuid_regs.r_ecx], ecx
        mov     [esi + cpuid_regs.r_edx], edx
        ret
endp

; __stdcall void IntelDecode(cpuid_info * info)
proc IntelDecode, info:DWORD
        push    esi
        mov     edx, [info]
        movzx   eax, [edx + cpuid_info.model]
        movzx   ecx, [edx + cpuid_info.emodel]
        shl     ecx, 4
        or      eax, ecx
        movzx   ecx, [edx + cpuid_info.family]
        shl     ecx, 8
        or      eax, ecx
        movzx   ecx, [edx + cpuid_info.efamily]
        shl     ecx, 12
        or      eax, ecx
        xchg    eax, ecx
        ; ecx is key for search
        mov     esi, intel_codes
        cld
  @@:
        lodsd
        cmp     eax, ecx
        jz      .found
        lodsd
        test    eax, eax
        jz      .return
        jmp     @b
  .found:
        lodsd
        mov     [edx + cpuid_info.codename], eax
  .return:
        pop     esi
        ret
endp

proc AmdDecode, info:DWORD
        push    esi
        mov     edx, [info]
        movzx   eax, [edx + cpuid_info.model]
        movzx   ecx, [edx + cpuid_info.family]
        shl     ecx, 8
        or      eax, ecx
        ; if family is less than 0x0f then emodel and efamily is reserved
        cmp     [edx + cpuid_info.family], 0x0f
        jl      @f
        movzx   ecx, [edx + cpuid_info.emodel]
        shl     ecx, 4
        or      eax, ecx
        movzx   ecx, [edx + cpuid_info.efamily]
        shl     ecx, 12
        or      eax, ecx
        xchg    eax, ecx
        ; ecx is key for search
        mov     esi, amd_codes
        cld
  @@:
        lodsd
        cmp     eax, ecx
        jz      .found
        lodsd
        test    eax, eax
        jz      .return
        jmp     @b
  .found:
        lodsd
        mov     [edx + cpuid_info.codename], eax
  .return:
        pop     esi
        ret
endp

; __stdcall void CpuidInit(cpuid_info * info, const u32 * dump)
proc CpuidInit, info:DWORD, dump:DWORD
locals
        max_leaf dd ?
        max_ext_leaf dd ?
        regs    cpuid_regs
endl
        push    ebx esi edi
        ; obtain name
        lea     eax, [regs]
        stdcall CpuidCall, [dump], 0, 0, eax
        ; store maximum number of leaf
        mov     [max_leaf], eax
        ; store name of vendor [ebx:edx:ecx]
        mov     ebx, [info]
        lea     edi, [ebx + cpuid_info.name]
        mov     eax, [regs + cpuid_regs.r_ebx]
        cld
        stosd
        mov     eax, [regs + cpuid_regs.r_edx]
        stosd
        mov     eax, [regs + cpuid_regs.r_ecx]
        stosd
        ; save vendor
        mov     [ebx + cpuid_info.id], eax
        ; terminate string
        xor     eax, eax
        stosb
        ; processor info
        lea     eax, [regs]
        stdcall CpuidCall, [dump], 1, 0, eax
        mov     eax, [regs + cpuid_regs.r_eax]
        mov     ebx, [info]
        ; stepping [3:0]
        mov     ecx, eax
        and     ecx, 0xf
        mov     [ebx + cpuid_info.step], cl
        ; model [7:4]
        mov     ecx, eax
        shr     ecx, 4
        and     ecx, 0xf
        mov     [ebx + cpuid_info.model], cl
        ; family [11:8]
        mov     ecx, eax
        shr     ecx, 8
        and     ecx, 0xf
        mov     [ebx + cpuid_info.family], cl
        ; processor type [13:12]
        mov     ecx, eax
        shr     ecx, 12
        and     ecx, 0x3
        mov     [ebx + cpuid_info.type], cl
        ; extended model [19:16]
        mov     ecx, eax
        shr     ecx, 16
        and     ecx, 0xf
        mov     [ebx + cpuid_info.emodel], cl
        ; extended family [27:20]
        mov     ecx, eax
        shr     ecx, 20
        mov     [ebx + cpuid_info.efamily], cl
        ; obtain brand name
        mov     esi, 0x80000002
        lea     edi, [ebx + cpuid_info.brand]
  @@:
        ; call in loop, three times
        lea     eax, [regs]
        stdcall CpuidCall, [dump], esi, 0, eax
        mov     eax, [regs + cpuid_regs.r_eax]
        stosd
        mov     eax, [regs + cpuid_regs.r_ebx]
        stosd
        mov     eax, [regs + cpuid_regs.r_ecx]
        stosd
        mov     eax, [regs + cpuid_regs.r_edx]
        stosd
        inc     esi
        cmp     esi, 0x80000004
        jle     @b
        xor     eax, eax
        stosb

        mov     eax, unknown_codename
        mov     [ebx + cpuid_info.codename], eax

        mov     eax, [ebx + cpuid_info.id]
        cmp     eax, CPU_INTEL
        jz      .intel
        cmp     eax, CPU_AMD
        jz      .amd
        jmp     .return

  .intel:
        stdcall IntelDecode, ebx
        jmp     .return

  .amd:
        stdcall AmdDecode, ebx

  .return:
        pop     edi esi ebx
        ret
endp

proc AppendRegs dst:DWORD, r_eax:DWORD, r_ecx:DWORD, regs:DWORD
        mov     eax, [dst]
        test    eax, eax
        jz      .nothing

        push    edi esi
        mov     edi, [dst]
        mov     eax, [r_eax]
        cld
        stosd
        mov     eax, [r_ecx]
        stosd
        mov     esi, [regs]
        mov     ecx, 4
        rep     movsd

        mov     eax, edi
        pop     esi edi
  .nothing:
        ret
endp

; __stdcall u32 CpuidDump(void * dst, u32 size)
proc CpuidDump dst:DWORD, size:DWORD
locals
        length  dd ?
        max_leaf dd ?
        max_sleaf dd ?
        regs    cpuid_regs
endl
        push    ebx esi edi
        mov     edi, [dst]

        xor     ebx, ebx
        xor     esi, esi
        mov     [length], ebx
  .read:
        lea     eax, [regs]
        stdcall CpuidCall, 0, ebx, esi, eax
        ; save result
        lea     eax, [regs]
        stdcall AppendRegs, edi, ebx, esi, eax
        mov     edi, eax ; new offset
        add     [length], 4 * 6
        ; check result
        test    ebx, ebx
        jz      .cpuid_0000_0000
        cmp     ebx, 0x4
        jz      .cpuid_0000_0004
        cmp     ebx, 0x7
        jz      .cpuid_0000_0007
        cmp     ebx, 0xb
        jz      .cpuid_0000_000b
        cmp     ebx, 0xd
        jz      .cpuid_0000_000d
        cmp     ebx, 0x14
        jz      .cpuid_0000_0014
        cmp     ebx, 0x80000000
        jz      .cpuid_8000_0000

  .next_leaf:
        inc     ebx
        cmp     ebx, [max_leaf]
        jg      .dump_ok
        jmp     .read

  .next_subleaf:
        inc     esi
        jmp     .read

  .cpuid_0000_0000:
        mov     eax, [regs + cpuid_regs.r_eax]
        ; get maximum supported standard level
        mov     [max_leaf], eax
        jmp     .next_leaf

  .cpuid_0000_0004:
        ; get cache configuration descriptors
        mov     eax, [regs + cpuid_regs.r_eax]
        ; cache level and type mask
        and     eax, 0x1f
        test    eax, eax
        jnz     .next_subleaf
        xor     esi, esi
        jmp     .next_leaf

  .cpuid_0000_0007:
        ; get feature flags
        test    esi, esi
        jnz     @f
        ; get maximum supported sub-level
        mov     eax, [regs + cpuid_regs.r_eax]
        mov     [max_sleaf], eax
  @@:
        cmp     esi, [max_sleaf]
        jl      .next_subleaf
        xor     esi, esi
        jmp     .next_leaf

  .cpuid_0000_000b:
        ; get topology enumeration information
        mov     eax, [regs + cpuid_regs.r_ecx]
        ; level type and number mask
        and     eax, 0xff00
        test    eax, eax
        jnz     .next_subleaf
        xor     esi, esi
        jmp     .next_leaf

  .cpuid_0000_000d:
        ; TODO: get extended state enumeration
        jmp     .next_leaf

  .cpuid_0000_0014:
        ; get processor trace (PT) capability enumeration
        test    esi, esi
        jnz     @f
        ; get maximum supported sub-level
        mov     eax, [regs + cpuid_regs.r_eax]
        mov     [max_sleaf], eax
  @@:
        cmp     esi, [max_sleaf]
        jl      .next_subleaf
        xor     esi, esi
        jmp     .next_leaf

  .cpuid_8000_0000:
        mov     eax, [regs + cpuid_regs.r_eax]
        and     eax, 0xff ; 255 is maximum leaf
        add     eax, ebx
        mov     [max_leaf], eax
        jmp     .next_leaf

  .dump_ok:
        cmp     ebx, 0x80000000
        jae     .exit
        mov     ebx, 0x80000000
        xor     esi, esi
        jmp     .read

  .exit:
        mov     eax, [length]
        pop     edi esi ebx
        ret
endp

proc ToHex num:DWORD, len:DWORD, dst:DWORD
        push    ebx edi
        mov     eax, [num]
        mov     ebx, 16
        mov     ecx, [len]
        mov     edi, [dst]
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
        pop     edi ebx
        ret
endp

; src -> pointer to string
; result -> pointer to store result
; return -> pointer to the first not valid symbol
proc FromHex src:DWORD, result:DWORD
        push    esi
        mov     esi, [src]
        xor     edx, edx
.next:
        xor     eax, eax
        lodsb
        cmp     al, '0'
        jl      .exit
        cmp     al, '9'
        ja      @f
        sub     al, '0'
        jmp     .ok
@@:
        cmp     al, 'A'
        jl      .exit
        cmp     al, 'F'
        ja      @f
        sub     al, 'A' - 10
        jmp     .ok
@@:
        cmp     al, 'a'
        jl      .exit
        cmp     al, 'f'
        ja      .exit
        sub     al, 'a' - 10
.ok:
        shl     edx, 4
        add     edx, eax
        jmp     .next
.exit:
        mov     eax, esi
        dec     eax
        mov     esi, [result]
        mov     [esi], edx
        pop     esi
        ret
endp

; src is 24 bytes array contains leaf, subleaf, eax, ebx, ecx and edx values
; dst is a buffer for string output (at least 60 bytes)
; return pointer to the end of src
proc DumpToStr uses esi edi, src:DWORD, dst:DWORD
        stdcall string.copy, dump_cpuid, [dst]
        stdcall string.length, [dst]
        add     eax, [dst]
        mov     edi, eax

        mov     esi, [src]

        lodsd
        stdcall ToHex, eax, 8, edi
        add     edi, 8
        mov     al, ':'
        stosb

        lodsd
        stdcall ToHex, eax, 2, edi
        add     edi, 2
        mov     ax, ' ='
        stosw

        mov     ecx, 4
  @@:
        mov     al, ' '
        stosb
        lodsd
        push    ecx
        stdcall ToHex, eax, 8, edi
        add     edi, 8
        pop     ecx
        loop    @b

        mov     al, 13
        stosb
        mov     al, 10
        stosb

        mov     eax, esi
        ret
endp


proc DumpParse uses ebx esi edi, src:DWORD, dst:DWORD
locals
        num     dd ?
        lines   dd ?
endl
        mov     esi, [src]
        mov     edi, [dst]
        mov     [lines], 0
.next:
        cmp     byte [esi], 0
        jz      .exit

        stdcall string.length, dump_cpuid
        mov     ebx, eax
        stdcall string.cmp, esi, dump_cpuid, eax
        test    eax, eax
        jnz     .err
        ; parse leaf number
        add     esi, ebx
        lea     eax, [num]
        stdcall FromHex, esi, eax
        mov     esi, eax
        lodsb
        cmp     al, ':'
        jnz     .err
        ; parse subleaf number
        ; save
        test    edi, edi
        jz      @f
        mov     eax, [num]
        stosd
@@:
        lea     eax, [num]
        stdcall FromHex, esi, eax
        mov     esi, eax
        lodsb
        cmp     al, ' '
        jnz     .err
        lodsb
        cmp     al, '='
        jnz     .err
        ; parse cpuid results
        mov     ecx, 4
@@:
        lodsb
        cmp     al, ' '
        jnz     .err
        ; save
        test    edi, edi
        jz      .b0
        mov     eax, [num]
        stosd
.b0:
        lea     eax, [num]
        stdcall FromHex, esi, eax
        mov     esi, eax
        loop    @b
        lodsb
        cmp     al, 13
        jnz     .err
        lodsb
        cmp     al, 10
        jnz     .err
        inc     [lines]
        test    edi, edi
        jz      .b1
        mov     eax, [num]
        stosd
.b1:
        jmp     .next

.exit:
.err:
        mov     eax, [lines]
        ret
endp