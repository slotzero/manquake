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
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>

#include <asm/io.h>

#include "vga.h"
#include "vgakeyboard.h"
#include "vgamouse.h"

#include "quakedef.h"
#include "d_local.h"

#define stringify(m) { #m, m }

unsigned short       d_8to16table[256];
static byte		*vid_surfcache;
static int		VID_highhunkmark;

int num_modes;
vga_modeinfo *modes;
int current_mode;

int num_shades=32;

struct
{
	char *name;
	int num;
} mice[] =
{
	stringify(MOUSE_MICROSOFT),
	stringify(MOUSE_MOUSESYSTEMS),
	stringify(MOUSE_MMSERIES),
	stringify(MOUSE_LOGITECH),
	stringify(MOUSE_BUSMOUSE),
	stringify(MOUSE_PS2),
};

static byte vid_current_palette[768];

int num_mice = sizeof (mice) / sizeof(mice[0]);

int	d_con_indirect = 0;

int		svgalib_inited=0;
int		UseMouse = 1;
int		UseDisplay = 1;
int		UseKeyboard = 1;

int		mouserate = MOUSE_DEFAULTSAMPLERATE;

cvar_t		vid_mode = {"vid_mode","5",false};
cvar_t		vid_redrawfull = {"vid_redrawfull","0",false};
cvar_t		vid_waitforrefresh = {"vid_waitforrefresh","0",true};

char	*framebuffer_ptr;

cvar_t  mouse_button_commands[3] =
{
    {"mouse1","+attack"},
    {"mouse2","+strafe"},
    {"mouse3","+forward"},
};

int     mouse_buttons;
int     mouse_buttonstate;
int     mouse_oldbuttonstate;
float   mouse_x, mouse_y;
float	old_mouse_x, old_mouse_y;
int		mx, my;

cvar_t	m_filter = {"m_filter","0"};

static byte     backingbuf[48*24];

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes, VGA_planar;
byte	*VGA_pagebase;


/*
=================
D_BeginDirectRect
=================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	int i, j, k, plane, reps, repshift, offset, vidpage, off;

	if (!svgalib_inited || !vid.direct || !vga_oktowrite()) return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	} else {
		reps = 1;
		repshift = 0;
	}

	vidpage = 0;
	vga_setpage(0);

	if (VGA_planar)
	{
		for (plane=0 ; plane<4 ; plane++)
		{
		// select the correct plane for reading and writing
			outb(0x02, 0x3C4);
			outb(1 << plane, 0x3C5);
			outb(4, 0x3CE);
			outb(plane, 0x3CF);

			for (i=0 ; i<(height << repshift) ; i += reps)
			{
				for (k=0 ; k<reps ; k++)
				{
					for (j=0 ; j<(width >> 2) ; j++)
					{
						backingbuf[(i + k) * 24 + (j << 2) + plane] =
								vid.direct[(y + i + k) * VGA_rowbytes +
								(x >> 2) + j];
						vid.direct[(y + i + k) * VGA_rowbytes + (x>>2) + j] =
								pbitmap[(i >> repshift) * 24 +
								(j << 2) + plane];
					}
				}
			}
		}
	} else {
		for (i=0 ; i<(height << repshift) ; i += reps)
		{
			for (j=0 ; j<reps ; j++)
			{
				offset = x + ((y << repshift) + i + j) * vid.rowbytes;
				off = offset % 0x10000;
				if ((offset / 0x10000) != vidpage) {
					vidpage=offset / 0x10000;
					vga_setpage(vidpage);
				}
				memcpy (&backingbuf[(i + j) * 24],
						vid.direct + off, width);
				memcpy (vid.direct + off,
						&pbitmap[(i >> repshift)*width], width);
			}
		}
	}
}


/*
=================
D_EndDirectRect
=================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
	int i, j, k, plane, reps, repshift, offset, vidpage, off;

	if (!svgalib_inited || !vid.direct || !vga_oktowrite()) return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	} else {
		reps = 1;
		repshift = 0;
	}

	vidpage = 0;
	vga_setpage(0);

	if (VGA_planar)
	{
		for (plane=0 ; plane<4 ; plane++)
		{
		// select the correct plane for writing
			outb(2, 0x3C4);
			outb(1 << plane, 0x3C5);
			outb(4, 0x3CE);
			outb(plane, 0x3CF);

			for (i=0 ; i<(height << repshift) ; i += reps)
			{
				for (k=0 ; k<reps ; k++)
				{
					for (j=0 ; j<(width >> 2) ; j++)
					{
						vid.direct[(y + i + k) * VGA_rowbytes + (x>>2) + j] =
								backingbuf[(i + k) * 24 + (j << 2) + plane];
					}
				}
			}
		}
	} else {
		for (i=0 ; i<(height << repshift) ; i += reps)
		{
			for (j=0 ; j<reps ; j++)
			{
				offset = x + ((y << repshift) + i + j) * vid.rowbytes;
				off = offset % 0x10000;
				if ((offset / 0x10000) != vidpage) {
					vidpage=offset / 0x10000;
					vga_setpage(vidpage);
				}
				memcpy (vid.direct + off,
						&backingbuf[(i +j)*24],
						width);
			}
		}
	}
}


/*
=================
VID_Debug_f
=================
*/
void VID_Debug_f (void)
{
	Con_Printf("mode: %d\n",current_mode);
	Con_Printf("height x width: %d x %d\n",vid.height,vid.width);
	Con_Printf("bpp: %d\n",modes[current_mode].bytesperpixel*8);
	Con_Printf("vid.aspect: %f\n",vid.aspect);
}


/*
=================
VID_ShiftPalette
=================
*/
void VID_ShiftPalette(unsigned char *p)
{
	VID_SetPalette(p);
}


/*
=================
VID_SetPalette
=================
*/
void VID_SetPalette(byte *palette)
{

	static int tmppal[256*3];
	int *tp;
	int i;

	if (!svgalib_inited)
		return;

	memcpy(vid_current_palette, palette, sizeof(vid_current_palette));

	if (vga_getcolors() == 256)
	{

		tp = tmppal;
		for (i=256*3 ; i ; i--)
			*(tp++) = *(palette++) >> 2;

		if (UseDisplay && vga_oktowrite())
			vga_setpalvec(0, 256, tmppal);

	}
}


/*
=================
VID_SetMode
=================
*/
int VID_SetMode (int modenum, unsigned char *palette)
{
	int bsize, zsize, tsize;

	if ((modenum >= num_modes) || (modenum < 0) || !modes[modenum].width)
	{
		Cvar_SetValue ("vid_mode", (float)current_mode);

		Con_Printf("No such video mode: %d\n",modenum);

		return 0;
	}

	Cvar_SetValue ("vid_mode", (float)modenum);

	current_mode=modenum;

	vid.width = modes[current_mode].width;
	vid.height = modes[current_mode].height;

	VGA_width = modes[current_mode].width;
	VGA_height = modes[current_mode].height;
	VGA_planar = modes[current_mode].bytesperpixel == 0;
	VGA_rowbytes = modes[current_mode].linewidth;
	vid.rowbytes = modes[current_mode].linewidth;
	if (VGA_planar) {
		VGA_bufferrowbytes = modes[current_mode].linewidth * 4;
		vid.rowbytes = modes[current_mode].linewidth*4;
	}

	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.colormap = (pixel_t *) host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;
	vid.numpages = 1;

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;

	// alloc zbuffer and surface cache


	bsize = vid.rowbytes * vid.height;


	VID_highhunkmark = Hunk_HighMark ();


// get goin'

	vga_setmode(current_mode);
	VID_SetPalette(palette);

	VGA_pagebase = vid.direct = framebuffer_ptr = (char *) vga_getgraphmem();
//		if (vga_setlinearaddressing()>0)
//			framebuffer_ptr = (char *) vga_getgraphmem();
	if (!framebuffer_ptr)
		Sys_Error("This mode isn't hapnin'\n");

	vga_setpage(0);

	svgalib_inited=1;

	vid.recalc_refdef = 1;				// force a surface cache flush

	return 0;
}


/*
=================
VID_Update
=================
*/
void VID_Update(vrect_t *rects)
{
	if (!svgalib_inited)
		return;

	if (!vga_oktowrite())
		return; // can't update screen if it's not active

	if (vid_waitforrefresh.value)
		vga_waitretrace();

	else if (vid_redrawfull.value) {
		int total = vid.rowbytes * vid.height;
		int offset;

		for (offset=0;offset<total;offset+=0x10000) {
			vga_setpage(offset/0x10000);
			memcpy(framebuffer_ptr,
					vid.buffer + offset,
					((total-offset>0x10000)?0x10000:(total-offset)));
		}
	} else {
		int ycount;
		int offset;
		int vidpage=0;

		vga_setpage(0);

		while (rects)
		{
			ycount = rects->height;
			offset = rects->y * vid.rowbytes + rects->x;
			while (ycount--)
			{
				register int i = offset % 0x10000;

				if ((offset / 0x10000) != vidpage) {
					vidpage=offset / 0x10000;
					vga_setpage(vidpage);
				}
				if (rects->width + i > 0x10000) {
					memcpy(framebuffer_ptr + i,
							vid.buffer + offset,
							0x10000 - i);
					vga_setpage(++vidpage);
					memcpy(framebuffer_ptr,
							vid.buffer + offset + 0x10000 - i,
							rects->width - 0x10000 + i);
				} else
					memcpy(framebuffer_ptr + i,
							vid.buffer + offset,
							rects->width);
				offset += vid.rowbytes;
			}

			rects = rects->pnext;
		}
	}

	if (vid_mode.value != current_mode)
		VID_SetMode ((int)vid_mode.value, vid_current_palette);
}


/*
=================
Sys_SendKeyEvents
=================
*/
void Sys_SendKeyEvents(void)
{
	if (!svgalib_inited)
		return;

	if (UseKeyboard)
		while (keyboard_update());
}


/*
=================
IN_Shutdown
=================
*/
void IN_Shutdown(void)
{
	if (UseMouse)
		mouse_close();
}
