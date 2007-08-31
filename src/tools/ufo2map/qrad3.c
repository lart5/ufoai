/**
 * @file qrad3.c
 * @brief
 * @note every surface must be divided into at least two patches each axis
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

#include "qrad.h"

patch_t		*face_patches[MAX_MAP_FACES];
entity_t	*face_entity[MAX_MAP_FACES];
patch_t		patches[MAX_PATCHES];
unsigned	num_patches;

vec3_t		radiosity[MAX_PATCHES];		/* light leaving a patch */
vec3_t		illumination[MAX_PATCHES];	/* light arriving at a patch */

vec3_t		face_offset[MAX_MAP_FACES];		/* for rotating bmodels */
dplane_t	backplanes[MAX_MAP_PLANES];

char		inbase[32], outbase[32];

int			fakeplanes;					/* created planes for origin offset */

void BuildLightmaps(void);
int TestLine(const vec3_t start, const vec3_t stop);

qboolean	noradiosity = qfalse;

/*
===================================================================
MISC
===================================================================
*/


/**
 * @brief
 */
static void MakeBackplanes (void)
{
	int i;

	for (i = 0; i < numplanes; i++) {
		backplanes[i].dist = -dplanes[i].dist;
		VectorSubtract(vec3_origin, dplanes[i].normal, backplanes[i].normal);
	}
}

#if 0
/**
 * @brief
 */
static void CalcVertexNormals (unsigned int vnum)
{
	qboolean	found;
	int		i, k;
	dface_t	*face;
	int		*se;
	int		v;
	vec3_t	normal;
	vec3_t	vert;

	/* clear normal and successively add plane normal vectors */
	vnum++;
	VectorCopy(dvertexes[vnum].point, vert);
	VectorClear(normal);
	found = qfalse;

	for (i = 0, face = dfaces; i < numfaces; i++, face++)
		if (texinfo[face->texinfo].flags & 0x8000)
			for (k = 0, se = &dsurfedges[face->firstedge]; k < face->numedges; k++, se++) {
				if (*se < 0)
					v = dedges[-*se].v[1];
				else
					v = dedges[*se].v[0];

				if (v == vnum || VectorCompare(dvertexes[v].point, vert)) {
					/* found a plane containing that vertex */
					VectorAdd(normal, dplanes[face->planenum].normal, normal);
					found = qtrue;
					break;
				}
			}

/*	normal[0] = (float)(rand()%256);
	normal[1] = (float)(rand()%256);
	normal[2] = (float)(rand()%256);
	VectorCopy(normal, lastNormal);*/

	if (found && !VectorCompare(normal, vec3_origin)) {
		VectorCopy(normal, vnormals[vnum]);
		VectorNormalize(vnormals[vnum]);
	} else
		VectorClear(vnormals[vnum]);

	Sys_FPrintf(SYS_VRB, "(%1.4f %1.4f %1.4f)\n", (vnormals[vnum])[0], (vnormals[vnum])[1], (vnormals[vnum])[2]);
}
#endif

/*
===================================================================
TRANSFER SCALES
===================================================================
*/

/**
 * @brief
 */
static int PointInLeafnum (vec3_t point)
{
	int nodenum;
	vec_t dist;
	dnode_t *node;
	dplane_t *plane;

	nodenum = 0;
	while (nodenum >= 0) {
		node = &dnodes[nodenum];
		plane = &dplanes[node->planenum];
		dist = DotProduct (point, plane->normal) - plane->dist;
		if (dist > 0)
			nodenum = node->children[0];
		else
			nodenum = node->children[1];
	}

	return -nodenum - 1;
}


/**
 * @brief
 */
dleaf_t *Rad_PointInLeaf (vec3_t point)
{
	int num;

	num = PointInLeafnum (point);
	return &dleafs[num];
}


static int total_transfer;
/**
 * @brief
 */
static void MakeTransfers (unsigned int i)
{
	unsigned int			j;
	vec3_t		delta;
	vec_t		dist, scale;
	float		trans;
	int			itrans;
	patch_t		*patch, *patch2;
	float		total;
	dplane_t	plane;
	vec3_t		origin;
	float		transfers[MAX_PATCHES], *all_transfers;
	int			s;
	int			itotal;

	patch = patches + i;
	total = 0;

	VectorCopy(patch->origin, origin);
	plane = *patch->plane;

	/* find out which patch2s will collect light */
	/* from patch */

	all_transfers = transfers;
	patch->numtransfers = 0;
	for (j = 0, patch2 = patches; j < num_patches; j++, patch2++) {
		transfers[j] = 0;

		if (j == i)
			continue;

		/* calculate vector */
		VectorSubtract(patch2->origin, origin, delta);
		dist = VectorNormalize(delta);
		if (!dist)
			continue;	/* should never happen */

		/* reletive angles */
		scale = DotProduct(delta, plane.normal);
		scale *= -DotProduct(delta, patch2->plane->normal);
		if (scale <= 0)
			continue;

		trans = scale * patch2->area / (dist * dist);

		if (trans < 0.1)
			continue;

		/* check exact transfer */
		if (TestLine(patch->origin, patch2->origin))
			continue;

#if 0
		if (trans < 0)
			trans = 0;		/* rounding errors... */
#endif

		transfers[j] = trans;
		if (trans > 0) {
			total += trans;
			patch->numtransfers++;
		}
	}

	/* copy the transfers out and normalize */
	/* total should be somewhere near PI if everything went right */
	/* because partial occlusion isn't accounted for, and nearby */
	/* patches have underestimated form factors, it will usually */
	/* be higher than PI */
	if (patch->numtransfers) {
		transfer_t *t;

		if (patch->numtransfers < 0 || patch->numtransfers > MAX_PATCHES)
			Error("Weird numtransfers");
		s = patch->numtransfers * sizeof(transfer_t);
		patch->transfers = malloc(s);
		if (!patch->transfers)
			Error("Memory allocation failure");

		/* normalize all transfers so all of the light */
		/* is transfered to the surroundings */
		t = patch->transfers;
		itotal = 0;
		for (j = 0; j < num_patches; j++) {
			if (transfers[j] <= 0)
				continue;
			itrans = transfers[j] * 0x10000 / total;
			itotal += itrans;
			t->transfer = itrans;
			t->patch = j;
			t++;
		}
	}

	/* don't bother locking around this.  not that important. */
	total_transfer += patch->numtransfers;
}


/**
 * @brief
 */
static void FreeTransfers (void)
{
	unsigned int i;

	for (i = 0; i < num_patches; i++) {
		if (patches[i].transfers)
			free(patches[i].transfers);
		patches[i].transfers = NULL;
	}
}

/**
 * @brief
 */
static void WriteWorld (const char *name)
{
	unsigned int j;
	int i;
	FILE *out;
	patch_t *patch;
	winding_t *w;

	out = fopen(name, "w");
	if (!out)
		Error("Couldn't open %s", name);

	for (j = 0, patch = patches; j < num_patches; j++, patch++) {
		w = patch->winding;
		fprintf(out, "%i\n", w->numpoints);
		for (i = 0; i < w->numpoints; i++) {
			fprintf(out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
				w->p[i][0], w->p[i][1], w->p[i][2],
				patch->totallight[0], patch->totallight[1], patch->totallight[2]);
		}
		fprintf(out, "\n");
	}

	fclose(out);
}

/**
 * @brief
 */
static float CollectLight (void)
{
	unsigned int i, j;
	patch_t	*patch;
	vec_t	total;

	total = 0;

	for (i = 0, patch = patches; i < num_patches; i++, patch++) {
		for (j = 0; j < 3; j++) {
			patch->totallight[j] += illumination[i][j] / patch->area;
			radiosity[i][j] = illumination[i][j] * patch->reflectivity[j];
		}

		total += radiosity[i][0] + radiosity[i][1] + radiosity[i][2];
		VectorClear(illumination[i]);
	}

	return total;
}


/**
 * @brief Send light out to other patches
 */
static void ShootLight (unsigned int patchnum)
{
	int			k, l;
	transfer_t	*trans;
	int			num;
	patch_t		*patch;
	vec3_t		send;

	/* this is the amount of light we are distributing */
	/* prescale it so that multiplying by the 16 bit */
	/* transfer values gives a proper output value */
	for (k = 0; k < 3; k++)
		send[k] = radiosity[patchnum][k] / 0x10000;
	patch = &patches[patchnum];

	trans = patch->transfers;
	num = patch->numtransfers;

	for (k = 0; k < num; k++, trans++) {
		for (l = 0; l < 3; l++)
			illumination[trans->patch][l] += send[l] * trans->transfer;
	}
}

/**
 * @brief
 */
static void BounceLight (void)
{
	unsigned int i, j;
	float	added;
	char	name[64];
	patch_t	*p;
	char buf[12];

	for (i = 0; i < num_patches; i++) {
		p = &patches[i];
		for (j = 0; j < 3; j++) {
/*			p->totallight[j] = p->samplelight[j]; */
			radiosity[i][j] = p->samplelight[j] * p->reflectivity[j] * p->area;
		}
	}

	for (i = 0; i < config.numbounce; i++) {
		snprintf(buf, sizeof(buf), " %i LGHTBNCE", i);
		U2M_ProgressBar(ShootLight, num_patches, qtrue, buf);
		added = CollectLight();

		Sys_FPrintf(SYS_VRB, "bounce:%i added:%f\n", i, added);
		if (config.dumppatches && (i == 0 || i == config.numbounce - 1)) {
			sprintf(name, "bounce%i.txt", i);
			WriteWorld(name);
		}
	}
}



/*============================================================== */

/**
 * @brief
 */
static void CheckPatches (void)
{
	unsigned int i;
	patch_t *patch;

	for (i = 0; i < num_patches; i++) {
		patch = &patches[i];
		if (patch->totallight[0] < 0 || patch->totallight[1] < 0 || patch->totallight[2] < 0)
			Error("negative patch totallight\n");
	}
}

/**
 * @brief
 */
void RadWorld (void)
{
	if (numnodes == 0 || numfaces == 0)
		Error("Empty map");

	MakeBackplanes();

	MakeTnodes(256);

	/* turn each face into a single patch */
	MakePatches();

	/* subdivide patches to a maximum dimension */
	SubdividePatches();

	/* calculate vertex normals for smooth lightning */
	/*U2M_ProgressBar(CalcVertexNormals, numvertexes - 1, qtrue, "VERTEXNRM");*/

	/* create directlights out of patches and lights */
	CreateDirectLights();

	/* build initial facelights */
	U2M_ProgressBar(BuildFacelights, numfaces, qtrue, "FACELIGHTS");

	if (config.numbounce > 0) {
		/* build transfer lists */
		U2M_ProgressBar(MakeTransfers, num_patches, qtrue, "TRANSFERS");
		Sys_FPrintf(SYS_VRB, "transfer lists: %5.1f megs\n",
			(float)total_transfer * sizeof(transfer_t) / (1024 * 1024));

		/* spread light around */
		BounceLight();

		FreeTransfers();

		CheckPatches();
	}

	/* blend bounced light into direct light and save */
	PairEdges();
	LinkPlaneFaces();

	/* initialize light data */
	dlightdata[0] = config.lightquant;
	lightdatasize = 1;

	U2M_ProgressBar(FinalLightFace, numfaces, qtrue, "FINALLIGHT");
/*	CloseTNodes();*/
}
