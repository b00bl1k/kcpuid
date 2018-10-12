;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2018. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; AMD BD (Family 15h)
zambezi     db 'Bulldozer/Zambezi', 0
vishera     db 'Piledriver/Vishera', 0

amd_codes:
        dd 0x6f01, zambezi
        dd 0x6f02, vishera
        dd 0, 0
