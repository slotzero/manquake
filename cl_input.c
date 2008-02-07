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
// cl.input.c  -- builds an intended movement command to send to the server

// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include "quakedef.h"

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition

===============================================================================
*/


kbutton_t	in_mlook, in_klook;
kbutton_t	in_left, in_right, in_forward, in_back;
kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t	in_strafe, in_speed, in_use, in_jump, in_attack;
kbutton_t	in_up, in_down;

int			in_impulse;

/*
===============
CL_KeyState

Returns 0.25 if a key was pressed and released during the frame,
0.5 if it was pressed and held
0 if held then released, and
1.0 if held for the entire time
===============
*/
float CL_KeyState (kbutton_t *key)
{
	float		val;
	qboolean	impulsedown, impulseup, down;

	impulsedown = key->state & 2;
	impulseup = key->state & 4;
	down = key->state & 1;
	val = 0;

	if (impulsedown && !impulseup)
		if (down)
			val = 0.5;	// pressed and held this frame
		else
			val = 0;	//	I_Error ();
	if (impulseup && !impulsedown)
		if (down)
			val = 0;	//	I_Error ();
		else
			val = 0;	// released this frame
	if (!impulsedown && !impulseup)
		if (down)
			val = 1.0;	// held the entire frame
		else
			val = 0;	// up the entire frame
	if (impulsedown && impulseup)
		if (down)
			val = 0.75;	// released and re-pressed this frame
		else
			val = 0.25;	// pressed and released this frame

	key->state &= 1;		// clear impulses

	return val;
}




//==========================================================================

cvar_t	cl_upspeed = {"cl_upspeed","200"};
cvar_t	cl_forwardspeed = {"cl_forwardspeed","200", true};
cvar_t	cl_backspeed = {"cl_backspeed","200", true};
cvar_t	cl_sidespeed = {"cl_sidespeed","350"};

cvar_t	cl_movespeedkey = {"cl_movespeedkey","2.0"};

cvar_t	cl_yawspeed = {"cl_yawspeed","140"};
cvar_t	cl_pitchspeed = {"cl_pitchspeed","150"};

cvar_t	cl_anglespeedkey = {"cl_anglespeedkey","1.5"};

cvar_t	pq_lag = {"pq_lag", "0"};			// JPG - synthetic lag
cvar_t	cl_fullpitch = {"cl_fullpitch", "0"};	// JPG 2.01 - get rid of the "unknown command" messages

/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
void CL_AdjustAngles (void)
{
	float	speed;
	float	up, down;

	if (in_speed.state & 1)
		speed = host_frametime * cl_anglespeedkey.value;
	else
		speed = host_frametime;

	if (!(in_strafe.state & 1))
	{
		cl.viewangles[YAW] -= speed*cl_yawspeed.value*CL_KeyState (&in_right);
		cl.viewangles[YAW] += speed*cl_yawspeed.value*CL_KeyState (&in_left);
		cl.viewangles[YAW] = anglemod(cl.viewangles[YAW]);
	}
	if (in_klook.state & 1)
	{
		V_StopPitchDrift ();
		cl.viewangles[PITCH] -= speed*cl_pitchspeed.value * CL_KeyState (&in_forward);
		cl.viewangles[PITCH] += speed*cl_pitchspeed.value * CL_KeyState (&in_back);
	}

	up = CL_KeyState (&in_lookup);
	down = CL_KeyState(&in_lookdown);

	cl.viewangles[PITCH] -= speed*cl_pitchspeed.value * up;
	cl.viewangles[PITCH] += speed*cl_pitchspeed.value * down;

	if (up || down)
		V_StopPitchDrift ();

	// JPG 1.05 - add pq_fullpitch
	if (pq_fullpitch.value)
	{
		if (cl.viewangles[PITCH] > 90)
			cl.viewangles[PITCH] = 90;
		if (cl.viewangles[PITCH] < -90)
			cl.viewangles[PITCH] = -90;
	}
	else
	{
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}

	if (cl.viewangles[ROLL] > 50)
		cl.viewangles[ROLL] = 50;
	if (cl.viewangles[ROLL] < -50)
		cl.viewangles[ROLL] = -50;

}

/*
================
CL_BaseMove

Send the intended movement message to the server
================
*/
void CL_BaseMove (usercmd_t *cmd)
{
	if (cls.signon != SIGNONS)
		return;

	CL_AdjustAngles ();

	Q_memset (cmd, 0, sizeof(*cmd));

	if (in_strafe.state & 1)
	{
		cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_right);
		cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_left);
	}

	cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_moveright);
	cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_moveleft);

	cmd->upmove += cl_upspeed.value * CL_KeyState (&in_up);
	cmd->upmove -= cl_upspeed.value * CL_KeyState (&in_down);

	if (! (in_klook.state & 1) )
	{
		cmd->forwardmove += cl_forwardspeed.value * CL_KeyState (&in_forward);
		cmd->forwardmove -= cl_backspeed.value * CL_KeyState (&in_back);
	}

//
// adjust for speed key
//
	if (in_speed.state & 1)
	{
		cmd->forwardmove *= cl_movespeedkey.value;
		cmd->sidemove *= cl_movespeedkey.value;
		cmd->upmove *= cl_movespeedkey.value;
	}
}

sizebuf_t lag_buff[32]; // JPG - support for synthetic lag
byte lag_data[32][128];  // JPG - support for synthetic lag
unsigned int lag_head, lag_tail; // JPG - support for synthetic lag
double lag_sendtime[32]; // JPG - support for synthetic lag


/* JPG - this function sends delayed move messages
==============
CL_SendLagMove
==============
*/
void CL_SendLagMove (void)
{
	if (cls.demoplayback || (cls.state != ca_connected) || (cls.signon != SIGNONS))
		return;

	while ((lag_tail < lag_head) && (lag_sendtime[lag_tail & 31] <= realtime))
	{
		lag_tail++;
		if (++cl.movemessages <= 2)
		{
			lag_head = lag_tail = 0;  // JPG - hack: if cl.movemessages has been reset, we should reset these too
			continue;	// return -> continue
		}

		if (NET_SendUnreliableMessage (cls.netcon, &lag_buff[(lag_tail-1)&31]) == -1)
		{
			Con_Printf ("CL_SendMove: lost server connection\n");
			CL_Disconnect ();
		}
	}
}

/*
==============
CL_SendMove
==============
*/
void CL_SendMove (usercmd_t *cmd)
{
	int		i;
	int		bits;
	sizebuf_t *buf;	// JPG - turned into a pointer (made corresponding changes below)
	static byte	data[128]; // JPG - replaced with lag_data

	buf = &lag_buff[lag_head & 31];
	buf->maxsize = 128;
	buf->cursize = 0;
	buf->data = lag_data[lag_head & 31]; // JPG - added head index
	lag_sendtime[lag_head++ & 31] = realtime + (pq_lag.value / 1000.0);

	cl.cmd = *cmd;

//
// send the movement message
//
    MSG_WriteByte (buf, clc_move);

	MSG_WriteFloat (buf, cl.mtime[0]);	// so server can get ping times

	if (!cls.demoplayback && (cls.netcon->mod == MOD_PROQUAKE)) // JPG - precise aim for ProQuake!
	{
		for (i=0 ; i<3 ; i++)
			MSG_WritePreciseAngle (buf, cl.viewangles[i]);
	}
	else
	{
		for (i=0 ; i<3 ; i++)
			MSG_WriteAngle (buf, cl.viewangles[i]);
	}

    MSG_WriteShort (buf, cmd->forwardmove);
    MSG_WriteShort (buf, cmd->sidemove);
    MSG_WriteShort (buf, cmd->upmove);

//
// send button bits
//
	bits = 0;

	if ( in_attack.state & 3 )
		bits |= 1;
	in_attack.state &= ~2;

	if (in_jump.state & 3)
		bits |= 2;
	in_jump.state &= ~2;

    MSG_WriteByte (buf, bits);

    MSG_WriteByte (buf, in_impulse);
	in_impulse = 0;

//
// deliver the message
//
	if (cls.demoplayback)
		return;

//
// allways dump the first two message, because it may contain leftover inputs
// from the last level
//

	// JPG - replaced this with a call to CL_SendLagMove
	/*
	if (++cl.movemessages <= 2)
		return;

	if (NET_SendUnreliableMessage (cls.netcon, &buf) == -1)
	{
		Con_Printf ("CL_SendMove: lost server connection\n");
		CL_Disconnect ();
	}
	*/
	CL_SendLagMove();
}
