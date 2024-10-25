# F256KsimpleCdoodles

**looking for examples in other languages?**

_Go check my repo for Basic doodles here:_ https://github.com/Mu0n/F256KbasicBASICdoodles

_Go check my repo for ASM doodles here:_ https://github.com/Mu0n/F256KsimpleASMdoodles


A collection of small C (llvm-mos) experiments for the F256K from Foenix Retro Systems. Compiled into .PGZ format for easy executing on the target hardware (or in the FoenixIDE emulator).

Foenix Retro Systems
Website: https://c256foenix.com/

llvm-mos Compiler and Library package from Kangaroo Punch Studios:
https://kangaroopunch.com/view/ShowSoftware?id=13

Foenix Retro Systems is a homebrew effort to bring 8/16/32-bit era processors in a new, reimagined retro series of computer designed by Stefany Allaire (https://twitter.com/StefanyAllaire)

## write-on-screen

![Screenshot 2024-09-24 23-37-53](https://github.com/user-attachments/assets/8215ea3e-947c-42b6-87cf-6b9dfdb093ee)

Simply writes a word with random foreground and background text color

## textClock

![Screenshot 2024-09-24 23-36-13](https://github.com/user-attachments/assets/f2f858f3-10a9-41a9-8729-384206eaa001)

This will display fetch the clock data using the kernel call _Clock.GetTime_, which depends on the RTC (real time clock, needs a battery) and properly formats the output into human-readable text, using the library's textPrintInt functions. In order to accomplish this, a small helper function converts the data obtained in BCD (binary coded decimal - https://en.wikipedia.org/wiki/Binary-coded_decimal). Last but not least, the text of the clock's converted data is refreshed every second on screen.

## myTimer

 ![Screenshot 2024-09-24 23-36-50](https://github.com/user-attachments/assets/c87ef3e4-b682-4a95-8dc9-b733cbb43884)
   
This will set up 6 kernel timers, some based on frames, some based on seconds, in order to drive a character based animation (which cycles between characters #2 to #14, which look like partially filled rectangles).
The full list of timers used is:
1) every frame
2) every 5 frames
3) every 20 frames
4) every second
5) every 3 seconds
6) every minute

## midiStuff

![midStuff2](https://github.com/user-attachments/assets/6c89f4dd-bee8-4a22-86d8-e7407d4dfcf5)

This is meant for the F256K2 (and Jr. "the Second") since it targets the SAM2695 Dream IC found on those machines only. Use the arrow keys left/right to select the note to be played, use the up/down to select the instrument (from 0 to 128) and play a short test note with the space bar.
