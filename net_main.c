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
// net_main.c

#include "quakedef.h"

qsocket_t	*net_activeSockets = NULL;
qsocket_t	*net_freeSockets = NULL;
int			net_numsockets = 0;

qboolean	tcpipAvailable = false;

int			net_hostport;
int			DEFAULTnet_hostport = 26000;
int			DEFAULTnet_clientport = 2600; // single port server
qboolean	single_port_server = false;

char		my_tcpip_address[NET_NAMELEN];

static qboolean	listening = false;

sizebuf_t		net_message;
int				net_activeconnections = 0;

int messagesSent = 0;
int messagesReceived = 0;
int unreliableMessagesSent = 0;
int unreliableMessagesReceived = 0;

cvar_t	net_messagetimeout = {"net_messagetimeout","300"};
cvar_t	net_connecttimeout = {"net_connecttimeout","10"};	// JPG 2.01 - qkick/qflood protection
cvar_t	hostname = {"hostname", "UNNAMED"};
cvar_t	pq_password = {"pq_password", ""};			// JPG 3.00 - password protection
cvar_t	rcon_password = {"rcon_password", ""};			// JPG 3.00 - rcon password
cvar_t	rcon_server = {"rcon_server", ""};			// JPG 3.00 - rcon server
char	server_name[MAX_QPATH];					// JPG 3.50 - use the current server if rcon_server is not set

// Slot Zero 3.50-1  Change client IP.
cvar_t	ip_visible	= {"ip_visible",	"private"};
cvar_t	ip_visible2	= {"ip_visible2",	"private"};
cvar_t	ip_hidden	= {"ip_hidden",		"disabled"};
cvar_t	ip_hidden2	= {"ip_hidden2",	"disabled"};

// JPG 3.00 - rcon
#define RCON_BUFF_SIZE	8192
char		rcon_buff[RCON_BUFF_SIZE];
sizebuf_t	rcon_message = {false, false, rcon_buff, RCON_BUFF_SIZE, 0};
qboolean	rcon_active = false;

qboolean	configRestored = false;

// these two macros are to make the code more readable
#define sfunc	net_drivers[sock->driver]
#define dfunc	net_drivers[net_driverlevel]

int	net_driverlevel;

double			net_time;

double SetNetTime(void)
{
	net_time = Sys_FloatTime();
	return net_time;
}


// JPG 3.00 - need this for linux build
#ifndef _WIN32
unsigned _lrotl (unsigned x, int s)
{
	s &= 31;
	return (x << s) | (x >> (32 - s));
}
unsigned _lrotr (unsigned x, int s)
{
	s &= 31;
	return (x >> s) | (x << (32 - s));
}
#endif


/*
===================
NET_NewQSocket

Called by drivers when a new communications endpoint is required
The sequence and buffer fields will be filled in properly
===================
*/
qsocket_t *NET_NewQSocket (void)
{
	qsocket_t	*sock;

	if (net_freeSockets == NULL)
		return NULL;

	if (net_activeconnections >= svs.maxclients)
		return NULL;

	// get one from free list
	sock = net_freeSockets;
	net_freeSockets = sock->next;

	// add it to active list
	sock->next = net_activeSockets;
	net_activeSockets = sock;

	sock->disconnected = false;
	sock->connecttime = net_time;
	strcpy (sock->address,"UNSET ADDRESS");
	sock->driver = net_driverlevel;
	sock->socket = 0;
	sock->driverdata = NULL;
	sock->canSend = true;
	sock->sendNext = false;
	sock->lastMessageTime = net_time;
	sock->ackSequence = 0;
	sock->sendSequence = 0;
	sock->unreliableSendSequence = 0;
	sock->sendMessageLength = 0;
	sock->receiveSequence = 0;
	sock->unreliableReceiveSequence = 0;
	sock->receiveMessageLength = 0;

	return sock;
}


void NET_FreeQSocket(qsocket_t *sock)
{
	qsocket_t	*s;

	// remove it from active list
	if (sock == net_activeSockets)
		net_activeSockets = net_activeSockets->next;
	else
	{
		for (s = net_activeSockets; s; s = s->next)
			if (s->next == sock)
			{
				s->next = sock->next;
				break;
			}
		if (!s)
			Sys_Error ("NET_FreeQSocket: not active\n");
	}

	// add it to free list
	sock->next = net_freeSockets;
	net_freeSockets = sock;
	sock->disconnected = true;
}


static void NET_Listen_f (void)
{
	if (Cmd_Argc () != 2)
	{
		Con_Printf ("\"listen\" is \"%u\"\n", listening ? 1 : 0);
		return;
	}

	listening = atoi(Cmd_Argv(1)) ? true : false;

	for (net_driverlevel=0 ; net_driverlevel<net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		dfunc.Listen (listening);
	}
}


static void MaxPlayers_f (void)
{
	int 	n;

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("\"maxplayers\" is \"%u\"\n", svs.maxclients);
		return;
	}

	if (sv.active)
	{
		Con_Printf ("maxplayers can not be changed while a server is running.\n");
		return;
	}

	n = atoi(Cmd_Argv(1));
	if (n < 1)
		n = 1;
	if (n > svs.maxclientslimit)
	{
		n = svs.maxclientslimit;
		Con_Printf ("\"maxplayers\" set to \"%u\"\n", n);
	}

	if ((n == 1) && listening)
		Cbuf_AddText ("listen 0\n");

	if ((n > 1) && (!listening))
		Cbuf_AddText ("listen 1\n");

	svs.maxclients = n;
	if (n == 1)
		Cvar_Set ("deathmatch", "0");
	else
		Cvar_Set ("deathmatch", "1");
}


static void NET_Port_f (void)
{
	int 	n;

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("\"port\" is \"%u\"\n", net_hostport);
		return;
	}

	n = atoi(Cmd_Argv(1));
	if (n < 1 || n > 65534)
	{
		Con_Printf ("Bad value, must be between 1 and 65534\n");
		return;
	}

	DEFAULTnet_hostport = n;
	net_hostport = n;

	if (listening)
	{
		// force a change to the new port
		Cbuf_AddText ("listen 0\n");
		Cbuf_AddText ("listen 1\n");
	}
}


// XXX REMOVE THESE
int hostCacheCount = 0;
hostcache_t hostcache[HOSTCACHESIZE];

/*
===================
NET_CheckNewConnections
===================
*/
qsocket_t *NET_CheckNewConnections (void)
{
	qsocket_t	*ret;

	SetNetTime();

	for (net_driverlevel=0 ; net_driverlevel<net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == false)
			continue;
		if (net_driverlevel && listening == false)
			continue;
		ret = dfunc.CheckNewConnections ();
		if (ret)
			return ret;
	}

	return NULL;
}

/*
===================
NET_Close
===================
*/
void NET_Close (qsocket_t *sock)
{
	if (!sock)
		return;

	if (sock->disconnected)
		return;

	SetNetTime();

	// call the driver_Close function
	sfunc.Close (sock);

	NET_FreeQSocket(sock);
}


/*
=================
NET_GetMessage

If there is a complete message, return it in net_message

returns 0 if no data is waiting
returns 1 if a message was received
returns -1 if connection is invalid
=================
*/

extern void PrintStats(qsocket_t *s);

int	NET_GetMessage (qsocket_t *sock)
{
	int ret;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_DPrintf("NET_GetMessage: disconnected socket\n");
		return -1;
	}

	SetNetTime();

	ret = sfunc.QGetMessage(sock);

	// see if this connection has timed out
	if (ret == 0 && sock->driver)
	{
		if (net_time - sock->lastMessageTime > net_messagetimeout.value)
		{
			NET_Close(sock);
			return -1;
		}

		// JPG 2.01 - qflood/qkick protection
		if (net_time - sock->lastMessageTime > net_connecttimeout.value && sv.active &&
			host_client && sock == host_client->netconnection && !strcmp(host_client->name, "unconnected"))
		{
			NET_Close(sock);
			return -1;
		}
	}


	if (ret > 0)
	{
		// JPG 3.20 - cheat free
		if (pq_cheatfree && sock->mod != MOD_QSMACK && (sock->mod_version < 35 || sock->encrypt))
		{
			// Con_Printf("NET_Decrypt\n");
			Security_Decode(net_message.data, net_message.data, net_message.cursize, sock->client_port);
		}

		if (sock->driver)
		{
			sock->lastMessageTime = net_time;
			if (ret == 1)
				messagesReceived++;
			else if (ret == 2)
				unreliableMessagesReceived++;
		}
	}

	return ret;
}


/*
==================
NET_SendMessage

Try to send a complete length+message unit over the reliable stream.
returns 0 if the message cannot be delivered reliably, but the connection
		is still considered valid
returns 1 if the message was sent properly
returns -1 if the connection died
==================
*/

// JPG 3.20 - cheat free
byte buff[NET_MAXMESSAGE];
sizebuf_t newdata;

int NET_SendMessage (qsocket_t *sock, sizebuf_t *data)
{
	int		r;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("NET_SendMessage: disconnected socket\n");
		return -1;
	}

	// JPG 3.20 - cheat free
	if (pq_cheatfree && sock->mod != MOD_QSMACK)
	{
		if (sock->mod_version < 35 || sock->encrypt == 1 || sock->encrypt == 2)	// JPG 3.50
		{
			// Con_Printf("NET_Encrypt\n");
			Security_Encode(data->data, buff, data->cursize, sock->client_port);
			newdata.data = buff;
			newdata.cursize = data->cursize;
			data = &newdata;
		}
		if (sock->encrypt == 1)
			sock->encrypt = 0;
		else if (sock->encrypt == 3)
			sock->encrypt = 2;
	}

	SetNetTime();
	r = sfunc.QSendMessage(sock, data);
	if (r == 1 && sock->driver)
		messagesSent++;

	return r;
}

int NET_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data)
{
	int		r;

	if (!sock)
		return -1;

	if (sock->disconnected)
	{
		Con_Printf("NET_SendMessage: disconnected socket\n");
		return -1;
	}

	// JPG 3.20 - cheat free
	if (pq_cheatfree && sock->mod != MOD_QSMACK)
	{
		if (sock->mod_version < 35 || sock->encrypt == 1 || sock->encrypt == 2)	// JPG 3.50
		{
			// Con_Printf("NET_EncryptUnreliable\n");
			Security_Encode(data->data, buff, data->cursize, sock->client_port);
			newdata.data = buff;
			newdata.cursize = data->cursize;
			data = &newdata;
		}
		if (sock->encrypt == 1)
			sock->encrypt = 0;
		else if (sock->encrypt == 3)
			sock->encrypt = 2;
	}

	SetNetTime();
	r = sfunc.SendUnreliableMessage(sock, data);
	if (r == 1 && sock->driver)
		unreliableMessagesSent++;

	return r;
}


/*
==================
NET_CanSendMessage

Returns true or false if the given qsocket can currently accept a
message to be transmitted.
==================
*/
qboolean NET_CanSendMessage (qsocket_t *sock)
{
	int		r;

	if (!sock)
		return false;

	if (sock->disconnected)
		return false;

	SetNetTime();

	r = sfunc.CanSendMessage(sock);

	return r;
}


int NET_SendToAll(sizebuf_t *data, int blocktime)
{
	double		start;
	int			i;
	int			count = 0;
	qboolean	state1 [MAX_SCOREBOARD];
	qboolean	state2 [MAX_SCOREBOARD];

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->netconnection)
			continue;
		if (host_client->active)
		{
			if (host_client->netconnection->driver == 0)
			{
				NET_SendMessage(host_client->netconnection, data);
				state1[i] = true;
				state2[i] = true;
				continue;
			}
			count++;
			state1[i] = false;
			state2[i] = false;
		}
		else
		{
			state1[i] = true;
			state2[i] = true;
		}
	}

	start = Sys_FloatTime();
	while (count)
	{
		count = 0;
		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (! state1[i])
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					state1[i] = true;
					NET_SendMessage(host_client->netconnection, data);
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
				}
				count++;
				continue;
			}

			if (! state2[i])
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					state2[i] = true;
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
				}
				count++;
				continue;
			}
		}
		if ((Sys_FloatTime() - start) > blocktime)
			break;
	}
	return count;
}


//=============================================================================


/*
====================
NET_Init
====================
*/
void NET_Init (void)
{
	int			i;
	int			controlSocket;
	qsocket_t	*s;

	if (i = COM_CheckParm ("-ssp"))
	{
		single_port_server = true;

		if (i < com_argc-1 && atoi(com_argv[i+1]))
			DEFAULTnet_clientport = atoi (com_argv[i+1]);
	}

	i = COM_CheckParm ("-port");
	if (!i)
		i = COM_CheckParm ("-udpport");

	if (i)
	{
		if (i < com_argc-1 && atoi(com_argv[i+1]))
			DEFAULTnet_hostport = atoi (com_argv[i+1]);
		else
			Sys_Error ("NET_Init: you must specify a number after -port");
	}

	net_hostport = DEFAULTnet_hostport;

	listening = true;
	net_numsockets = svs.maxclientslimit;

	SetNetTime();

	for (i = 0; i < net_numsockets; i++)
	{
		s = (qsocket_t *)Hunk_AllocName(sizeof(qsocket_t), "qsocket");
		s->next = net_freeSockets;
		net_freeSockets = s;
		s->disconnected = true;
#ifdef _WIN32
		s->client_port	= DEFAULTnet_clientport + i; // single port server
#endif
	}

	// allocate space for network message buffer
	SZ_Alloc (&net_message, NET_MAXMESSAGE);

	Cvar_RegisterVariable (&net_messagetimeout);
	Cvar_RegisterVariable (&net_connecttimeout);	// JPG 2.01 - qkick/qflood protection
	Cvar_RegisterVariable (&hostname);
	Cvar_RegisterVariable (&pq_password);		// JPG 3.00 - password protection
	Cvar_RegisterVariable (&rcon_password);		// JPG 3.00 - rcon password
	Cvar_RegisterVariable (&rcon_server);		// JPG 3.00 - rcon server

	// Slot Zero 3.50-1  Change client IP.
	Cvar_RegisterVariable (&ip_visible);
	Cvar_RegisterVariable (&ip_visible2);
	Cvar_RegisterVariable (&ip_hidden);
	Cvar_RegisterVariable (&ip_hidden2);

	Cmd_AddCommand ("listen", NET_Listen_f);
	Cmd_AddCommand ("maxplayers", MaxPlayers_f);
	Cmd_AddCommand ("port", NET_Port_f);

	// initialize all the drivers
	for (net_driverlevel=0 ; net_driverlevel<net_numdrivers ; net_driverlevel++)
	{
		controlSocket = net_drivers[net_driverlevel].Init();
		if (controlSocket == -1)
			continue;
		net_drivers[net_driverlevel].initialized = true;
		net_drivers[net_driverlevel].controlSock = controlSocket;
		if (listening)
			net_drivers[net_driverlevel].Listen (true);
	}

	if (*my_tcpip_address)
		Con_DPrintf("TCP/IP address %s\n", my_tcpip_address);

	// JPG 3.20 - cheat free
	if (pq_cheatfreeEnabled)
	{
		net_seed = rand() ^ (rand() << 10) ^ (rand() << 20);
		net_seed &= 0x7fffffff;
		if (net_seed == 0x7fffffff)
			net_seed = 0;
		net_seed |= 1;
		if (net_seed <= 1)
			net_seed = 0x34719;
		Security_SetSeed(net_seed, argv[0]);
	}
}


/*
====================
NET_Shutdown
====================
*/

void NET_Shutdown (void)
{
	qsocket_t	*sock;

	SetNetTime();

	for (sock = net_activeSockets; sock; sock = sock->next)
		NET_Close(sock);

//
// shutdown the drivers
//
	for (net_driverlevel = 0; net_driverlevel < net_numdrivers; net_driverlevel++)
	{
		if (net_drivers[net_driverlevel].initialized == true)
		{
			net_drivers[net_driverlevel].Shutdown ();
			net_drivers[net_driverlevel].initialized = false;
		}
	}
}


static PollProcedure *pollProcedureList = NULL;

void NET_Poll(void)
{
	PollProcedure *pp;

	if (!configRestored)
		configRestored = true;

	SetNetTime();

	for (pp = pollProcedureList; pp; pp = pp->next)
	{
		if (pp->nextTime > net_time)
			break;
		pollProcedureList = pp->next;
		pp->procedure(pp->arg);
	}
}


void SchedulePollProcedure(PollProcedure *proc, double timeOffset)
{
	PollProcedure *pp, *prev;

	proc->nextTime = Sys_FloatTime() + timeOffset;
	for (pp = pollProcedureList, prev = NULL; pp; pp = pp->next)
	{
		if (pp->nextTime >= proc->nextTime)
			break;
		prev = pp;
	}

	if (prev == NULL)
	{
		proc->next = pollProcedureList;
		pollProcedureList = proc;
		return;
	}

	proc->next = pp;
	prev->next = proc;
}
