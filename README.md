# Fake BSOD

fake bsod is a progarm i made after watching some videos on a GTAV mod called chaos.
its basically a mod that does random stuff at random times, one of which was a fake death screen.
so i got the idea of creating a fake program that would blue screen the target computer at radom times.

### how does it work?

first of all i want to make it clear that this program wont call kernel BSOD (`NtRaiseHardError()`) so it wont actually
give you a BSOD. 

how it works is, in random times (first time is a constant 10 minutes), it will first give you an error sound, 
then it will mute the master volume of your windows, then it pops up a window (on top of all other programs) 
thats very much like the actual windows BSOD **BUT** the text is changed and the qr-code takes you to a good place 
;)

after poping-up, it will stay there for 10seconds and then it will hide it self, unmute the master volume and waits
until another random time to do the samething again.

### how to use?
download the latest release, uncompress and run it.

### how to build?

simple, install `gcc` (i suggest you use mingw as i did too), then add it to PATH of windows and run `build.cmd`,
it will make `BSOD.exe` inside the new build directory, run it and wait for it :)
(qr.bmp needs to be in the same directory as the program, for it to be shown! build.cmd copies automatically)

**For closing the program, open taskmanager and end task BSOD.exe process**

### Additional Notes
 - **This program doesn't harm your pc (see the source)**
 - **This program doesn't call Kernel BSOD**
