#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>

#include "quakedef.h"

int nostdout = 0;

char *basedir = ".";
char *cachedir = "/tmp";

/*
===============================================================================

SYNCHRONIZATION - JPG 3.30

===============================================================================
*/

int hlock;

/*
================
Sys_GetLock
================
*/
void Sys_GetLock (void)
{
	int i;

	for (i = 0 ; i < 10 ; i++)
	{
		hlock = open(va("%s/lock.dat",com_gamedir), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
		if (hlock != -1)
			return;
		sleep(1);
	}

	Sys_Printf("Warning: could not open lock; using crowbar\n");
}


/*
================
Sys_ReleaseLock
================
*/
void Sys_ReleaseLock (void)
{
	if (hlock != -1)
		close(hlock);
	unlink(va("%s/lock.dat",com_gamedir));
}

// =======================================================================
// General routines
// =======================================================================

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[4096];		// JPG 3.30 - increased this from 1024 to 2048
								// Slot Zero 3.50-2  Increased this from 2048 to 4096.
	unsigned char		*p;

    // Slot Zero 3.50-2  Memory overwrite protection for Sys_Printf.
    va_start (argptr, fmt);
	dpvsnprintf (text, sizeof(text), fmt, argptr);
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

    // Slot Zero 3.50-2  QIP: Piped output of a dedicated server not written immediately fix.
    fflush(stdout);

	// JPG 3.00 - rcon (64 doesn't mean anything special, but we need some extra space because NET_MAXMESSAGE == RCON_BUFF_SIZE)
	if (rcon_active && (rcon_message.cursize < rcon_message.maxsize - strlen(text) - 64))
	{
		rcon_message.cursize--;
		MSG_WriteString(&rcon_message, text);
	}
}


void Sys_Quit (void)
{
	Host_Shutdown();
    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	fflush(stdout);
	exit(0);
}


void Sys_Init(void)
{
#if id386
	Sys_SetFPCW();
#endif
}


void Sys_Error (char *error, ...)
{
    va_list     argptr;
    char        string[1024];

	// change stdin to non blocking
    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

    va_start (argptr,error);
    dpvsnprintf (string, sizeof(string), error, argptr); // Slot Zero 3.50-2
    va_end (argptr);
	fprintf(stderr, "Error: %s\n", string);

	Host_Shutdown ();
	exit (1);

}


/*
============
Sys_FileTime

returns -1 if not present
============
*/
int	Sys_FileTime (char *path)
{
	struct	stat	buf;

	if (stat (path,&buf) == -1)
		return -1;

	return buf.st_mtime;
}


void Sys_mkdir (char *path)
{
    mkdir (path, 0777);
}


int Sys_FileOpenRead (char *path, int *handle)
{
	int	h;
	struct stat	fileinfo;


	h = open (path, O_RDONLY, 0666);
	*handle = h;
	if (h == -1)
		return -1;

	if (fstat (h,&fileinfo) == -1)
		Sys_Error ("Error fstating %s", path);

	return fileinfo.st_size;
}


int Sys_FileOpenWrite (char *path)
{
	int     handle;

	umask (0);

	handle = open(path,O_RDWR | O_CREAT | O_TRUNC, 0666);

	if (handle == -1)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));

	return handle;
}


int Sys_FileWrite (int handle, void *src, int count)
{
	return write (handle, src, count);
}


void Sys_FileClose (int handle)
{
	close (handle);
}


void Sys_FileSeek (int handle, int position)
{
	lseek (handle, position, SEEK_SET);
}


int Sys_FileRead (int handle, void *dest, int count)
{
    return read (handle, dest, count);
}


double Sys_FloatTime (void)
{
    struct timeval tp;
    struct timezone tzp;
    static int      secbase;

    gettimeofday(&tp, &tzp);

    if (!secbase)
    {
        secbase = tp.tv_sec;
        return tp.tv_usec/1000000.0;
    }

    return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}

// =======================================================================
// Sleeps for microseconds
// =======================================================================

char *Sys_ConsoleInput(void)
{
    static char text[256];
    int     len;
	fd_set	fdset;
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

char **argv;	// JPG 3.00 - need this for exe filename

int main (int c, char **v)
{
	double		time, oldtime, newtime;
	quakeparms_t parms;
	int j;

//	static char cwd[1024];

//	signal(SIGFPE, floating_point_exception_handler);
	signal(SIGFPE, SIG_IGN);

	memset(&parms, 0, sizeof(parms));

	COM_InitArgv(c, v);
	parms.argc = com_argc;
	parms.argv = com_argv;
	argv = v;

	parms.memsize = 8*1024*1024;

	j = COM_CheckParm("-mem");
	if (j)
		parms.memsize = (int) (atof(com_argv[j+1]) * 1024 * 1024);
	parms.membase = malloc (parms.memsize);

	parms.basedir = basedir;
	// caching is disabled by default, use -cachedir to enable
	//	parms.cachedir = cachedir;

	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

    Host_Init(&parms);

	Sys_Init();

	if (COM_CheckParm("-nostdout"))
		nostdout = 1;
	else
	{
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
		printf ("Linux ProQuake Version %4.2f\n", PROQUAKE_VERSION);
	}

    oldtime = Sys_FloatTime () - 0.1;
    while (1)
    {
		// find time spent rendering last frame
        newtime = Sys_FloatTime ();
        time = newtime - oldtime;

		if (time < sys_ticrate.value)
		{
			usleep(1);
			continue; // not time to run a server only tic yet
		}

		time = sys_ticrate.value;

        if (time > sys_ticrate.value*2)
            oldtime = newtime;
        else
            oldtime += time;

        Host_Frame (time);
	}
}


/*
=================
Sys_SendKeyEvents
=================
*/
void Sys_SendKeyEvents(void)
{
}
