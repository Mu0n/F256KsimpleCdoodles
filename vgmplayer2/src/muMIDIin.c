#include "f256lib.h"
#include "../src/muMIDIin.h"
#include "../src/muMidi.h"
#include "../src/mudispatch.h"
#include "../src/textUI.h"

bool noteColors[88]={1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1,0,1,0,1, 1,0,1,0,1,0,1, 1};

void resetMID(midiInData *themidi)
{
themidi->recByte = 0x00; 
themidi->nextIsNote = 0x00;
themidi->nextIsSpeed = 0x00; 
themidi->isHit = 0x00; 
themidi->lastCmd = 0x90; 
themidi->lastNote = 0x00; 
themidi->storedNote = 0x00;	
}

void dealMIDIIn(midiInData *themidi)
{
uint16_t midiPending=0;

	if(!(PEEK(MIDI_CTRL) & 0x02)) //rx not empty
		{
			midiPending = PEEKW(MIDI_RXD) & 0x0FFF; //discard top 4 bits of MIDI_RXD+1
			//deal with the MIDI bytes and exhaust the FIFO buffer
				for(uint16_t i=0; i<midiPending; i++)
					{
					//get the next MIDI in FIFO buffer byte
					themidi->recByte=PEEK(MIDI_FIFO);
					if(themidi->recByte == 0xfe) continue; //active sense, ignored
					if(themidi->nextIsSpeed) //this block activates when a note is getting finished on the 3rd byte ie 0x90 0x39 0x40 (noteOn middleC midSpeed)
						{
						themidi->nextIsSpeed = false;
						//force a minimum level with this instead: recByte<0x70?0x70:recByte 
						/*
						if(themidi->isHit == true) //turn the last one off before dealing with the new one
							{
							dispatchNote(false,0,themidi->lastNote,themidi->recByte, false, 0, gPtr);
							}
						*/
						dispatchNote(themidi->isHit,0,themidi->storedNote,0x7F, false, 0, gPtr); //do the note or turn off the note
						if(themidi->isHit == false) //turn 'em off if the note is ended
							{
							//sidActive = false;
							}
						themidi->lastNote = themidi->storedNote;
						}
					else if(themidi->nextIsNote) //this block triggers if the previous byte was a NoteOn or NoteOff (0x90,0x80) command previously
						{
							
							//////////////////
							//NOTE COLORING
							///////////////////
							
							
						//figure out which note on the graphic is going to be highlighted
						uint8_t noteColorIndex = themidi->recByte-0x14;
						
						//first case is when the last command is a 0x90 'NoteOn' command
						if(themidi->isHit) {graphicsDefineColor(0, noteColorIndex,0xFF,0x00,0xFF); //paint it as a hit note
						//textGotoXY(0,20);textPrintInt(recByte);
						}
						//otherwise it's a 0x80 'NoteOff' command
						else {
							uint8_t detectedColor = noteColors[noteColorIndex-1]?0xFF:0x00;
							graphicsDefineColor(0, noteColorIndex,detectedColor,detectedColor,detectedColor); //swap back the original color according to this look up ref table noteColors
						}
						
						
						//////////////////
						//NOTE COLORING
						///////////////////
						
						themidi->nextIsNote = false; //return to previous state after a note is being dealt with
						themidi->storedNote = themidi->recByte;
						themidi->nextIsSpeed = true;
						}
					else if((themidi->nextIsNote == false) && (themidi->nextIsSpeed == false)) //what command are we getting next?
					{
					switch(themidi->recByte & 0xF0)
						{
						case 0x90: //we know it's a 'NoteOn', get ready to analyze the note byte, which is next
							themidi->nextIsNote = true;
							themidi->isHit=true;
							themidi->lastCmd = themidi->recByte;
							break;
						case 0x80: //we know it's a 'NoteOff', get ready to analyze the note byte, which is next
							themidi->nextIsNote = true;
							themidi->isHit=false;
							themidi->lastCmd = themidi->recByte;
							break;
						case 0x00 ... 0x7F:
							themidi->storedNote = themidi->recByte;
							themidi->nextIsNote = false; //false because we just received it!
							themidi->nextIsSpeed = true;
							switch(themidi->lastCmd & 0xF0)
								{
								case 0x90:
									themidi->isHit=true;
									break;
								case 0x80:
									themidi->isHit=true;
									break;
								}
							break;
						}
					}
				}
				
		}
		
}
}