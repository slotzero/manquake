Title    : ProQuake version 3.50
Filename : wqpro350.zip / glpro350.zip
Version  : 3.50
Date     : September 21, 2002
Author   : J.P. Grossman (a.k.a. Mephistopheles)
Email    : jpg@ai.mit.edu
URL      : http://planetquake.com/proquake


Welcome to ProQuake.  

ProQuake is the first modification to the quake source which is specifically 
intended for intense deathmatch play.  You won't find spiffy new graphics, 
cheats, changes to the physics, or features that work "most of the time".  
What you will find is a rock solid set of enhancements to unmodified netquake.
Things that quake should have had from the get go.. like precise aim.  Small, 
simple changes that improve the quality of netplay enormously.

ProQuake is fully compatible with standard NetQuake.  NetQuake clients can 
connect to ProQuake servers, ProQuake clients can connect to NetQuake 
servers, and ProQuake servers can run exactly the same modifications as 
NetQuake servers.  ProQuake is also fully compatible with the advanced 
features of Clanring CRMod++ version 6.0 including team scores, match 
timer and client pings.

==============
Important Note
==============

If you are getting an error telling you that your video mode is not supported 
and that you should try 65536 colours or the -window option, you need to go to
http://www.3dfxgamers.com to download the latest miniGL driver.

===========
What's New?
===========

Proquake 3.50 introduces the following features/improvements/fixes:

Client:

- Eliminated cheat-free lag for modem players on 3.50 servers
- "cheatfree" command tells you if you're connected to a cheat-free server
- Connect to 3.40 servers through routers/NAT/IP Masquerading 
- Fullbright shaft in glpro
- r_polyblend in wqpro (same as gl_polyblend in glpro)
- Support for more graphics modes in glpro
- Four button mouse support
- Mouse wheel support
- QuakePro+ "bestweapon" command

Server:

- Eliminated cheat-free lag for 3.50 modem players
- Append "(cheat-free)" to the host name for cheat-free servers
- 3.40 clients can connect through routers/NAT/IP Masquerading 
- iplog supports multiple servers
- Limit of 64 entries per IP address to prevent iplog spamming
- Clients aren't kicked from cheatfree servers if they don't have the map

============
Installation
============

1.  wqpro: Make sure that you have winquake installed and working properly
    glpro: Make sure that you have GLquake installed and working properly

2.  Place wqpro.exe/glpro.exe in the same directory as winquake/glquake

3.  Create the subdirectory id1/locs and put the .loc files there

4.  Start wqpro.exe/glpro.exe exactly as you started winquake/glquake

5.  READ THE MANUAL!!!

=================
Technical Support
=================

Send comments/questions/suggestions/bug reports to jpg@ai.mit.edu

If you encounter a problem with wqpro/glpro, before you even *think* of 
sending me email, make sure that you do not have similar problems with 
winquake/glquake.  ProQuake is a modification of win/gl quake; if you 
can't get win/gl quake to work then you won't be able to get ProQuake 
to work, and I certainly won't be able to help you.

==================
Author Information
==================

J.P. Grossman is a seventh year graduate student at M.I.T. in the department
of Electrical Engineering and Computer Science.  He has been hacking Quake 
since Oct. '97 when he began working on the Elohim Server.  J.P. has been 
hacking computers in general for over 23 years.  He started in grade 1 
writing BASIC programs for a VIC-20 and saving them on a tape drive, often 
accidentally erasing the end of the previous program.

================
Acknowledgements
================

First and foremost I'd like to thank John Carmack and the boys (and girls) at
Id Software for bringing us the greatest game ever.  Part of me wishes that Id
had burned to the ground after releasing Quake I, but I suppose that in that 
case they never would have releasd the source, for which I am also grateful.

A big thanks goes to Tovi "Izra'il" Grossman for his help with beta testing.
Tovi is by far and away the best tester I have ever worked with.  He is both
thorough and ruthless.. this man could find bugs in a class 10 semiconductor
fabrication clean room.

I am greatly indebted to Slot Zero for his extensive testing of cheat-free
ProQuake and the many resulting bug reports.  It is thanks to him that
I was able to release v3.1 so soon after v3.01.

Thanks to Lance "Ba'alzaman" Woollett and his Kiwi Quaker friends for their
help in testing version 3.40 across a router firewall.

