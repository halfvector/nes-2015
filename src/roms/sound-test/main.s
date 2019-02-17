.include "defs.s"

.code

;;; ----------------------------------------------------------------------------
;;; Reset handler

.proc reset
    sei            ; Disable interrupts
    cld            ; Clear decimal mode
    ldx #$ff
    txs            ; Initialize SP = $FF
    inx
    stx PPUCTRL        ; PPUCTRL = 0
    stx PPUMASK        ; PPUMASK = 0
    stx APUSTATUS        ; APUSTATUS = 0

    ;; PPU warmup, wait two frames, plus a third later.
    ;; http://forums.nesdev.com/viewtopic.php?f=2&t=3958
:    bit PPUSTATUS
    bpl :-
:    bit PPUSTATUS
    bpl :-

    ;; Zero ram.
    txa
:    sta $000, x
    sta $100, x
    sta $200, x
    sta $300, x
    sta $400, x
    sta $500, x
    sta $600, x
    sta $700, x
    inx
    bne :-

    ;; Final wait for PPU warmup.
:    bit PPUSTATUS
    bpl :-

    ;; Play audio forever.
    lda #$01        ; enable channel
    sta APUSTATUS
    lda #%10111000  ;Duty 00, Volume 8 (half volume)
    sta $4000
    lda #$FD        ;$0FD is 440hz for pulse wave
    sta $4002       ;low 8 bits of period
    lda #$00
    sta $4003       ;high 3 bits of period

;    lda #$04        ; enable triangle channel
;    sta APUSTATUS
;    lda #%00000001
;    sta $4008
;    lda #$7E        ;$07E is 440hz for triangle wave
;    sta $400A
;    lda #%00001000  ;play forever
;    sta $400B

forever:
    jmp forever
.endproc

;;; ----------------------------------------------------------------------------
;;; NMI (vertical blank) handler

.proc nmi
    rti
.endproc

;;; ----------------------------------------------------------------------------
;;; IRQ handler

.proc irq
    rti
.endproc

;;; ----------------------------------------------------------------------------
;;; Vector table

.segment "VECTOR"
.addr nmi
.addr reset
.addr irq

;;; ----------------------------------------------------------------------------
;;; Empty CHR data, for now

.segment "CHR0a"
.segment "CHR0b"
