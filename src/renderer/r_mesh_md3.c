/**
 * @file r_mesh_md3.c
 * @brief MD3 Model drawing code
 */

/*
All original material Copyright (C) 2002-2007 UFO: Alien Invasion team.

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

#include "r_local.h"
#include "r_error.h"

static void R_DrawAliasMD3FrameLerp (entity_t* e, mAliasModel_t *md3, mAliasMesh_t mesh)
{
	int i, j;
	mAliasFrame_t	*frame, *oldframe;
	vec3_t	move, delta, vectors[3];
	mAliasVertex_t	*v, *ov;
	vec3_t	tempVertexArray[4096];
	vec3_t	tempNormalsArray[4096];
	vec3_t	color1,color2,color3;
	float	alpha;
	float	frontlerp;
	vec3_t	tempangle;

	color1[0] = color1[1] = color1[2] = 0;
	color2[0] = color2[1] = color2[2] = 0;
	color3[0] = color3[1] = color3[2] = 0;

	frontlerp = 1.0 - e->as.backlerp;

	if (e->flags & RF_TRANSLUCENT)
		alpha = e->alpha;
	else
		alpha = 1.0;

	frame = md3->frames + e->as.frame;
	oldframe = md3->frames + e->as.oldframe;

	VectorSubtract(e->oldorigin, e->origin, delta);
	VectorCopy(e->angles, tempangle);
	tempangle[YAW] = tempangle[YAW] - 90;
	AngleVectors(tempangle, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct(delta, vectors[0]);	/* forward */
	move[1] = -DotProduct(delta, vectors[1]);	/* left */
	move[2] = DotProduct(delta, vectors[2]);	/* up */

	VectorAdd(move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] = e->as.backlerp * move[i] + frontlerp * frame->translate[i];
	}

	v = mesh.vertexes + e->as.frame * mesh.num_verts;
	ov = mesh.vertexes + e->as.oldframe * mesh.num_verts;
	for (i = 0; i < mesh.num_verts; i++, v++, ov++) {
		VectorSet(tempNormalsArray[i],
				v->normal[0] + (ov->normal[0] - v->normal[0]) * e->as.backlerp,
				v->normal[1] + (ov->normal[1] - v->normal[1]) * e->as.backlerp,
				v->normal[2] + (ov->normal[2] - v->normal[2]) * e->as.backlerp);
		VectorSet(tempVertexArray[i],
				move[0] + ov->point[0] * e->as.backlerp + v->point[0] * frontlerp,
				move[1] + ov->point[1] * e->as.backlerp + v->point[1] * frontlerp,
				move[2] + ov->point[2] * e->as.backlerp + v->point[2] * frontlerp);
	}
	qglBegin(GL_TRIANGLES);

	for (j = 0; j < mesh.num_tris; j++) {
		qglTexCoord2f(mesh.stcoords[mesh.indexes[3 * j + 0]].st[0], mesh.stcoords[mesh.indexes[3 * j + 0]].st[1]);
		qglVertex3fv(tempVertexArray[mesh.indexes[3 * j + 0]]);

		qglTexCoord2f(mesh.stcoords[mesh.indexes[3 * j + 1]].st[0], mesh.stcoords[mesh.indexes[3 * j + 1]].st[1]);
		qglVertex3fv(tempVertexArray[mesh.indexes[3 * j + 1]]);

		qglTexCoord2f(mesh.stcoords[mesh.indexes[3 * j + 2]].st[0], mesh.stcoords[mesh.indexes[3 * j + 2]].st[1]);
		qglVertex3fv(tempVertexArray[mesh.indexes[3 * j + 2]]);
	}
	qglEnd();
	R_CheckError();
}

/**
 * @sa R_DrawAliasMD2Model
 * @todo Implement the md2 renderer effect (e.g. RF_HIGHLIGHT)
 */
void R_DrawAliasMD3Model (entity_t *e)
{
	mAliasModel_t *md3;
	image_t *skin;
	int i;

	assert(e->model->type == mod_alias_md3);

	md3 = (mAliasModel_t *)e->model->alias.extraData;

	for (i = 0; i < md3->num_meshes; i++) {
		c_alias_polys += md3->meshes[i].num_tris;
	}

	qglPushMatrix();

	qglMultMatrixf(trafo[e - r_entities].matrix);

	if ((e->as.frame >= md3->num_frames) || (e->as.frame < 0)) {
		Com_Printf("R_DrawAliasMD3Model %s: no such frame %d\n", e->model->name, e->as.frame);
		e->as.frame = 0;
		e->as.oldframe = 0;
	}

	if ((e->as.oldframe >= md3->num_frames) || (e->as.oldframe < 0)) {
		Com_Printf("R_DrawAliasMD3Model %s: no such oldframe %d\n",
			e->model->name, e->as.oldframe);
		e->as.frame = 0;
		e->as.oldframe = 0;
	}

	for (i = 0; i < md3->num_meshes; i++) {
		skin = e->model->alias.skins_img[e->skinnum];
		R_Bind(skin->texnum);
		/* locate the proper data */
		c_alias_polys += md3->meshes[i].num_tris;
		R_DrawAliasMD3FrameLerp(e, md3, md3->meshes[i]);
	}

	R_DrawEntityEffects(e);

	qglPopMatrix();

	R_Color(NULL);
}
