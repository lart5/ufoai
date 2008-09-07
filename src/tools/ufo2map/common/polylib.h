/**
 * @file polylib.h
 */

/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#ifndef _POLYLIB
#define _POLYLIB

#define	MAX_POINTS_ON_WINDING	64

/* you can define on_epsilon in the makefile as tighter */
#ifndef	ON_EPSILON
#define	ON_EPSILON	0.1
#endif

winding_t *AllocWinding(int points);
vec_t WindingArea(const winding_t *w);
void WindingCenter(const winding_t *w, vec3_t center);
void ClipWindingEpsilon(const winding_t *in, const vec3_t normal, const vec_t dist,
	const vec_t epsilon, winding_t **front, winding_t **back);
winding_t *ChopWinding(winding_t *in, vec3_t normal, vec_t dist);
winding_t *CopyWinding(const winding_t *w);
winding_t *ReverseWinding(const winding_t *w);
winding_t *BaseWindingForPlane(const vec3_t normal, const vec_t dist);
void RemoveColinearPoints(winding_t *w);
void FreeWinding(winding_t *w);
void WindingBounds(winding_t *w, vec3_t mins, vec3_t maxs);

/* frees the original if clipped */
void ChopWindingInPlace(winding_t **w, const vec3_t normal, const vec_t dist, const vec_t epsilon);


#endif /* POLYLIB */
