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

; __stdcall u32 CpuidCall(u32 leaf, u32 subleaf, cpuid_regs * regs)
proc CpuidCall, leaf:DWORD, subleaf:DWORD, regs:DWORD
        push    ebx esi
        mov     eax, [leaf]
        mov     ecx, [subleaf]
        cpuid
        mov     esi, [regs]
        mov     [esi + cpuid_regs.r_eax], eax
        mov     [esi + cpuid_regs.r_ebx], ebx
        mov     [esi + cpuid_regs.r_ecx], ecx
        mov     [esi + cpuid_regs.r_edx], edx
        pop     esi ebx
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

; __stdcall void CpuidInit(cpuid_info * info)
proc CpuidInit, info:DWORD
locals
        max_leaf dd ?
        max_ext_leaf dd ?
        regs    cpuid_regs
endl
        push    ebx esi edi
        ; obtain name
        lea     eax, [regs]
        stdcall CpuidCall, 0, 0, eax
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
        stdcall CpuidCall, 1, 0, eax
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
        stdcall CpuidCall, esi, 0, eax
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

proc AppendRegs, dst:DWORD, r_eax:DWORD, r_ecx:DWORD, regs:DWORD
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
proc CpuidDump, dst:DWORD, size:DWORD
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
        stdcall CpuidCall, ebx, esi, eax
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
