START:
    0x30 01         ;;zet_poort_uit(poort=1, hsio=0);
    0x20 FF         ;; wachten(0xff)
    0x60 00 08      ;; spring(label=blinky);
    0xFF            ;; STOPPEN

BLINKY:
    0x31 01         ;;zet_poort_aan(poort=1, hsio=0);
    0x20 FF         ;; wachten(0xff)
    0x30 01         ;;zet_poort_uit(poort=1, hsio=0);
    0x20 FF         ;; wachten(0xff)
    0x60 00 08      ;; spring(label=blinky);

EINDE:
    0x30 01         ;;zet_poort_uit(poort=1, hsio=0);
    0xFF            ;; STOPPEN
