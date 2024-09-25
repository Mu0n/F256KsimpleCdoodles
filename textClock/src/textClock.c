#define F256LIB_IMPLEMENTATION
#include "f256lib.h"

//this function will convert binary coded decimals into regular decimals
byte bcdToDec(byte bcd){
	uint16_t tens = (bcd >> 4) & 0xF;
	uint16_t units = bcd & 0xF;
	return LOW_BYTE(tens * 10 + units);
}

int main(int argc, char *argv[]) {
	struct time_t time_data;
	byte oldsec=0; //keeps track to figure out if there's a change

	POKEW(0x00f0,LOW_BYTE(&time_data));
	textClear();
	textSetDouble(true,true);
	while(true)
		{
		kernelCall(Clock.GetTime);
		
		if(time_data.seconds != oldsec) //only update if different
			{
			if(time_data.seconds==0)kernelCall(Clock.GetTime);
			oldsec=time_data.seconds;
			textGotoXY(10,10);
			textPrint("                         ");
			textGotoXY(10,10);
			printf("month %d day %d %d:%d:%d",bcdToDec(time_data.month),
								bcdToDec(time_data.day),
								bcdToDec(time_data.hours),
								bcdToDec(time_data.minutes),
								bcdToDec(time_data.seconds));
			}
		}
	return 0;}
