This is README.TXT for Monster, a multiplayer adventure game for VMS.
Monster was written by Rich Skrenta at Northwestern University.

You may freely copy, distribute and change Monster as you wish.  Let me
know if you get it up and running, and if you change it, just because I'm
interested.  Send mail to

	skrenta@nuacc.acns.nwu.edu   or
	skrenta@nuacc.bitnet

Monster was written in VMS Pascal under VMS 4.6.  It uses file sharing and
record locking for communication.  Outside of that, it doesn't do anything
tricky.  However, after playing around with a VMS 4.2 system, I have
doubts if it will work on a system that old.  If you've got a reasonably
recent version of VMS and a Pascal compiler, you shouldn't have any problems.

The Monster source is in two files:  a short one, approx 300 lines, called
guts.pas, and a big one, mon.pas, approx 10,000 lines.  The compiled program
contains everything necessary to create and maintain the Monster universe.
There is no separate maintenance program.  Instead, a specific person in
the game has privileges, and is known as the "Monster Manager".  The MM
can do system maintenance while playing, and other players can even observe
his work.

Credit for the work to convert GUTS.PAS to a more portable form goes to
       
         Michael "the spide" Young   MCY1580@RITVAX.BITNET
         Chris "siouxane" Meck       CLM4346@RITVAX.BITNET

Many thanks to them for solving this sticky problem!

Rich Skrenta
November, 1988.