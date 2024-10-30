#define F256LIB_IMPLEMENTATION

#define MIDI_CTRL 0xDDA0
#define MIDI_OUT 0xDDA1
#define MIDI_RX_1 0xDDA2
#define MIDI_RX_2 0xDDA3

#define TIMER_FRAMES 0
#define TIMER_SECONDS 1

 //TIMER_TEXT for the 1-frame long text refresh timer for data display
 //TIMER_NOTE is for a midi note timer
#define TIMER_RX_COOKIE 0
#define TIMER_GUI_COOKIE 1
#define TIMER_RX_DELAY 1
#define TIMER_GUI_DELAY 1

#include "f256lib.h"
struct timer_t midiRxTimer, GUITimer; //timer_t structure for setting timer through the kernel

uint16_t note = 0x36, oldnote; /*note is the current midi hex note code to send. oldnote keeps the previous one so it can be Note_off'ed away after the timer expires, or a new note is called*/
uint16_t prgInst = 0; /* program change value, the MIDI instrument number */

bool setTimer(const struct timer_t *timer)
{
    *(uint8_t*)0xf3 = timer->units;
    *(uint8_t*)0xf4 = timer->absolute;
    *(uint8_t*)0xf5 = timer->cookie;
    kernelCall(Clock.SetTimer);
	return !kernelError;
}

uint8_t getTimerAbsolute(uint8_t units)
{
    *(uint8_t*)0xf3 = units | 0x80;
    return kernelCall(Clock.SetTimer);
}

void setInstruments()
{
	POKE(MIDI_OUT,123);
	POKE(MIDI_OUT,0);
	POKE(MIDI_OUT,0xFF);
}
void setup()
{
	setInstruments();
	GUITimer.units = TIMER_SECONDS;
	GUITimer.absolute = TIMER_GUI_DELAY;
	GUITimer.cookie = TIMER_GUI_COOKIE;
	
	midiRxTimer.units = TIMER_FRAMES;
	midiRxTimer.absolute = TIMER_RX_DELAY;
	midiRxTimer.cookie = TIMER_RX_COOKIE;
	setTimer(&midiRxTimer);
	setTimer(&GUITimer);
}
void fetchNextEventMIDI()
{
	uint16_t count=0;
	
	count = count | PEEK(MIDI_RX_1);
	count = count | (uint16_t)(PEEK(MIDI_RX_2)>>8);
	
	printf("count %d ",count);
	if(count > 0) POKE(MIDI_OUT, PEEK(MIDI_OUT));
}
void midiNoteOn(uint8_t wantNote, uint8_t chan)
{
	//Send a Note_On midi command on channel 0		
	POKE(MIDI_OUT, 0x90 | chan);
	POKE(MIDI_OUT, wantNote);
	POKE(MIDI_OUT, 0x7F);
}



/*
VIRQ = $FFFE
INT_PEND_0 = $D660 ; Pending register for interrupts 0 - 7
INT_PEND_1 = $D661 ; Pending register for interrupts 8 - 15
INT_MASK_0 = $D66C ; Mask register for interrupts 0 - 7
INT_MASK_1 = $D66D ; Mask register for interrupts 8 - 15
start: ; Disable IRQ handling
sei
; Load my IRQ handler into the IRQ vector
; NOTE: this code just takes over IRQs completely. It could save
; the pointer to the old handler and chain to it when it has
; handled its interrupt. But what is proper really depends on
; what the program is trying to do.
lda #<my_handler
sta VIRQ
lda #>my_handler
sta VIRQ+1
; Mask off all but the SOF interrupt
lda #$ff
sta INT_MASK_1
and #~INT00_VKY_SOF
sta INT_MASK_0
; Clear all pending interrupts
lda #$ff
sta INT_PEND_0
sta INT_PEND_1
; Put a character in the upper right of the screen
lda #SYS_CTRL_TEXT_PG
sta SYS_CTRL_1
lda #’@’
sta $c000
; Set the color of the character
lda #SYS_CTRL_COLOR_PG
sta SYS_CTRL_1
lda #$F0
sta $c000
; Go back to I/O page 0
stz SYS_CTRL_1
; Make sure we’re in text mode
lda #$01 ; enable TEXT
sta $D000 ; Save that to VICKY master control register 0
stz $D001
; Re-enable IRQ handling
cli


SYS_CTRL_1 = $0001
SYS_CTRL_TEXT_PG = $02
my_handler: pha
; Save the system control register
lda SYS_CTRL_1
pha
; Switch to I/O page 0
stz SYS_CTRL_1
; Check for SOF flag
lda #INT00_VKY_SOF
bit INT_PEND_0
beq return ; If it’s zero, exit the handler
; Yes: clear the flag for SOF
sta INT_PEND_0
; Move to the text screen page
lda #SYS_CTRL_TEXT_PG
sta SYS_CTRL_1
; Increment the character at position 0
inc $c000
; Restore the system control register
return: pla
sta SYS_CTRL_1
; Return to the original code
pla
rti

*/
int main(int argc, char *argv[]) {
	bool disa = false;
	setup();
	POKE(1,0);
    while(true) 
        {
		
        }
return 0;}
