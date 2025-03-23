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

## NoFussDemos

Go there https://github.com/Mu0n/F256KsimpleCdoodles/tree/main/NoFussDemos to pick a selection of a few demos that are standalone .pgz executable programs.

## F256amp 
(its code in mp3wSA - its executable is in NoFussDemos https://github.com/Mu0n/F256KsimpleCdoodles/tree/main/NoFussDemos)

![Screenshot 2025-03-10 03-05-46](https://github.com/user-attachments/assets/7dc572aa-fb2e-415b-ae45-cee5c28b32e5)

F256amp is a mp3 player that uses the VS1053b sound chip only found in the Jr.2 and K2 (it will not work and quit out on a Jr. or a K). The command line is as such:
<pre>
/- F256amp yourtune.mp3
</pre>

if you put the program in a root folder called _apps and rename the program as mp3player.pgz, you can use Micah's f/manager https://github.com/WartyMN/F256-FileManager/releases to auto-launch this program when you pick and execute .mp3 (.wav, .ogg, .wma, etc) files from its interface.

## midiplayer v1.2 
(its code in midisam - its executable is in NoFussDemos https://github.com/Mu0n/F256KsimpleCdoodles/tree/main/NoFussDemos)

![midiplayerv1 2](https://github.com/user-attachments/assets/52fd42fa-7758-4a7a-a53e-640ed4c081ec)

midiplayer plays standard MIDI files, type 0 with 1 track or type 1 with multiple tracks.  It uses the sound chips only found in the Jr.2 and K2, using by default the SAM2695 which sounds best for MIDI playback, but you can also toggle to the VS1053b MIDI playback, which has its own timbre. If you attempt to play it on the original Jr. or K, you will see the visuals, but you will only have silence. 
Known bug: type 1 which have several tempo changes will disrupt the song duration estimation because it will base it only on the last tempo of the song instead of taking into account every tempo change. The command line is as such:
<pre>
/- midiplayer yourtune.mid
</pre>

if you put the program in a root folder called _apps and rename the program as midiplayer.pgz, you can use Micah's f/manager https://github.com/WartyMN/F256-FileManager/releases to auto-launch this program when you pick and execute .mid files from its interface.

## Sprite

![Screenshot 2025-01-17 16-30-14](https://github.com/user-attachments/assets/b6ed8445-34f7-447d-9877-c3cb20900a3b)

This is a small example that shows how to embed a sprite graphic with multiple frames of animation, a full screen bitmap and a common palette for both.
Hit space to advance through the sprite frames and see the peon chop wood!

## Bach's MIDI Hero

![image](https://github.com/user-attachments/assets/f721fd45-dbf1-4660-a6fc-2d9ce965399e)

This is my game entry to the Foenix community's Game Jam for Oct 25-27 in 2024.
It's a musical game where you can freely play notes using the keys on your keyboard (white row of keys from Z to / and Q to ], black keys from A to ; and 2 to +). By default, it can use the MIDI chip in the K2 and Jr.2 since they come equipped with a sam2695 IC from Dream. A toggle to PSG sound can be done so that the original K and Jr. can also participate, albeit with simpler sound. A demo mode of the tune can be activated with F5. Win with a perfect score while you play the main mode (F1) and get a special reward!

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

![Screenshot 2024-11-11 10-27-28](https://github.com/user-attachments/assets/70ff66e5-73a5-4fbc-b671-1087199a1108)

This is meant for the F256K2 (and Jr. "the Second") since it targets the SAM2695 Dream IC found on those machines only. Use the arrow keys left/right to select the note to be played, use the up/down to select the instrument (from 0 to 128) and play a short test note with the space bar. If you plug in a MIDI controller in the MIDI in port, you can also play that way (much easier and faster for tests). The relevant MIDI uart register addresses will try to display in real time the data that passes through them. 

## lcd

![image](https://github.com/user-attachments/assets/a4a7e8f2-8f41-4910-ad2e-3d91afb2b08f)

This only works with a F256K2 since it sends a special converted bitmap image to an LCD only present in the K2 enclosure. Here are some instructions in the wiki to get this process going: https://wiki.f256foenix.com/index.php?title=Use_the_K2_LCD 
Therein, you'll find a link to a python script to help the conversion process.

## lightshow

![image](https://github.com/user-attachments/assets/4b302ed3-5d81-4043-b752-4b1560b14972)

This is supposed to detect 2 RGB addressable LEDs on a Jr. or Jr2, 3 RGB LEDs on a F256K and 4 RGB LEDs on a F256K2.
It will pick random RGB values and send them to the registers every 20 frames (about 3 times per second)


