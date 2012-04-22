/**
 * @file
 */

/*
Copyright (C) 2002-2011 UFO: Alien Invasion.

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

#ifndef CLIENT_UI_UI_NODE_MAP_H
#define CLIENT_UI_UI_NODE_MAP_H

#include "../ui_nodes.h"

class uiMapNode : public uiLocatedNode {
	void draw(uiNode_t* node) OVERRIDE;
	void mouseUp(uiNode_t* node, int x, int y, int button) OVERRIDE;
	void capturedMouseMove(uiNode_t* node, int x, int y) OVERRIDE;
	void capturedMouseLost(uiNode_t* node) OVERRIDE;
	void loading(uiNode_t* node) OVERRIDE;
	bool scroll(uiNode_t* node, int deltaX, int deltaY) OVERRIDE;
	void leftClick(uiNode_t* node, int x, int y) OVERRIDE;
	bool startDragging(uiNode_t* node, int startX, int startY, int currentX, int currentY, int button) OVERRIDE;
};

#define UI_MAPEXTRADATA_TYPE mapExtraData_t
#define UI_MAPEXTRADATA(node) UI_EXTRADATA(node, UI_MAPEXTRADATA_TYPE)
#define UI_MAPEXTRADATACONST(node) UI_EXTRADATACONST(node, UI_MAPEXTRADATA_TYPE)

typedef struct mapExtraData_s {
	float paddingRight;
} mapExtraData_t;

void UI_RegisterMapNode(struct uiBehaviour_s *behaviour);

#endif
