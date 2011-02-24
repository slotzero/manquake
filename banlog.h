/*  $Id: banlog.h,v 1.3 2011/02/24 07:22:18 slotzero Exp $

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

typedef struct tagBANLog
{
	int addr;
	char name[16];
	struct tagBANLog *parent;
	struct tagBANLog *children[2];
} banlog_t;

extern int banlog_size;

void BANLog_Init (void);
void BANLog_WriteLog (void);
void BANLog_Add (int addr, char *name);
void BANLog_Remove (int addr);
void BANLog_Delete (banlog_t *node);
banlog_t *BANLog_Merge (banlog_t *left, banlog_t *right);
int BANLog_Identify (int addr);
void BANLog_Dump (void);
void BANLog_Import (void);
