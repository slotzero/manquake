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
// console.c

#ifdef NeXT
#include <libc.h>
#endif
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <fcntl.h>
#include "quakedef.h"

#include <time.h> // JPG - needed for console log

int 		con_linewidth;

float		con_cursorspeed = 4;

// JPG - upped CON_TEXTSIZE from 16384 to 65536
#define		CON_TEXTSIZE	65536

qboolean 	con_forcedup;		// because no entities to refresh

int			con_totallines;		// total lines in console scrollback
int			con_backscroll;		// lines up from bottom to display
int			con_current;		// where next message will be printed
int			con_x;				// offset in current line for next print
char		*con_text=0;

#define	NUM_CON_TIMES 4
float		con_times[NUM_CON_TIMES];	// realtime time the line was generated
								// for transparent notify lines

int			con_vislines;

qboolean	con_debuglog;

#define		MAXCMDLINE	256
extern	char	key_lines[32][MAXCMDLINE];
extern	int		key_linepos;

qboolean	con_initialized;

char logfilename[128];	// JPG - support for different filenames


/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	if (con_text)
		memset (con_text, ' ', CON_TEXTSIZE);
}


/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;

	for (i=0 ; i<NUM_CON_TIMES ; i++)
		con_times[i] = 0;
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE];

	width = (0 >> 3) - 2;

	if (width == con_linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 38;
		con_linewidth = width;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		memset (con_text, ' ', CON_TEXTSIZE);
	}
	else
	{
		oldwidth = con_linewidth;
		con_linewidth = width;
		oldtotallines = con_totallines;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		numlines = oldtotallines;

		if (con_totallines < numlines)
			numlines = con_totallines;

		numchars = oldwidth;

		if (con_linewidth < numchars)
			numchars = con_linewidth;

		memcpy (tbuf, con_text, CON_TEXTSIZE);
		memset (con_text, ' ', CON_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con_text[(con_totallines - 1 - i) * con_linewidth + j] =
						tbuf[((con_current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con_backscroll = 0;
	con_current = con_totallines - 1;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
#define MAXGAMEDIRLEN	1000
	char	temp[MAXGAMEDIRLEN+1];
	// char	*t2 = "/qconsole.log"; // JPG - don't need this
	char	*ch;	// JPG - added this
	int		fd, n;	// JPG - added these
    time_t  ltime;		// JPG - for console log file

	con_debuglog = COM_CheckParm("-condebug");

	if (con_debuglog)
	{
		// JPG - check for different file name
		if ((con_debuglog < com_argc - 1) && (*com_argv[con_debuglog+1] != '-') && (*com_argv[con_debuglog+1] != '+'))
		{
			if ((ch = strchr(com_argv[con_debuglog+1], '%')) && (ch[1] == 'd'))
			{
				n = 0;
				do
				{
					n = n + 1;
					dpsnprintf(logfilename, sizeof(logfilename), com_argv[con_debuglog+1], n);
					strcat(logfilename, ".log");
					dpsnprintf(temp, sizeof(temp), "%s/%s", com_gamedir, logfilename);
					fd = open(temp, O_CREAT | O_EXCL | O_WRONLY, 0666);
				}
				while (fd == -1);
				close(fd);
			}
			else
				dpsnprintf(logfilename, sizeof(logfilename), "%s.log", com_argv[con_debuglog+1]);
		}
		else
			strcpy(logfilename, "qconsole.log");

		// JPG - changed t2 to logfilename
		if (strlen (com_gamedir) < (MAXGAMEDIRLEN - strlen (logfilename)))
		{
			dpsnprintf (temp, sizeof(temp), "%s/%s", com_gamedir, logfilename); // JPG - added the '/'
			unlink (temp);
		}

		// JPG - print initial message
		Con_Printf("Log file initialized.\n");
		Con_Printf("%s/%s\n", com_gamedir, logfilename);
		time( &ltime );
		Con_Printf( "%s\n", ctime( &ltime ) );
	}

	con_text = Hunk_AllocName (CON_TEXTSIZE, "context");
	memset (con_text, ' ', CON_TEXTSIZE);
	con_linewidth = -1;
	Con_CheckResize ();

	Con_Printf ("Console initialized.\n");

//
// register our commands
//
	Cmd_AddCommand ("clear", Con_Clear_f);
	con_initialized = true;
}

#define DIGIT(x) ((x) >= '0' && (x) <= '9')

// JPG - increased this from 4096 to 16384 and moved it up here
// See http://www.inside3d.com/qip/q1/bugs.htm, NVidia 5.16 drivers can cause crash
#define	MAXPRINTMSG	16384

/* JPG - took *file out of the argument list and used logfilename instead
================
Con_DebugLog
================
*/
void Con_DebugLog(char *fmt, ...)
{
    va_list argptr;
    static char data[MAXPRINTMSG];	// JPG 3.02 - changed from 1024 to MAXPRINTMSG
    int fd;

    va_start(argptr, fmt);
    dpvsnprintf (data, sizeof(data), fmt, argptr);
    va_end(argptr);
    fd = open(va("%s/%s", com_gamedir, logfilename), O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fd, data, strlen(data));
    close(fd);
}


/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
================
*/
void Con_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;

	va_start (argptr,fmt);
	dpvsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

// also echo to debugging console
	Sys_Printf ("%s", msg);	// also echo to debugging console

// log all messages to file
	if (con_debuglog)
		Con_DebugLog("%s", msg);  // JPG - got rid of filename

	if (!con_initialized)
		return;
}


/*
================
Con_DPrintf

A Con_Printf that only shows up if the "developer" cvar is set
================
*/
void Con_DPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!developer.value)
		return;			// don't confuse non-developers with techie stuff...

	va_start (argptr,fmt);
	dpvsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Con_Printf ("%s", msg);
}


/*
==================
Con_SafePrintf

Okay to call even when the screen can't be updated
==================
*/
void Con_SafePrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[1024];

	va_start (argptr,fmt);
	dpvsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Con_Printf ("%s", msg);
}
