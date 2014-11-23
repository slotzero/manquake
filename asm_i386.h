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

#ifndef __ASM_I386__
#define __ASM_I386__

#ifdef ELF
#define C(label) label
#else
#define C(label) _##label
#endif

//
// !!! note that this file must match the corresponding C structures at all
// times !!!
//

// plane_t structure
// !!! if this is changed, it must be changed in model.h too !!!
// !!! if the size of this is changed, the array lookup in SV_HullPointContents
//     must be changed too !!!
#define pl_normal	0
#define pl_dist		12
#define pl_type		16
#define pl_signbits	17
#define pl_pad		18
#define pl_size		20

// hull_t structure
// !!! if this is changed, it must be changed in model.h too !!!
#define	hu_clipnodes		0
#define	hu_planes			4
#define	hu_firstclipnode	8
#define	hu_lastclipnode		12
#define	hu_clip_mins		16
#define	hu_clip_maxs		28

// Slot Zero 3.50-2  Prevent players from using wall hack. (3 lines)
//#define hu_size  			40
#define hu_available		40
#define hu_size  			44

// dnode_t structure
// !!! if this is changed, it must be changed in bspfile.h too !!!
#define	nd_planenum		0
#define	nd_children		4
#define	nd_mins			8
#define	nd_maxs			20
#define	nd_firstface	32
#define	nd_numfaces		36
#define nd_size			40

// MOVED ->

// stvert_t structure
// !!! if this is changed, it must be changed in modelgen.h too !!!
#define stv_onseam	0
#define stv_s		4
#define stv_t		8
#define stv_size	12

// trivertx_t structure
// !!! if this is changed, it must be changed in modelgen.h too !!!
#define tv_v				0
#define tv_lightnormalindex	3
#define tv_size				4

// medge_t structure
// !!! if this is changed, it must be changed in model.h too !!!
#define me_v				0
#define me_cachededgeoffset	4
#define me_size				8

// mvertex_t structure
// !!! if this is changed, it must be changed in model.h too !!!
#define mv_position		0
#define mv_size			12

// mtriangle_t structure
// !!! if this is changed, it must be changed in model.h too !!!
#define mtri_facesfront		0
#define mtri_vertindex		4
#define mtri_size			16	// !!! if this changes, array indexing in !!!
								// !!! d_polysa.s must be changed to match !!!
#define mtri_shift			4

#endif
