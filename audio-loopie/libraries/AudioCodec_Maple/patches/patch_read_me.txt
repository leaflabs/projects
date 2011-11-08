patch_read_me.txt
guest openmusiclabs 7.26.11

this folder contains patches for the Maple library that allow the codecshield to run, or to run better. the various file(s) are listed below.

timer.c - this fixes a timer interrupt bug on Maple_IDE_0.0.11 and lower.  the codecshield will not function without this patch.  all other Maple routines will still run with this patch.  replace the "timer.c" file in the folder:

\maple-ide-0.0.11-windowsxp32\hardware\leaflabs\cores\maple\

with the "timer.c" file included here.

you should make a backup copy of the original "timer.c" file just in case future bugs arrive and other code you write doesnt work with it. 

if you are using maple-ide-0.0.12 or higher you probably dont need to use this patch.
