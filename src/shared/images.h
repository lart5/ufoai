/**
 * @file images.h
 * @brief Image loading and saving functions
 */

/*
 Copyright (C) 2002-2009 Quake2World.
 Copyright (C) 2002-2009 UFO: Alien Invasion.

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

#ifndef IMAGES_H_
#define IMAGES_H_

#include "../common/common.h"
#include "ufotypes.h"
#include <SDL_image.h>

qboolean Img_LoadImage(const char *name, SDL_Surface **surf);
void R_WriteCompressedTGA(qFILE *f, const byte *buffer, int width, int height);
void R_WritePNG(qFILE *f, byte *buffer, int width, int height);
void R_WriteJPG(qFILE *f, byte *buffer, int width, int height, int quality);

#endif /* IMAGES_H_ */
