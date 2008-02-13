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

sys_unixd.h -- a dedicated unix server

*/

#include "quakedef.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>


#define TRUE 1
#define UNIXDED_VERSION 1.00

qboolean                    isDedicated;

int nostdout = 0;

/*
===============================================================================

FILE IO

===============================================================================
*/

#define	MAX_HANDLES		10
FILE	*sys_handles[MAX_HANDLES];

int		findhandle (void)
{
	int		i;

	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE	*f;
	int		i;

	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;

	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE	*f;
	int		i;

	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;

	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int	Sys_FileTime (char *path)
{
	FILE	*f;

	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}

	return -1;
}

void Sys_mkdir (char *path)
{
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_DebugLog(char *file, char *fmt, ...)
{
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

//    MessageBox(NULL, text, "Error", 0 /* MB_OK */ );
	printf ("ERROR: %s\n", text);

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[2048];	// JPG 3.30 - changed this from 1024 to 2048
	unsigned char		*p;

	va_start (argptr,fmt);
	vsprintf (text,fmt,argptr);
	va_end (argptr);

	// JPG 1.05 - translate to plain text
	if (pq_dequake.value)
	{
		unsigned char *ch;
		for (ch = text ; *ch ; ch++)
			*ch = dequake[*ch];
	}

	if (strlen(text) > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");

    if (nostdout)
        return;

	for (p = (unsigned char *)text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}
void Sys_Quit (void)
{
	Host_Shutdown();
    if (!nostdout){
        fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	    fflush(stdout);
    }
	exit(0);
}

double Sys_FloatTime (void)
{                                                                                   struct timeval tp;                                                              struct timezone tzp;                                                            static int      secbase;                                                                                                                                        gettimeofday(&tp, &tzp);                                                                                                                                        if (!secbase)                                                                   {                                                                                   secbase = tp.tv_sec;                                                            return tp.tv_usec/1000000.0;
        }

    return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}

void Sys_Sleep (void)
{
}


void Sys_SendKeyEvents (void)
{
}

char *Sys_ConsoleInput (void)
{
    static char text[256];
    int     len;
    fd_set  fdset;
    struct timeval timeout;

        FD_ZERO(&fdset);
        FD_SET(0, &fdset); // stdin
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        if (select (1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
            return NULL;

        len = read (0, text, sizeof(text));
        if (len < 1)
            return NULL;
        text[len-1] = 0;    // rip off the /n and terminate

        return text;
}



/*
==================
main

==================
*/
char	*newargv[256];
char    *basedir = ".";

int main (int argc, char **argv)
{
	quakeparms_t	parms;
	double			time, oldtime, newtime;
	static	char	cwd[1024];
    int j;

	memset (&parms, 0, sizeof(parms));

	COM_InitArgv (argc, argv);

	parms.memsize = 16*1024*1024;

	if (j = COM_CheckParm("-mem"))
		parms.memsize = (int) (Q_atof(com_argv[j+1]) * 1024 * 1024);

	parms.membase = malloc (parms.memsize);
	parms.basedir = basedir;

	// dedicated server ONLY!
    isDedicated = TRUE;
	if (!COM_CheckParm ("-dedicated"))
	{
		memcpy (newargv, argv, argc*4);
		newargv[argc] = "-dedicated";
		argc++;
		argv = newargv;
		COM_InitArgv (argc, argv);
	}

	parms.argc = argc;
	parms.argv = argv;

	Host_Init (&parms);

	if (COM_CheckParm("-nostdout"))
		nostdout = 1;
	else
    {
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
		printf ("Unixded Quake -- Version %0.3f\n", UNIXDED_VERSION);
	}

	oldtime = Sys_FloatTime () - sys_ticrate.value;

    /* main server message loop */
	while (1)
    {
        newtime = Sys_FloatTime ();
        time = newtime - oldtime;

        if (time < sys_ticrate.value )
        {
            usleep(1);
            continue;       // not time to run a server only tic yet
        }

        Host_Frame (time);

        oldtime = newtime;
    }
    /* return success of application */
    return TRUE;
}

#if !id386
void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}
#endif
