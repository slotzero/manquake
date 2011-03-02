/*

    Copyright (C) 2011  David 'Slot Zero' Roberts.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "quakedef.h"

banlog_t *banlogs;
banlog_t *banlog_head;

int banlog_size;
int banlog_next;
int banlog_full;

#define DEFAULT_BANLOGSIZE	0x1000

/*
====================
BANLog_Init
====================
*/
void BANLog_Init (void)
{
	int p;
	FILE *f;
	banlog_t temp;

	// Allocate space for the BAN logs
	banlog_size = 0;
	if (COM_CheckParm ("-nobanlog"))
		return;
	p = COM_CheckParm ("-banlogsize");
	if (p && p < com_argc - 1)
		banlog_size = atoi(com_argv[p+1]) * 1024 / sizeof(banlog_t);
	if (!banlog_size)
		banlog_size = DEFAULT_BANLOGSIZE;

	banlogs = (banlog_t *) Hunk_AllocName(banlog_size * sizeof(banlog_t), "banlog");
	banlog_next = 0;
	banlog_head = NULL;
	banlog_full = 0;

	// Attempt to load log data from banlog.dat
	Sys_GetLock();
	f = fopen(va("%s/banlog.dat",com_gamedir), "r");
	if (f)
	{
		while(fread(&temp, 20, 1, f))
			BANLog_Add(temp.addr, temp.name);
		fclose(f);
	}
	Sys_ReleaseLock();
}

/*
====================
BANLog_Import
====================
*/
void BANLog_Import (void)
{
	FILE *f;
	banlog_t temp;

	if (cmd_source == src_client)
		return;

	if (!banlog_size)
	{
		Con_Printf("BAN logging not available\nRemove -nobanlog command line option\n");
		return;
	}

	if (Cmd_Argc() < 2)
	{
		Con_Printf("Usage: banmerge <filename>\n");
		return;
	}
	f = fopen(va("%s", Cmd_Argv(1)), "r");
	if (f)
	{
		while(fread(&temp, 20, 1, f))
			BANLog_Add(temp.addr, temp.name);
		fclose(f);
		Con_Printf("Merged %s\n", Cmd_Argv(1));
	}
	else
		Con_Printf("Could not open %s\n", Cmd_Argv(1));
}

/*
====================
BANLog_WriteLog
====================
*/
void BANLog_WriteLog (void)
{
	FILE *f;
	int i;
	banlog_t temp;

	if (!banlog_size)
		return;

	Sys_GetLock();

	f = fopen(va("%s/banlog.dat",com_gamedir), "w");
	if (f)
	{
		if (banlog_full)
		{
			for (i = banlog_next + 1 ; i < banlog_size ; i++)
			{
				if (strcmp (banlogs[i].name, "")) // hack
					fwrite(&banlogs[i], 20, 1, f);
			}
		}
		for (i = 0 ; i < banlog_next ; i++)
		{
			if (strcmp (banlogs[i].name, "")) // hack
				fwrite(&banlogs[i], 20, 1, f);
		}

		fclose(f);
		Con_Printf("Wrote banlog.dat\n");
	}
	else
		Con_Printf("Could not write banlog.dat\n");

	Sys_ReleaseLock();
}


/*
====================
BANLog_Add
====================
*/
void BANLog_Add (int addr, char *name)
{
	void (*print) (char *fmt, ...);
	banlog_t *banlog_new;
	banlog_t **ppnew;
	banlog_t *parent;
	char name2[16];
	char *ch;
	int a,b,c;

	if (!banlog_size)
		return;

	if (cmd_source == src_client && sv.active)
		print = SV_ClientPrintf;
	else
		print = Con_Printf;


	a = addr >> 16;
	b = (addr >> 8) & 0xff;
	c = addr & 0xff;
	if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255)
	{
		print ("ip address [%d.%d.%d.xxx] out of range\n", a, b, c);
		return;
	}

	// delete trailing spaces
	strncpy(name2, name, 15);
	ch = &name2[15];
	*ch = 0;
	while (ch >= name2 && (*ch == 0 || *ch == ' '))
		*ch-- = 0;
	if (ch < name2)
	{
		print ("invalid name [%s]\n", name);
		return;
	}

	banlog_new = &banlogs[banlog_next];

	parent = NULL;
	ppnew = &banlog_head;
	while (*ppnew)
	{
		if ((*ppnew)->addr == addr)
		{
			print ("ip address [%d.%d.%d.xxx] already exists\n", a, b, c);
			return;
		}
		parent = *ppnew;
		ppnew = &(*ppnew)->children[addr > (*ppnew)->addr];
	}
	*ppnew = banlog_new;
	strcpy(banlog_new->name, name2);
	banlog_new->addr = addr;
	banlog_new->parent = parent;
	banlog_new->children[0] = NULL;
	banlog_new->children[1] = NULL;

	print ("ip address [%d.%d.%d.xxx] added by %s\n", a, b, c, name2);
	if (print == SV_ClientPrintf)
		Con_Printf ("ip address [%d.%d.%d.xxx] added by %s [%s]\n", a, b, c, name2, host_client->netconnection->address);

	if (++banlog_next == banlog_size)
	{
		banlog_next = 0;
		banlog_full = 1;
	}
	if (banlog_full)
		BANLog_Delete(&banlogs[banlog_next]);
}

/*
====================
BANLog_Remove
====================
*/
void BANLog_Remove (int addr)
{
	void (*print) (char *fmt, ...);
	banlog_t **ppnew;
	int a,b,c;

	if (!banlog_size)
		return;

	if (cmd_source == src_client)
		print = SV_ClientPrintf;
	else
		print = Con_Printf;

	a = addr >> 16;
	b = (addr >> 8) & 0xff;
	c = addr & 0xff;
	if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255)
	{
		print ("ip address [%d.%d.%d.xxx] out of range\n", a, b, c);
		return;
	}

	ppnew = &banlog_head;
	while (*ppnew)
	{
		if ((*ppnew)->addr == addr)
		{
			strcpy ((*ppnew)->name, ""); // hack
			BANLog_Delete((*ppnew));
			print ("ip address [%d.%d.%d.xxx] removed\n", a, b, c);
			return;
		}
		ppnew = &(*ppnew)->children[addr > (*ppnew)->addr];
	}
	print ("ip address [%d.%d.%d.xxx] not found\n", a, b, c);
}

/*
====================
BANLog_Delete
====================
*/
void BANLog_Delete (banlog_t *node)
{
	banlog_t *newlog;

	newlog = BANLog_Merge(node->children[0], node->children[1]);
	if (newlog)
		newlog->parent = node->parent;
	if (node->parent)
		node->parent->children[node->addr > node->parent->addr] = newlog;
	else
		banlog_head = newlog;
}

/*
====================
BANLog_Merge
====================
*/
banlog_t *BANLog_Merge (banlog_t *left, banlog_t *right)
{
	if (!left)
		return right;
	if (!right)
		return left;

	if (rand() & 1)
	{
		left->children[1] = BANLog_Merge(left->children[1], right);
		left->children[1]->parent = left;
		return left;
	}
	right->children[0] = BANLog_Merge(left, right->children[0]);
	right->children[0]->parent = right;
	return right;
}

/*
====================
BANLog_Identify
====================
*/
int BANLog_Identify (int addr)
{
	banlog_t *node;

	node = banlog_head;
	while (node)
	{
		if (node->addr == addr)
		{
			Con_Printf ("Banning IP address [%d.%d.%d.xxx]\n", addr >> 16, (addr >> 8) & 0xff, addr & 0xff);
			return 1;
		}
		node = node->children[addr > node->addr];
	}
	return 0;
}

/*
====================
BANLog_DumpTree
====================
*/
void BANLog_DumpTree (banlog_t *root, FILE *f)
{
	char address[16];
	char name[16];
	unsigned char *ch;

	if (!root)
		return;
	BANLog_DumpTree(root->children[0], f);

	dpsnprintf(address, sizeof(address), "%d.%d.%d.xxx", root->addr >> 16, (root->addr >> 8) & 0xff, root->addr & 0xff);
	strcpy(name, root->name);
	for (ch = name ; *ch ; ch++)
	{
		*ch = dequake[*ch];
		if (*ch == 10 || *ch == 13)
			*ch = ' ';
	}

	if (!f)
	{
		if (cmd_source == src_client)
			SV_ClientPrintf ("%-16s  %s\n", address, name);
		else
			Con_Printf ("%-16s  %s\n", address, name);
	}
	else
		fprintf(f, "%-16s  %s\n", address, name);

	BANLog_DumpTree(root->children[1], f);
}

/*
====================
BANLog_Dump
====================
*/
void BANLog_Dump (void)
{
	FILE *f;

	if (cmd_source == src_client)
		return;

	if (!banlog_size)
	{
		Con_Printf("BAN logging not available\nRemove -nobanlog command line option\n");
		return;
	}

	f = fopen(va("%s/banlog.txt",com_gamedir), "w");
	if (!f)
	{
		Con_Printf ("Couldn't write banlog.txt.\n");
		return;
	}

	BANLog_DumpTree(banlog_head, f);
	fclose(f);
	Con_Printf("Wrote banlog.txt\n");

	BANLog_WriteLog ();
}
