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

#include "quakedef.h"

extern cvar_t	pausable;

// JPG - added these for spam protection
extern cvar_t	pq_spam_rate;
extern cvar_t	pq_spam_grace;

// JPG 3.20 - control muting of players that change colour/name
extern cvar_t	pq_tempmute;

// JPG - feature request from Slot
extern cvar_t	pq_showedict;

// JPG 3.20 - optionally remove '\r'
extern cvar_t	pq_removecr;

// JPG 3.20 - optionally write player binds to server log
extern cvar_t	pq_logbinds;

// Slot Zero 3.50-2  Mute spamming client.
extern cvar_t	pq_mute_spam_client;

// Slot Zero 3.50-2  IP masking.
extern cvar_t	pq_ipmask;

int	current_skill;

void Mod_Print (void);

/*
==================
Host_Quit_f
==================
*/
void Host_Quit_f (void)
{
	Host_ShutdownServer(false);
	Sys_Quit ();
}


/*
==================
Host_Status_f
==================
*/
void Host_Status_f (void)
{
	client_t	*client;
	int			seconds;
	int			minutes;
	int			hours = 0;
	int			j;
	int			a, b, c, qsmack;	// Slot Zero 3.50-1  Added this.
	void		(*print) (char *fmt, ...);

	qsmack = 0;
	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
		print = Con_Printf;
	}
	else
	{
		print = SV_ClientPrintf;
		if (host_client->netconnection->mod == MOD_QSMACK)
			qsmack = 1;
	}

	print ("host:    %s\n", Cvar_VariableString ("hostname"));
	print ("version: ProQuake %4.2f %s\n", PROQUAKE_VERSION, pq_cheatfree ? "cheat-free" : ""); // JPG - added ProQuake
	if (tcpipAvailable)
		print ("tcp/ip:  %s\n", my_tcpip_address);
	if (ipxAvailable)
		print ("ipx:     %s\n", my_ipx_address);
	print ("map:     %s\n", sv.name);
	print ("players: %i active (%i max)\n\n", net_activeconnections, svs.maxclients);
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = (int)(net_time - client->netconnection->connecttime);
		minutes = seconds / 60;
		if (minutes)
		{
			seconds -= (minutes * 60);
			hours = minutes / 60;
			if (hours)
				minutes -= (hours * 60);
		}
		else
			hours = 0;
		// Slot Zero 3.50-2  Put [ ] brackets around frags.
		if (host_client == client && !qsmack)
			print ("#%-2u %-16.16s \20%3i\21 %2i:%02i:%02i\n", j+1, client->name, (int)client->edict->v.frags, hours, minutes, seconds);
		else
			print ("#%-2u %-16.16s  %3i  %2i:%02i:%02i\n", j+1, client->name, (int)client->edict->v.frags, hours, minutes, seconds);
		// Slot Zero 3.50-2  IP masking (6 lines)
		if (!pq_ipmask.value || host_client->privileged || qsmack || cmd_source == src_command)
			print ("   %s\n", client->netconnection->address);
		else if (pq_ipmask.value == 1 && sscanf(client->netconnection->address, "%d.%d.%d", &a, &b, &c) == 3)
			print ("   %d.%d.%d.xxx\n", a, b, c);
		else
			print ("   private\n");
	}
}


/*
==================
Host_Cheatfree_f
==================
*/
void Host_Cheatfree_f (void)
{
	if (sv.active)
		Con_Printf(pq_cheatfree ? "This is a cheat-free server\n" : "This is not a cheat-free server\n");
	else
		Con_Printf(pq_cheatfree ? "Connected to a cheat-free server\n" : "Not connected to a cheat-free server\n");
}


/*
==================
Host_God_f

Sets client to godmode
==================
*/
void Host_God_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_GODMODE;
	if (!((int)sv_player->v.flags & FL_GODMODE) )
		SV_ClientPrintf ("godmode OFF\n");
	else
		SV_ClientPrintf ("godmode ON\n");
}


void Host_Notarget_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_NOTARGET;
	if (!((int)sv_player->v.flags & FL_NOTARGET) )
		SV_ClientPrintf ("notarget OFF\n");
	else
		SV_ClientPrintf ("notarget ON\n");
}

qboolean noclip_anglehack;

void Host_Noclip_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv_player->v.movetype != MOVETYPE_NOCLIP)
	{
		noclip_anglehack = true;
		sv_player->v.movetype = MOVETYPE_NOCLIP;
		SV_ClientPrintf ("noclip ON\n");
	}
	else
	{
		noclip_anglehack = false;
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf ("noclip OFF\n");
	}
}


/*
==================
Host_Fly_f

Sets client to flymode
==================
*/
void Host_Fly_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	if (sv_player->v.movetype != MOVETYPE_FLY)
	{
		sv_player->v.movetype = MOVETYPE_FLY;
		SV_ClientPrintf ("flymode ON\n");
	}
	else
	{
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf ("flymode OFF\n");
	}
}


/*
==================
Host_Ping_f

==================
*/
void Host_Ping_f (void)
{
	int		i, j;
	float	total;
	client_t	*client;
	void		(*print) (char *fmt, ...);

	if (cmd_source == src_command)
		print = Con_Printf;
	else
		print = SV_ClientPrintf;

	print ("Client ping times:\n");
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		total = 0;
		for (j=0 ; j<NUM_PING_TIMES ; j++)
			total+=client->ping_times[j];
		total /= NUM_PING_TIMES;
		print ("%4i %s\n", (int)(total*1000), client->name);
	}
}


/*
===============================================================================

SERVER TRANSITIONS

===============================================================================
*/


/*
======================
Host_Map_f

handle a
map <servername>
command from the console.  Active clients are kicked off.
======================
*/
void Host_Map_f (void)
{
	int		i;
	char	name[MAX_QPATH];

	if (cmd_source != src_command)
		return;

	Host_ShutdownServer(false);

	svs.serverflags = 0;			// haven't completed an episode yet
	strcpy (name, Cmd_Argv(1));

	SV_SpawnServer (name);

	if (!sv.active)
		return;

	// JPG 3.20 - cheat free
	pq_cheatfree = (pq_cvar_cheatfree.value && pq_cheatfreeEnabled);
	if (pq_cheatfree)
		Con_Printf("Spawning cheat-free server\n");
}

/*
==================
Host_Changelevel_f

Goes to a new map, taking all clients along
==================
*/
void Host_Changelevel_f (void)
{
	char	level[MAX_QPATH];

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("changelevel <levelname> : continue game on a new level\n");
		return;
	}
	if (!sv.active)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}
	SV_SaveSpawnparms ();
	strcpy (level, Cmd_Argv(1));
	SV_SpawnServer (level);
}

/*
==================
Host_Restart_f

Restarts the current server for a dead player
==================
*/
void Host_Restart_f (void)
{
	char	mapname[MAX_QPATH];

	if (!sv.active)
		return;

	if (cmd_source != src_command)
		return;
	strcpy (mapname, sv.name);	// must copy out, because it gets cleared
								// in sv_spawnserver
	SV_SpawnServer (mapname);
}


extern char server_name[MAX_QPATH];	// JPG 3.50


/*
======================
Host_Name_f
======================
*/
void Host_Name_f (void)
{
	char	*newName;
	int a, b, c;	// JPG 1.05 - ip address logging

	if (Cmd_Argc () == 1)
	{
		Con_Printf ("\"name\" is \"%s\"\n", cl_name.string);
		return;
	}
	if (Cmd_Argc () == 2)
		newName = Cmd_Argv(1);
	else
		newName = Cmd_Args();
	newName[15] = 0;

	// JPG 3.02 - remove bad characters
	for (a = 0 ; newName[a] ; a++)
	{
		if (newName[a] == 10)
			newName[a] = ' ';
		else if (newName[a] == 13)
			newName[a] += 128;
	}

	if (cmd_source == src_command)
	{
		if (Q_strcmp(cl_name.string, newName) == 0)
			return;
		Cvar_Set ("cl_name", newName);
		return;
	}

	if (host_client->name[0] && strcmp(host_client->name, "unconnected") )
		if (Q_strcmp(host_client->name, newName) != 0)
			Con_Printf ("%s renamed to %s\n", host_client->name, newName);
	Q_strcpy (host_client->name, newName);
	host_client->edict->v.netname = host_client->name - pr_strings;

	// JPG 1.05 - log the IP address
	if (sscanf(host_client->netconnection->address, "%d.%d.%d", &a, &b, &c) == 3)
		IPLog_Add((a << 16) | (b << 8) | c, newName);

	// JPG 3.00 - prevent messages right after a colour/name change
	host_client->change_time = sv.time;

// send notification to all clients

	MSG_WriteByte (&sv.reliable_datagram, svc_updatename);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteString (&sv.reliable_datagram, host_client->name);
}


void Host_Version_f (void)
{
	Con_Printf ("ProQuake Version %4.2f Build %4.2f\n", PROQUAKE_VERSION, PROQUAKE_BUILD); // JPG - added ProQuake
	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
}


void Host_Say(qboolean teamonly)
{
	client_t *client;
	client_t *save;
	int		j;
	int		spam_client; // Slot Zero 3.50-2  Added this.
	char	*p;
	unsigned char	text[64];
	qboolean	fromServer = false;

	if (cmd_source == src_command)
	{
		fromServer = true;
		teamonly = false;
	}

	if (Cmd_Argc () < 2)
		return;

	save = host_client;

	p = Cmd_Args();
// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[Q_strlen(p)-1] = 0;
	}

	spam_client = 0; // Slot Zero 3.50-2  Mute spamming client.
// turn on color set 1
	if (!fromServer)
	{
        // Slot Zero 3.50-2  Prevent "unconnected" messages. (2 lines)
		if (!host_client->spawned)
			return;

		// JPG - spam protection
		if (sv.time - host_client->spam_time > pq_spam_rate.value * pq_spam_grace.value)
			host_client->spam_time = sv.time - pq_spam_rate.value * pq_spam_grace.value;
		host_client->spam_time += pq_spam_rate.value;
		if (host_client->spam_time > sv.time)
			spam_client = 1;

		// JPG 3.00 - don't allow messages right after a colour/name change
		if (pq_tempmute.value && sv.time - host_client->change_time < 1 && host_client->netconnection->mod != MOD_QSMACK)
			spam_client = 1;

		// Slot Zero 3.50-2  Mute spamming client. (2 lines)
		if (pq_mute_spam_client.value && spam_client)
			return;

		// JPG 3.11 - feature request from Slot Zero
		if (pq_showedict.value && !spam_client) // Slot Zero 3.50-2  Mute spamming client.
			Sys_Printf("#%d ", NUM_FOR_EDICT(host_client->edict));

		// Slot Zero 3.50-2  Observer Say (2 lines)
		if (teamonly && (int)save->edict->v.flags & FL_OBSERVER)
			sprintf (text, "%c[%s]: ", 1, save->name);
		else if (teamplay.value && teamonly) // JPG - added () for mm2
			sprintf (text, "%c(%s): ", 1, save->name);
		else
			sprintf (text, "%c%s: ", 1, save->name);

		// JPG 3.20 - optionally remove '\r'
		if (pq_removecr.value)
		{
			char *ch;
			for (ch = p ; *ch ; ch++)
			{
				if (*ch == '\r')
					*ch += 128;
			}
		}
	}
	else
		// Slot Zero 3.50-2  Server uses cl_name instead of hostname.string.
		sprintf (text, "%c<%s> ", 1, cl_name.string);

	j = sizeof(text) - 2 - Q_strlen(text);  // -2 for /n and null terminator
	if (Q_strlen(p) > j)
		p[j] = 0;

	strcat (text, p);
	strcat (text, "\n");

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client || !client->active || !client->spawned)
			continue;

		if (teamplay.value && teamonly && client->edict->v.team != save->edict->v.team)
			continue;

		// Slot Zero 3.50-2  Mute spamming client. (2 lines)
		if (spam_client && client != save)
			continue;

		// Slot Zero 3.50-2  Observer Say (3 lines)
		if (teamonly && (int)save->edict->v.flags & FL_OBSERVER
				&& !((int)client->edict->v.flags & FL_OBSERVER && (int)save->edict->v.flags & FL_OBSERVER))
			continue;

		host_client = client;
		SV_ClientPrintf("%s", text);
	}
	host_client = save;

	// Slot Zero 3.50-2  Mute spamming client. (2 lines)
	if (spam_client)
		return;

	// JPG 3.20 - optionally write player binds to server log
	if (pq_logbinds.value)
		Con_Printf("%s", &text[1]);
	else
		Sys_Printf("%s", &text[1]);
}


void Host_Say_f(void)
{
	Host_Say(false);
}


void Host_Say_Team_f(void)
{
	Host_Say(true);
}


void Host_Tell_f(void)
{
	client_t *client;
	client_t *save;
	int		j;
	char	*p;
	char	text[64];

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	// JPG - disabled Tell (to prevent cheating)
	if (host_client->netconnection->mod != MOD_QSMACK)
	{
		SV_ClientPrintf("%cTell is diabled on this server\n", 1);
		return;
	}

	if (Cmd_Argc () < 3)
		return;

	Q_strcpy(text, host_client->name);
	Q_strcat(text, ": ");

	p = Cmd_Args();

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[Q_strlen(p)-1] = 0;
	}

// check length & truncate if necessary
	j = sizeof(text) - 2 - Q_strlen(text);  // -2 for /n and null terminator
	if (Q_strlen(p) > j)
		p[j] = 0;

	strcat (text, p);
	strcat (text, "\n");

	save = host_client;
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active || !client->spawned)
			continue;
		if (Q_strcasecmp(client->name, Cmd_Argv(1)))
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
		break;
	}
	host_client = save;
}


/*
==================
Host_Color_f
==================
*/
void Host_Color_f(void)
{
	int		top, bottom;
	int		playercolor;

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"color\" is \"%i %i\"\n", ((int)cl_color.value) >> 4, ((int)cl_color.value) & 0x0f);
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else
	{
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}

	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;

	playercolor = top*16 + bottom;

	if (cmd_source == src_command)
	{
		Cvar_SetValue ("_cl_color", playercolor);
		return;
	}

	// JPG 3.11 - bail if the color isn't actually changing
	if (host_client->colors == playercolor)
		return;

	host_client->colors = playercolor;
	host_client->edict->v.team = bottom + 1;

	// JPG 3.00 - prevent messages right after a colour/name change
	if (bottom)
		host_client->change_time = sv.time;

// send notification to all clients
	MSG_WriteByte (&sv.reliable_datagram, svc_updatecolors);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, host_client->colors);
}

/*
==================
Host_Kill_f
==================
*/
void Host_Kill_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	// Slot Zero 3.50-1  Quake Info Pool's dead player (aka zombie) fix.
	if ((sv_player->v.health <= 0) && (sv_player->v.deadflag != DEAD_NO))	// Slot Zero 3.50-1  Add additonal arguments.
	{
		SV_ClientPrintf ("Can't suicide -- already dead!\n");	// JPG 3.02 allready->already
		return;
	}

	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (pr_global_struct->ClientKill);
}


/*
==================
Host_Pause_f
==================
*/
void Host_Pause_f (void)
{

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}
	if (!pausable.value)
		SV_ClientPrintf ("Pause not allowed.\n");
	else
	{
		sv.paused ^= 1;

		if (sv.paused)
		{
			SV_BroadcastPrintf ("%s paused the game\n", pr_strings + sv_player->v.netname);
		}
		else
		{
			SV_BroadcastPrintf ("%s unpaused the game\n",pr_strings + sv_player->v.netname);
		}

	// send notification to all clients
		MSG_WriteByte (&sv.reliable_datagram, svc_setpause);
		MSG_WriteByte (&sv.reliable_datagram, sv.paused);
	}
}

//===========================================================================


/*
==================
Host_PreSpawn_f
==================
*/
void Host_PreSpawn_f (void)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("prespawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("prespawn not valid -- already spawned\n");	// JPG 3.02 allready->already
		return;
	}

	SZ_Write (&host_client->message, sv.signon.data, sv.signon.cursize);
	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 2);
	host_client->sendsignon = true;

	host_client->netconnection->encrypt = 2; // JPG 3.50
}

/*
==================
Host_Spawn_f
==================
*/
void Host_Spawn_f (void)
{
	int		i;
	client_t	*client;
	edict_t	*ent;

	if (cmd_source == src_command)
	{
		Con_Printf ("spawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("Spawn not valid -- already spawned\n");	// JPG 3.02 allready->already
		return;
	}

	// JPG 3.20 - model and exe checking
	host_client->nomap = false;
	if (pq_cheatfree && host_client->netconnection->mod != MOD_QSMACK)
	{
		int i;
		unsigned long crc;
		unsigned a, b;

		a = MSG_ReadLong();
		b = MSG_ReadLong();

		if (!Security_Verify(a, b))
		{
			MSG_WriteByte(&host_client->message, svc_print);
			MSG_WriteString(&host_client->message, "Invalid executable\n");
			Con_Printf("%s (%s) connected with an invalid executable\n", host_client->name, host_client->netconnection->address);
			SV_DropClient(false);
			return;
		}

		for (i = 1 ; sv.model_precache[i] ; i++)
		{
			if (sv.model_precache[i][0] != '*')
			{
				crc = MSG_ReadLong();
				if (crc != sv.model_crc[i])
				{
					if (i == 1 && crc == 0)	// allow clients to connect if they don't have the map
					{
						Con_Printf("%s does not have map %s\n", host_client->name, sv.model_precache[1]);
						host_client->nomap = true;
						break;
					}
					else
					{
						MSG_WriteByte(&host_client->message, svc_print);
						MSG_WriteString(&host_client->message, va("%s is invalid\n", sv.model_precache[i]));
						Con_Printf("%s (%s) connected with an invalid %s\n", host_client->name, host_client->netconnection->address, sv.model_precache[i]);
						SV_DropClient(false);
						return;
					}
				}
			}
		}
	}

// run the entrance script
	if (sv.loadgame)
	{	// loaded games are fully inited allready
		// if this is the last client to be connected, unpause
		sv.paused = false;
	}
	else
	{
		// set up the edict
		ent = host_client->edict;

		memset (&ent->v, 0, progs->entityfields * 4);
		ent->v.colormap = NUM_FOR_EDICT(ent);
		ent->v.team = (host_client->colors & 15) + 1;
		ent->v.netname = host_client->name - pr_strings;

		// copy spawn parms out of the client_t

		for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
			(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];

		// call the spawn function

		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientConnect);

		if ((Sys_FloatTime() - host_client->netconnection->connecttime) <= sv.time)
        {
            // Slot Zero 3.50-2  Added edict to entered the game message.
            if (pq_showedict.value)
			    Sys_Printf("#%d ", NUM_FOR_EDICT(host_client->edict));

			Sys_Printf ("%s entered the game [%s]\n", host_client->name, host_client->netconnection->address);
        }

		PR_ExecuteProgram (pr_global_struct->PutClientInServer);
	}


// send all current names, colors, and frag counts
	SZ_Clear (&host_client->message);

// send time of update
	MSG_WriteByte (&host_client->message, svc_time);
	MSG_WriteFloat (&host_client->message, sv.time);

	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		MSG_WriteByte (&host_client->message, svc_updatename);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteString (&host_client->message, client->name);
		MSG_WriteByte (&host_client->message, svc_updatefrags);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteShort (&host_client->message, client->old_frags);
		MSG_WriteByte (&host_client->message, svc_updatecolors);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteByte (&host_client->message, client->colors);
	}

// send all current light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		MSG_WriteByte (&host_client->message, svc_lightstyle);
		MSG_WriteByte (&host_client->message, (char)i);
		MSG_WriteString (&host_client->message, sv.lightstyles[i]);
	}

//
// send some stats
//
	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALSECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct->total_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->message, pr_global_struct->total_monsters);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_SECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct->found_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_MONSTERS);
	MSG_WriteLong (&host_client->message, pr_global_struct->killed_monsters);

//
// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM( 1 + (host_client - svs.clients) );
	MSG_WriteByte (&host_client->message, svc_setangle);
	for (i=0 ; i < 2 ; i++)
		MSG_WriteAngle (&host_client->message, ent->v.angles[i] );
	MSG_WriteAngle (&host_client->message, 0 );

	SV_WriteClientdataToMessage (sv_player, &host_client->message);

	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 3);
	host_client->sendsignon = true;

	// JPG - added this for spam protection
	host_client->spam_time = 0;
}

/*
==================
Host_Begin_f
==================
*/
void Host_Begin_f (void)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("begin is not valid from the console\n");
		return;
	}

	host_client->spawned = true;

	host_client->netconnection->encrypt = 0;	// JPG 3.50
}

//===========================================================================


/*
==================
Host_Kick_f

Kicks a user off of the server
==================
*/
void Host_Kick_f (void)
{
	char		*who;
	char		*message = NULL;
	client_t	*save;
	int			i;
	qboolean	byNumber = false;

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
	}
	else if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	save = host_client;

	if (Cmd_Argc() > 2 && Q_strcmp(Cmd_Argv(1), "#") == 0)
	{
		i = Q_atof(Cmd_Argv(2)) - 1;
		if (i < 0 || i >= svs.maxclients)
			return;
		if (!svs.clients[i].active)
			return;
		host_client = &svs.clients[i];
		byNumber = true;
	}
	else
	{
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if (!host_client->active)
				continue;
			if (Q_strcasecmp(host_client->name, Cmd_Argv(1)) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (cmd_source == src_command)
			if (1)
				who = "Console";
			else
				who = cl_name.string;
		else
			who = save->name;

		// can't kick yourself!
		if (host_client == save)
			return;

		if (Cmd_Argc() > 2)
		{
			message = COM_Parse(Cmd_Args());
			if (byNumber)
			{
				message++;							// skip the #
				while (*message == ' ')				// skip white space
					message++;
				message += Q_strlen(Cmd_Argv(2));	// skip the number
			}
			while (*message && *message == ' ')
				message++;
		}
		if (message)
			SV_ClientPrintf ("Kicked by %s: %s\n", who, message);
		else
			SV_ClientPrintf ("Kicked by %s\n", who);
		SV_DropClient (false);
	}

	host_client = save;
}

/*
===============================================================================

DEBUGGING TOOLS

===============================================================================
*/

/*
==================
Host_Give_f
==================
*/
void Host_Give_f (void)
{
	char	*t;
	int		v, w;
	eval_t	*val;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch && !host_client->privileged)
		return;

	t = Cmd_Argv(1);
	v = atoi (Cmd_Argv(2));

	switch (t[0])
	{
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      // MED 01/04/97 added hipnotic give stuff
      if (hipnotic)
      {
         if (t[0] == '6')
         {
            if (t[1] == 'a')
               sv_player->v.items = (int)sv_player->v.items | HIT_PROXIMITY_GUN;
            else
               sv_player->v.items = (int)sv_player->v.items | IT_GRENADE_LAUNCHER;
         }
         else if (t[0] == '9')
            sv_player->v.items = (int)sv_player->v.items | HIT_LASER_CANNON;
         else if (t[0] == '0')
            sv_player->v.items = (int)sv_player->v.items | HIT_MJOLNIR;
         else if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
      else
      {
         if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
		break;

    case 's':
		if (rogue)
		{
	        val = GetEdictFieldValue(sv_player, "ammo_shells1");
		    if (val)
			    val->_float = v;
		}

        sv_player->v.ammo_shells = v;
        break;
    case 'n':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_nails1");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
		else
		{
			sv_player->v.ammo_nails = v;
		}
        break;
    case 'l':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_lava_nails");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
        break;
    case 'r':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_rockets1");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
		else
		{
			sv_player->v.ammo_rockets = v;
		}
        break;
    case 'm':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_multi_rockets");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
        break;
    case 'h':
        sv_player->v.health = v;
        break;
    case 'c':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_cells1");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
		else
		{
			sv_player->v.ammo_cells = v;
		}
        break;
    case 'p':
		if (rogue)
		{
			val = GetEdictFieldValue(sv_player, "ammo_plasma");
			if (val)
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
        break;
    }
}


/*
==================
Host_Startdemos_f
==================
*/
void Host_Startdemos_f (void)
{
	if (!sv.active)
		Cbuf_AddText ("map start\n");
}


/*
===============================================================================

PROQUAKE FUNCTIONS (JPG 1.05)

===============================================================================
*/

// used to translate to non-fun characters for identify <name>
char unfun[129] =
"................[]olzeasbt89...."
"........[]......olzeasbt89..[.]."
"aabcdefghijklmnopqrstuvwxyz[.].."
".abcdefghijklmnopqrstuvwxyz[.]..";

// try to find s1 inside of s2
int unfun_match (char *s1, char *s2)
{
	int i;
	for ( ; *s2 ; s2++)
	{
		for (i = 0 ; s1[i] ; i++)
		{
			if (unfun[s1[i] & 127] != unfun[s2[i] & 127])
				break;
		}
		if (!s1[i])
			return true;
	}
	return false;
}

/* JPG 1.05
==================
Host_Identify_f

Print all known names for the specified player's ip address
==================
*/
void Host_Identify_f (void)
{
	int i;
	int a, b, c;
	char name[16];

	if (!iplog_size)
	{
		Con_Printf("IP logging not available\nUse -iplog command line option\n");
		return;
	}

	if (Cmd_Argc() < 2)
	{
		Con_Printf("usage: identify <player number or name>\n");
		return;
	}

	if (sscanf(Cmd_Argv(1), "%d.%d.%d", &a, &b, &c) == 3)
	{
		Con_Printf("known aliases for %d.%d.%d:\n", a, b, c);
		IPLog_Identify((a << 16) | (b << 8) | c);
		return;
	}

	i = Q_atoi(Cmd_Argv(1)) - 1;
	if (i == -1)
	{
		if (sv.active)
		{
			for (i = 0 ; i < svs.maxclients ; i++)
			{
				if (svs.clients[i].active && unfun_match(Cmd_Argv(1), svs.clients[i].name))
					break;
			}
		}
	}

	if (sv.active)
	{
		if (i < 0 || i >= svs.maxclients || !svs.clients[i].active)
		{
			Con_Printf("No such player\n");
			return;
		}
		if (sscanf(svs.clients[i].netconnection->address, "%d.%d.%d", &a, &b, &c) != 3)
		{
			Con_Printf("Could not determine IP information for %s\n", svs.clients[i].name);
			return;
		}
		strncpy(name, svs.clients[i].name, 15);
		name[15] = 0;
		Con_Printf("known aliases for %s:\n", name);
		IPLog_Identify((a << 16) | (b << 8) | c);
	}
}

//=============================================================================


/*
==================
Host_InitCommands
==================
*/
void Host_InitCommands (void)
{
	Cmd_AddCommand ("status", Host_Status_f);
	Cmd_AddCommand ("cheatfree", Host_Cheatfree_f);	// JPG 3.50 - print cheat-free status
	Cmd_AddCommand ("quit", Host_Quit_f);
	Cmd_AddCommand ("exit", Host_Quit_f); // Slot Zero 3.50-2  Added this.
	Cmd_AddCommand ("god", Host_God_f);
	Cmd_AddCommand ("notarget", Host_Notarget_f);
	Cmd_AddCommand ("fly", Host_Fly_f);
	Cmd_AddCommand ("map", Host_Map_f);
	Cmd_AddCommand ("restart", Host_Restart_f);
	Cmd_AddCommand ("changelevel", Host_Changelevel_f);
	//Cmd_AddCommand ("connect", Host_Connect_f);
	//Cmd_AddCommand ("reconnect", Host_Reconnect_f);
	Cmd_AddCommand ("name", Host_Name_f);
	Cmd_AddCommand ("noclip", Host_Noclip_f);
	Cmd_AddCommand ("version", Host_Version_f);
	Cmd_AddCommand ("say", Host_Say_f);
	Cmd_AddCommand ("say_team", Host_Say_Team_f);
	Cmd_AddCommand ("tell", Host_Tell_f);
	Cmd_AddCommand ("color", Host_Color_f);
	Cmd_AddCommand ("kill", Host_Kill_f);
	Cmd_AddCommand ("pause", Host_Pause_f);
	Cmd_AddCommand ("spawn", Host_Spawn_f);
	Cmd_AddCommand ("begin", Host_Begin_f);
	Cmd_AddCommand ("prespawn", Host_PreSpawn_f);
	Cmd_AddCommand ("kick", Host_Kick_f);
	Cmd_AddCommand ("ping", Host_Ping_f);
	Cmd_AddCommand ("give", Host_Give_f);
	Cmd_AddCommand ("startdemos", Host_Startdemos_f);
	Cmd_AddCommand ("mcache", Mod_Print);
	Cmd_AddCommand ("identify", Host_Identify_f);	// JPG 1.05 - player IP logging
	Cmd_AddCommand ("ipdump", IPLog_Dump);			// JPG 1.05 - player IP logging
	Cmd_AddCommand ("ipmerge", IPLog_Import);		// JPG 3.00 - import an IP data file
}
