readme
--- Pitch only (enable=0b100) ---
   no-shift:    FAIL (1000.0 -> 0.0 Hz)
   octave up:   FAIL (expected ~2000.0, got 0.0 Hz)
 --- Full chain: Volâ†’Echoâ†’Pitch (enable=0b111) ---
   liveness:    FAIL (too many zeros: 1024/1024)
   clip check:  PASS
   freq check:  FAIL (expected ~1000, got 0.0 Hz)
 ========================================
   TOTAL ERRORS: 4
 ========================================
