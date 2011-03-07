/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// host.c -- coordinates spawning and killing of local servers

#include "quakedef.h"

/*

Memory is cleared / released when a server begins, not when it ends.

*/

quakeparms_t host_parms;

qboolean	host_initialized;		// true if into command execution

double		host_frametime;
double		realtime;				// without any filtering or bounding
double		oldrealtime;			// last frame run

int			host_hunklevel;
int			minimum_memory;

client_t	*host_client;			// current client

jmp_buf 	host_abortserver;

cvar_t	host_framerate = {"host_framerate","0"};	// set for slow motion
cvar_t	sys_ticrate = {"sys_ticrate","0.05"};
cvar_t	serverprofile = {"serverprofile","0"};
cvar_t	fraglimit = {"fraglimit","0",false,true};
cvar_t	timelimit = {"timelimit","0",false,true};
cvar_t	teamplay = {"teamplay","0",false,true};
cvar_t	samelevel = {"samelevel","0"};
cvar_t	noexit = {"noexit","0",false,true};
cvar_t	developer = {"developer","0"};
cvar_t	skill = {"skill","1"};						// 0 - 3
cvar_t	deathmatch = {"deathmatch","0"};			// 0, 1, or 2
cvar_t	coop = {"coop","0"};			// 0 or 1
cvar_t	pausable = {"pausable","1"};
cvar_t	temp1 = {"temp1","0"};
cvar_t	proquake = {"proquake", "L33T"}; // JPG - added this

// JPG - spam protection.  If a client's msg's/second exceeds spam rate
// for an extended period of time, the client is spamming.  Clients are
// allowed a temporary grace of pq_spam_grace messages.  Once used up,
// this grace regenerates while the client shuts up at a rate of one
// message per pq_spam_rate seconds.
cvar_t	pq_spam_rate = {"pq_spam_rate", "1.5"};
cvar_t	pq_spam_grace = {"pq_spam_grace", "10"};

// JPG 3.20 - control muting of players that change colour/name
cvar_t	pq_tempmute = {"pq_tempmute", "1"};

// JPG 3.20 - optionally write player binds to server log
cvar_t	pq_logbinds = {"pq_logbinds", "0"};

// JPG 3.11 - feature request from Slot Zero
cvar_t	pq_showedict = {"pq_showedict", "0"};

// JPG 1.05 - translate dedicated server console output to plain text
cvar_t	pq_dequake = {"pq_dequake", "1"};
cvar_t	pq_maxfps = {"pq_maxfps", "72.0"};	// JPG 1.05

// Slot Zero 3.50-2  Mute spamming client.
cvar_t	pq_mute_spam_client = {"pq_mute_spam_client", "0"};

// Slot Zero 3.50-2  IP masking.
cvar_t	pq_ipmask = {"pq_ipmask", "1"};

// Slot Zero 3.50-2  RQ support.
cvar_t	mod_protocol = {"mod_protocol", "1"};


/*
================
Host_EndGame
================
*/
void Host_EndGame (char *message, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,message);
	dpvsnprintf (string, sizeof(string), message, argptr);
	va_end (argptr);

	Con_DPrintf ("Host_EndGame: %s\n",string);

	if (sv.active)
		Host_ShutdownServer (false);

	Sys_Error ("Host_EndGame: %s\n",string);	// dedicated servers exit

	longjmp (host_abortserver, 1);
}


/*
================
Host_Error

This shuts down the server
================
*/
void Host_Error (char *error, ...)
{
	va_list		argptr;
	char		string[1024];
	static	qboolean inerror = false;

	if (inerror)
		Sys_Error ("Host_Error: recursively entered");
	inerror = true;

	va_start (argptr,error);
	dpvsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	Con_Printf ("Host_Error: %s\n",string);

	if (sv.active)
		Host_ShutdownServer (false);

	Sys_Error ("Host_Error: %s\n",string);	// dedicated servers exit

	inerror = false;

	longjmp (host_abortserver, 1);
}


/*
================
Host_FindMaxClients
================
*/
void Host_FindMaxClients (void)
{
	int		i;

	i = COM_CheckParm ("-dedicated");
	if (i)
	{
		if (i != (com_argc - 1))
			svs.maxclients = atoi (com_argv[i+1]);
		else
			svs.maxclients = 8;
	}
	else
		svs.maxclients = 8;

	i = COM_CheckParm ("-listen");
	if (i)
		Sys_Error ("Only -dedicated can be specified");

	if (svs.maxclients < 1)
		svs.maxclients = 8;
	else if (svs.maxclients > MAX_SCOREBOARD)
		svs.maxclients = MAX_SCOREBOARD;

	svs.maxclientslimit = svs.maxclients;
	if (svs.maxclientslimit < 4)
		svs.maxclientslimit = 4;
	svs.clients = Hunk_AllocName (svs.maxclientslimit*sizeof(client_t), "clients");

	Cvar_SetValue ("deathmatch", 1.0);
}

char dequake[256];	// JPG 1.05

/*
=======================
Host_InitDeQuake

JPG 1.05 - initialize the dequake array
======================
*/
void Host_InitDeQuake (void)
{
	int i;

	for (i = 1 ; i < 12 ; i++)
		dequake[i] = '#';
	dequake[9] = 9;
	dequake[10] = 10;
	dequake[13] = 13;
	dequake[12] = ' ';
	dequake[1] = dequake[5] = dequake[14] = dequake[15] = dequake[28] = '.';
	dequake[16] = '[';
	dequake[17] = ']';
	for (i = 0 ; i < 10 ; i++)
		dequake[18 + i] = '0' + i;
	dequake[29] = '<';
	dequake[30] = '-';
	dequake[31] = '>';
	for (i = 32 ; i < 128 ; i++)
		dequake[i] = i;
	for (i = 0 ; i < 128 ; i++)
		dequake[i+128] = dequake[i];
	dequake[128] = '(';
	dequake[129] = '=';
	dequake[130] = ')';
	dequake[131] = '*';
	dequake[138] = ' '; // Slot Zero 3.50-2  Dequake ASCII character 138.
	dequake[141] = '>';
}


/*
=======================
Host_InitLocal
======================
*/
void Host_InitLocal (void)
{
	Host_InitCommands ();

	Cvar_RegisterVariable (&host_framerate);
	Cvar_RegisterVariable (&sys_ticrate);
	Cvar_RegisterVariable (&serverprofile);
	Cvar_RegisterVariable (&fraglimit);
	Cvar_RegisterVariable (&timelimit);
	Cvar_RegisterVariable (&teamplay);
	Cvar_RegisterVariable (&samelevel);
	Cvar_RegisterVariable (&noexit);
	Cvar_RegisterVariable (&skill);
	Cvar_RegisterVariable (&developer);
	Cvar_RegisterVariable (&deathmatch);
	Cvar_RegisterVariable (&coop);
	Cvar_RegisterVariable (&pausable);
	Cvar_RegisterVariable (&temp1);
	Cvar_RegisterVariable (&proquake);				// JPG - added this so QuakeC can find it
	Cvar_RegisterVariable (&pq_spam_rate);			// JPG - spam protection
	Cvar_RegisterVariable (&pq_spam_grace);			// JPG - spam protection
	Cvar_RegisterVariable (&pq_tempmute);			// JPG 3.20 - temporary muting
	Cvar_RegisterVariable (&pq_showedict);			// JPG 3.11 - feature request from Slot Zero
	Cvar_RegisterVariable (&pq_dequake);			// JPG 1.05 - translate dedicated console output to plain text
	Cvar_RegisterVariable (&pq_maxfps);				// JPG 1.05
	Cvar_RegisterVariable (&pq_logbinds);			// JPG 3.20 - log player binds
	Cvar_RegisterVariable (&pq_mute_spam_client);	// Slot Zero 3.50-2  Mute spamming client.
	Cvar_RegisterVariable (&pq_ipmask);				// Slot Zero 3.50-2  IP masking.
	Cvar_RegisterVariable (&mod_protocol);			// Slot Zero 3.50-2  RQ Support.

	Host_FindMaxClients ();
	Host_InitDeQuake();	// JPG 1.05 - initialize dequake array
}


/*
=================
SV_ClientPrintf

Sends text across to be displayed
FIXME: make this just a stuffed echo?
=================
*/
void SV_ClientPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	dpvsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_print);
	MSG_WriteString (&host_client->message, string);
}


/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	int			i;

	va_start (argptr,fmt);
	dpvsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && svs.clients[i].spawned)
		{
			MSG_WriteByte (&svs.clients[i].message, svc_print);
			MSG_WriteString (&svs.clients[i].message, string);
		}
	}
}


/*
=================
Host_ClientCommands

Send text over to the client to be executed
=================
*/
void Host_ClientCommands (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,fmt);
	dpvsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	MSG_WriteByte (&host_client->message, svc_stufftext);
	MSG_WriteString (&host_client->message, string);
}


/*
=====================
SV_DropClient

Called when the player is getting totally kicked off the host
if (crash = true), don't bother sending signofs
=====================
*/
void SV_DropClient (qboolean crash)
{
	int		saveSelf;
	int		i;
	client_t *client;

	// JPG 3.00 - don't drop a client that's already been dropped!
	if (!host_client->active)
		return;

	if (!crash)
	{
		// send any final messages (don't check for errors)
		if (NET_CanSendMessage (host_client->netconnection))
		{
			MSG_WriteByte (&host_client->message, svc_disconnect);
			NET_SendMessage (host_client->netconnection, &host_client->message);
		}

		if (host_client->edict && host_client->spawned)
		{
		// call the prog function for removing a client
		// this will set the body to a dead frame, among other things
			saveSelf = pr_global_struct->self;
			pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
			PR_ExecuteProgram (pr_global_struct->ClientDisconnect);
			pr_global_struct->self = saveSelf;
		}

        // Slot Zero 3.50-2  Added edict to client removed message.
        if (pq_showedict.value)
			    Sys_Printf("#%d ", NUM_FOR_EDICT(host_client->edict));

		Sys_Printf ("Client %s removed\n",host_client->name);
	}

	// JPG 3.00 - check to see if it's a qsmack client
	if (host_client->netconnection->mod == MOD_QSMACK)
		qsmackActive = false;

// break the net connection
	NET_Close (host_client->netconnection);
	host_client->netconnection = NULL;

// free the client (the body stays around)
	host_client->active = false;
	host_client->name[0] = 0;
	host_client->old_frags = -999999;
	net_activeconnections--;

// send notification to all clients
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		MSG_WriteByte (&client->message, svc_updatename);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteString (&client->message, "");
		MSG_WriteByte (&client->message, svc_updatefrags);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteShort (&client->message, 0);
		MSG_WriteByte (&client->message, svc_updatecolors);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteByte (&client->message, 0);
	}
}


/*
==================
Host_ShutdownServer

This only happens at the end of a game, not between levels
==================
*/
void Host_ShutdownServer(qboolean crash)
{
	int		i;
	int		count;
	sizebuf_t	buf;
	char		message[4];
	double	start;

	if (!sv.active)
		return;

	sv.active = false;

// flush any pending messages - like the score!!!
	start = Sys_FloatTime();
	do
	{
		count = 0;
		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->active && host_client->message.cursize)
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					NET_SendMessage(host_client->netconnection, &host_client->message);
					SZ_Clear (&host_client->message);
				}
				else
				{
					NET_GetMessage(host_client->netconnection);
					count++;
				}
			}
		}
		if ((Sys_FloatTime() - start) > 3.0)
			break;
	}
	while (count);

// make sure all the clients know we're disconnecting
	buf.data = message;
	buf.maxsize = 4;
	buf.cursize = 0;
	MSG_WriteByte(&buf, svc_disconnect);
	count = NET_SendToAll(&buf, 5);
	if (count)
		Con_Printf("Host_ShutdownServer: NET_SendToAll failed for %u clients\n", count);

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_DropClient(crash);

//
// clear structures
//
	memset (&sv, 0, sizeof(sv));
	memset (svs.clients, 0, svs.maxclientslimit*sizeof(client_t));
}


/*
================
Host_ClearMemory

This clears all the memory used by the server, but does
not reinitialize anything.
================
*/
void Host_ClearMemory (void)
{
	Con_DPrintf ("Clearing memory\n");
	Mod_ClearAll ();
	if (host_hunklevel)
		Hunk_FreeToLowMark (host_hunklevel);

	memset (&sv, 0, sizeof(sv));
}


//============================================================================


/*
===================
Host_FilterTime

Returns false if the time is too short to run a frame
===================
*/
qboolean Host_FilterTime (float time)
{
	realtime += time;

	if (realtime - oldrealtime < 1.0 / pq_maxfps.value)
		return false;		// framerate is too high

	host_frametime = realtime - oldrealtime;
	oldrealtime = realtime;

	if (host_framerate.value > 0)
		host_frametime = host_framerate.value;
	else
	{	// don't allow really long or short frames
		if (host_frametime > 0.1)
			host_frametime = 0.1;
		if (host_frametime < 0.001)
			host_frametime = 0.001;
	}

	return true;
}


/*
===================
Host_GetConsoleCommands

Add them exactly as if they had been typed at the console
===================
*/
void Host_GetConsoleCommands (void)
{
	char	*cmd;

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
	}
}


/*
==================
Host_ServerFrame

==================
*/
void Host_ServerFrame (void)
{
// JPG 3.00 - stuff the port number into the server console once every minute
	static double port_time = 0;

	if (port_time > sv.time + 1 || port_time < sv.time - 60)
	{
		port_time = sv.time;
		Cmd_ExecuteString(va("port %d\n", net_hostport), src_command);
	}

// run the world state
	pr_global_struct->frametime = host_frametime;

// set the time and clear the general datagram
	SV_ClearDatagram ();

// check for new clients
	SV_CheckForNewClients ();

// read client messages
	SV_RunClients ();

// move things around and think
	if (!sv.paused)
		SV_Physics ();

// send all messages to the clients
	SV_SendClientMessages ();
}


/*
==================
Host_Frame

Runs all active servers
==================
*/
void _Host_Frame (float time)
{
	if (setjmp (host_abortserver))
		return;			// something bad happened, or the server disconnected

// keep the random time dependent
	rand ();

// decide the simulation time
	if (!Host_FilterTime (time))
		return;			// don't run too fast, or packets will flood out

// get new key events
	Sys_SendKeyEvents ();

// process console commands
	Cbuf_Execute ();

	NET_Poll();

// check for commands typed to the host
	Host_GetConsoleCommands ();

	if (sv.active)
		Host_ServerFrame ();
}

void Host_Frame (float time)
{
	double	time1, time2;
	static double	timetotal;
	static int		timecount;
	int		i, c, m;

	if (!serverprofile.value)
	{
		_Host_Frame (time);
		return;
	}

	time1 = Sys_FloatTime ();
	_Host_Frame (time);
	time2 = Sys_FloatTime ();

	timetotal += time2 - time1;
	timecount++;

	if (timecount < 1000)
		return;

	m = timetotal*1000/timecount;
	timecount = 0;
	timetotal = 0;
	c = 0;
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active)
			c++;
	}

	Con_Printf ("serverprofile: %2i clients %2i msec\n",  c,  m);
}


//============================================================================


/*
====================
Host_Init
====================
*/
void Host_Init (quakeparms_t *parms)
{

	if (standard_quake)
		minimum_memory = MINIMUM_MEMORY;
	else
		minimum_memory = MINIMUM_MEMORY_LEVELPAK;

	if (COM_CheckParm ("-minmemory"))
		parms->memsize = minimum_memory;

	host_parms = *parms;

	if (parms->memsize < minimum_memory)
		Sys_Error ("Only %4.1f megs of memory available, can't execute game", parms->memsize / (float)0x100000);

	com_argc = parms->argc;
	com_argv = parms->argv;

	// JPG 3.00 - moved this here
#ifdef _WIN32
	srand(time(NULL) ^ _getpid());
#else
	srand(time(NULL) ^ getpid());
#endif

	Memory_Init (parms->membase, parms->memsize);
	Cbuf_Init ();
	Cmd_Init ();
	Cvar_Init ();
	COM_Init (parms->basedir);
	Host_InitLocal ();
	W_LoadWadFile ("gfx.wad");
	Con_Init ();
	PR_Init ();
	Mod_Init ();
	Security_Init ();	// JPG 3.20 - cheat free
	NET_Init ();
	SV_Init ();
	IPLog_Init ();	// JPG 1.05 - ip address logging
	BANLog_Init ();

	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
	Con_Printf ("%4.1f megabyte heap\n",parms->memsize/ (1024*1024.0));

	R_InitTextures ();		// needed even for dedicated servers

	Cbuf_AddText ("exec config.cfg;exec autoexec.cfg;stuffcmds;startdemos\n");

	Hunk_AllocName (0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	host_initialized = true;

	Sys_Printf ("========Quake Initialized========\n");
}


/*
===============
Host_Shutdown

FIXME: this is a callback from Sys_Quit and Sys_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void Host_Shutdown(void)
{
	static qboolean isdown = false;

	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}
	isdown = true;

	IPLog_WriteLog ();	// JPG 1.05 - ip loggging
	NET_Shutdown ();
}
