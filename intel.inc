;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2018. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Family 5 Names
p5a     db 'P5 A-step', 0
p5      db 'P5', 0
p54c    db 'P54C', 0
p54t    db 'P54T Overdrive', 0
p55c    db 'P55C (with MMX)', 0
; Family 6 Names
; Big cores
proa   db 'Pentium Pro A-step', 0
pro    db 'Pentium Pro', 0
klamath db 'Pentium II (Klamath)', 0
deschutes db 'Pentium II (Deschutes)', 0
medocino db 'Celeron (Medocino)', 0
katmai  db 'Pentium III (Katmai)', 0
coppermine db 'Pentium III (Coppermine)', 0
banias  db 'Pentium M (Banias)', 0
cascades db 'Pentium III Xeon (Cascades)', 0
tualatin db 'Pentium III (Tualatin)', 0
dothan   db 'Pentium M (Dothan)', 0
yonah   db 'Pentium M (Yonah) / Core', 0
conroe  db 'Pentium D (Conroe)/ Core 2 (Kentsfield)', 0
; Pentium 4
willamette db 'Pentium 4 (Willamette)', 0
northwood db 'Pentium 4 (Northwood)', 0
gallatin db 'Pentium 4 EE (Gallatin)', 0
prescott db 'Pentium 4 (Prescott)', 0
prescott2m db 'Pentium 4 (Prescott 2M)', 0
cedar_mill db 'Pentium 4 (Cedar Mill)', 0


pentiumm db 'Pentiumm', 0
modpent db 'Modified Pentium M', 0
core    db 'Core', 0
penryn  db 'Penryn', 0
merom   db 'Merom Conroe', 0
yorkfield db 'Yorkfield', 0
bloomfield db 'Bloomfield', 0
dunnington db 'Dunnington', 0
clarksfield db 'Clarksfield', 0
gulftown db 'Gulftown/Westmere-EP', 0
arrandale db 'Arrandale/Clarkdale', 0
westmere db 'Westmere-EX', 0
sandy   db 'Sandy Bridge', 0
sandy_e db 'Sandy bridge-E', 0
nehalem db 'Nehalem-EX', 0
ivy     db 'Ivy Bridge', 0
haswell db 'Haswell', 0
broadwell db 'Broadwell', 0
sky     db 'Skylake', 0
kaby    db 'Kaby Lake', 0
coffe   db 'Coffee Lake', 0
cannon  db 'Cannon Lake', 0
whiskey db 'Whiskey Lake', 0
amber   db 'Amber Lake', 0
ice     db 'Ice Lake', 0
tiger   db 'Tigerlake', 0
alder   db 'Alder Lake', 0
meteor  db 'Meteor Lake', 0

intel_codes:
    ; code (family | model), name
    ; f5
    dd 0x0500, p5a
    dd 0x0501, p5
    dd 0x0502, p54c
    dd 0x0503, p54t
    dd 0x0504, p55c
    dd 0x0507, p54c
    dd 0x0508, p55c
    ; f6
    dd 0x0600, proa
    dd 0x0601, pro
    dd 0x0603, klamath
    dd 0x0605, deschutes
    dd 0x0606, medocino
    dd 0x0607, katmai
    dd 0x0608, coppermine
    dd 0x0609, banias
    dd 0x060a, cascades
    dd 0x060b, tualatin
    dd 0x060d, dothan
    dd 0x060e, yonah
    dd 0x060f, conroe

    dd 0x0f07, willamette
    dd 0x0f0a, willamette
    dd 0x0f12, willamette
    dd 0x0f13, willamette
    dd 0x0f24, northwood
    dd 0x0f25, northwood
    dd 0x0f27, northwood
    dd 0x0f29, northwood

    dd 0x0616, merom
    dd 0x0617, yorkfield
    dd 0x061a, bloomfield
    dd 0x061d, dunnington
    dd 0x061f, clarksfield
    dd 0x0625, arrandale
    dd 0x062a, sandy
    dd 0x062c, gulftown
    dd 0x062d, sandy_e
    dd 0x062e, nehalem
    dd 0x062f, westmere
    dd 0x063a, ivy
    dd 0x068e, kaby
    dd 0x069e, kaby
    dd 0, 0

