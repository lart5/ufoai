/**
 * @file m_node_text.c
 */

/*
Copyright (C) 1997-2008 UFO:AI Team

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

#include "m_main.h"
#include "m_font.h"

/**
 * @brief Scrolls the text in a textbox up/down.
 * @param[in] node The node of the text to be scrolled.
 * @param[in] offset Number of lines to scroll. Positive values scroll down, negative up.
 * @return Returns qtrue if scrolling was possible otherwise qfalse.
 */
qboolean MN_TextScroll (menuNode_t *node, int offset)
{
	int textScroll_new;

	if (!node || !node->height)
		return qfalse;

	if (abs(offset) >= node->height) {
		/* Offset value is bigger than textbox height. */
		return qfalse;
	}

	if (node->textLines <= node->height) {
		/* Number of lines are less than the height of the textbox. */
		node->textScroll = 0;
		return qfalse;
	}

	textScroll_new = node->textScroll + offset;

	if (textScroll_new <= 0) {
		/* Goto top line, no matter how big the offset was. */
		node->textScroll = 0;
		return qtrue;

	} else if (textScroll_new >= (node->textLines + 1 - node->height)) {
		/* Goto last possible line, no matter how big the offset was. */
		node->textScroll = node->textLines - node->height;
		return qtrue;

	} else {
		node->textScroll = textScroll_new;
		return qtrue;
	}
}

/**
 * @brief Scriptfunction that gets the wanted text node and scrolls the text.
 */
static void MN_TextScroll_f (void)
{
	int offset = 0;
	menuNode_t *node;

	if (Cmd_Argc() < 3) {
		Com_Printf("Usage: %s <nodename> <+/-offset>\n", Cmd_Argv(0));
		return;
	}

	node = MN_GetNodeFromCurrentMenu(Cmd_Argv(1));

	if (!node) {
		Com_DPrintf(DEBUG_CLIENT, "MN_TextScroll_f: Node '%s' not found.\n", Cmd_Argv(1));
		return;
	}

	if (!Q_strncmp(Cmd_Argv(2), "reset", 5)) {
		node->textScroll = 0;
		return;
	}

	offset = atoi(Cmd_Argv(2));

	if (offset == 0)
		return;

	MN_TextScroll(node, offset);
}

/**
 * @brief Scroll to the bottom
 */
void MN_TextScrollBottom (const char* nodeName)
{
	menuNode_t *node = MN_GetNodeFromCurrentMenu(nodeName);
	if (!node) {
		Com_DPrintf(DEBUG_CLIENT, "Node '%s' could not be found\n", nodeName);
		return;
	}

	if (node->textLines > node->height) {
		Com_DPrintf(DEBUG_CLIENT, "\nMN_TextScrollBottom: Scrolling to line %i\n", node->textLines - node->height + 1);
		node->textScroll = node->textLines - node->height + 1;
	}
}

/**
 * @note need a cleanup/merge/rearchitecture between MN_DrawTextNode2 and MN_DrawTextNode
 */
void MN_DrawTextNode2(menuNode_t *node) {
	message_t *message;
	int i;
	menu_t *menu = node->menu;
	const char *font;
	vec2_t nodepos;
	MN_GetNodeAbsPos(node, nodepos);
	if (mn.menuText[node->num]) {
		font = MN_GetFont(menu, node);
		MN_DrawTextNode(mn.menuText[node->num], NULL, font, node, nodepos[0], nodepos[1], node->size[0], node->size[1]);
	} else if (mn.menuTextLinkedList[node->num]) {
		font = MN_GetFont(menu, node);
		MN_DrawTextNode(NULL, mn.menuTextLinkedList[node->num], font, node, nodepos[0], nodepos[1], node->size[0], node->size[1]);
	} else if (node->num == TEXT_MESSAGESYSTEM) {
		linkedList_t *messagelist = NULL;
		char text[TIMESTAMP_TEXT + MAX_MESSAGE_TEXT];
		font = MN_GetFont(menu, node);

		message = mn.messageStack;
		while (message) {
			/* get formatted date text */
			/** @todo this is not utf-8 safe - but the messages are already translated */
			Com_sprintf(text, sizeof(text), "%s%s", message->timestamp, message->text);
			for (i = 0; i < (sizeof(text) - 1); i++) {
				if (text[i] == '\n') {
					text[i] = '\0';
					break;
				}
			}
			/* Make a list */
			LIST_Add(&messagelist, (byte*) text, sizeof(text));
			message = message->next;
		}
		if (messagelist) {
			MN_DrawTextNode(NULL, messagelist, font, node, nodepos[0], nodepos[1], node->size[0], node->size[1]);
			LIST_Delete(&messagelist);
		}
	}
}

/**
 * @brief Handles line breaks and drawing for MN_TEXT menu nodes
 * @sa MN_DrawMenus
 * @param[in] text Text to draw
 * @param[in] font Font string to use
 * @param[in] node The current menu node
 * @param[in] x The fixed x position every new line starts
 * @param[in] y The fixed y position the text node starts
 * @todo The node pointer can be NULL
 */
void MN_DrawTextNode (const char *text, const linkedList_t* list, const char* font, menuNode_t* node, int x, int y, int width, int height)
{
	const vec4_t scrollbarBackground = {0.03, 0.41, 0.05, 0.5};
	const vec4_t scrollbarColor = {0.03, 0.41, 0.05, 1.0};
	char textCopy[MAX_MENUTEXTLEN];
	int lineHeight = 0;
	char newFont[MAX_VAR];
	const char* oldFont = NULL;
	vec4_t colorHover;
	vec4_t colorSelectedHover;
	char *cur, *tab, *end;
	int x1, y1; /* variable x and y position */
	vec2_t nodepos;

	if (text) {
		Q_strncpyz(textCopy, text, sizeof(textCopy));
	} else if (list) {
		Q_strncpyz(textCopy, (const char*)list->data, sizeof(textCopy));
	} else
		Sys_Error("MN_DrawTextNode: Called without text or linkedList pointer");
	cur = textCopy;

	MN_GetNodeAbsPos(node, nodepos);
	/* Hover darkening effect for normal text lines. */
	VectorScale(node->color, 0.8, colorHover);
	colorHover[3] = node->color[3];

	/* Hover darkening effect for selected text lines. */
	VectorScale(node->selectedColor, 0.8, colorSelectedHover);
	colorSelectedHover[3] = node->selectedColor[3];

	/* scrollbar space */
	if (node->scrollbar) {
		width -= MN_SCROLLBAR_WIDTH + MN_SCROLLBAR_PADDING;
		if (node->scrollbarLeft)
			x += MN_SCROLLBAR_WIDTH + MN_SCROLLBAR_PADDING;
	}

	y1 = y;
	/*Com_Printf("\n\n\nnode->textLines: %i \n", node->textLines);*/
	node->textLines = 0; /* these are lines only in one-line texts! */
	/* but it's easy to fix, just change FontDrawString
	 * so that it returns a pair, #lines and height
	 * and add that to variable line; the only other file
	 * using FontDrawString result is client/cl_sequence.c
	 * and there just ignore #lines */
	do {
		/* new line starts from node x position */
		x1 = x;
		if (oldFont) {
			font = oldFont;
			oldFont = NULL;
		}

		/* text styles and inline images */
		if (cur[0] == '^') {
			switch (toupper(cur[1])) {
			case 'B':
				Com_sprintf(newFont, sizeof(newFont), "%s_bold", font);
				oldFont = font;
				font = newFont;
				cur += 2; /* don't print the format string */
				break;
			}
		} else if (!Q_strncmp(cur, "img:", 4)) {
			const char *token;
			cur += 4;
			token = COM_Parse((const char **)&cur);
			/** @todo once font_t from r_font.h is known everywhere we should scale the height here, too
			 * @todo once image_t is known everywhere we should fix this, too */
			x1 += R_DrawNormPic(x1, y1, 0, 0, 0, 0, 0, 0, node->align, node->blend, token);
		}

		/* get the position of the next newline - otherwise end will be null */
		end = strchr(cur, '\n');
		if (end)
			/* set the \n to \0 to draw only this part (before the \n) with our font renderer */
			/* let end point to the next char after the \n (or \0 now) */
			*end++ = '\0';

		/* highlighting */
		if (node) {
			if (node->textLines == node->textLineSelected && node->textLineSelected >= 0) {
				/* Draw current line in "selected" color (if the linenumber is stored). */
				R_Color(node->selectedColor);
			}

			if (node->mousefx && node->textLines + 1 == node->state) {
				/* Hightlight line if mousefx is true. */
				/* node->state is the line number to highlight */
				/** @todo what about multiline text that should be highlighted completely? */
				if (node->textLines == node->textLineSelected && node->textLineSelected >= 0) {
					R_ColorBlend(colorSelectedHover);
				} else {
					R_ColorBlend(colorHover);
				}
			}
		}


		/* tabulation, we assume all the tabs fit on a single line */
		do {
			tab = strchr(cur, '\t');
			/* if this string does not contain any tabstops break this do while ... */
			if (!tab)
				break;
			/* ... otherwise set the \t to \0 and increase the tab pointer to the next char */
			/* after the tab (needed for the next loop in this (the inner) do while) */
			*tab++ = '\0';
			/* now check whether we should draw this string */
			/*Com_Printf("tab - first part - node->textLines: %i \n", node->textLines);*/
			if (node)
				node->textLines++;
			R_FontDrawString(font, node->align, x1, y1, x, y, width, height, node->texh[0], cur, node->height, node->textScroll, &node->textLines, qfalse);
			if (node)
				node->textLines--;
			/* increase the x value as given via menu definition format string */
			/* or use 1/3 of the node size (width) */
			if (!node || !node->texh[1])
				x1 += (width / 3);
			else
				x1 += node->texh[1];
			/* now set cur to the first char after the \t */
			cur = tab;
		} while (1);

		/*Com_Printf("until newline - node->textLines: %i\n", node->textLines);*/
		/* the conditional expression at the end is a hack to draw "/n/n" as a blank line */
		lineHeight = R_FontDrawString(font, node->align, x1, y1, x, y, width, height, node->texh[0], (*cur ? cur : " "), node->height, node->textScroll, &node->textLines, qtrue);
		if (lineHeight > 0)
			y1 += lineHeight;

		if (node && node->mousefx)
			R_ColorBlend(node->color); /* restore original color */

		/* now set cur to the next char after the \n (see above) */
		cur = end;
		if (!cur && list) {
			list = list->next;
			if (list) {
				Q_strncpyz(textCopy, (const char*)list->data, sizeof(textCopy));
				cur = textCopy;
			}
		}
	} while (cur);

	/* draw scrollbars */
	if (node->scrollbar && node->height && node->textLines > node->height) {
		const int scrollbarX = nodepos[0] + (node->scrollbarLeft ? 0 : node->size[0] - MN_SCROLLBAR_WIDTH);
		const float scrollbarY = node->size[1] * node->height / node->textLines * MN_SCROLLBAR_HEIGHT;

		R_DrawFill(scrollbarX,
			nodepos[1],
			MN_SCROLLBAR_WIDTH,
			node->size[1],
			ALIGN_UL,
			scrollbarBackground);

		R_DrawFill(scrollbarX,
			nodepos[1] + (node->size[1] - scrollbarY) * node->textScroll / (node->textLines - node->height),
			MN_SCROLLBAR_WIDTH,
			scrollbarY,
			ALIGN_UL,
			scrollbarColor);
	}
}

/**
 * @brief Resets the mn.menuText pointers and the mn.menuTextLinkedList lists
 */
void MN_MenuTextReset (int menuTextID)
{
	assert(menuTextID < MAX_MENUTEXTS);
	assert(menuTextID >= 0);

	mn.menuText[menuTextID] = NULL;
	LIST_Delete(&mn.menuTextLinkedList[menuTextID]);
}


/**
 * @brief Resets the mn.menuText pointers from a func node
 * @note You can give this function a parameter to only delete a specific list
 */
static void MN_MenuTextReset_f (void)
{
	int i;

	if (Cmd_Argc() == 2) {
		i = atoi(Cmd_Argv(1));
		if (i >= 0 && i < MAX_MENUTEXTS)
			MN_MenuTextReset(i);
		else
			Com_Printf("%s: invalid mn.menuText ID: %i\n", Cmd_Argv(0), i);
	} else {
		for (i = 0; i < MAX_MENUTEXTS; i++)
			MN_MenuTextReset(i);
	}
}

void MN_NodeTextInit (void)
{
	/* textbox */
	Cmd_AddCommand("mn_textscroll", MN_TextScroll_f, NULL);
	Cmd_AddCommand("mn_textreset", MN_MenuTextReset_f, "Resets the mn.menuText pointers");
}

/**
 * @brief Calls the script command for a text node that is clickable
 * @note The node must have the click parameter in it's menu definition or there
 * must be a console command that has the same name as the node has + _click
 * attached.
 * @sa MN_TextRightClick
 * @todo Check for scrollbars and when one would click them scroll according to
 * mouse movement, maybe implement a new mousespace (MS_* - @sa cl_input.c)
 */
void MN_TextClick (menuNode_t * node, int x, int y)
{
	int mouseOver = 1;
	char cmd[MAX_VAR];
	Com_sprintf(cmd, sizeof(cmd), "%s_click", node->name);
	if (Cmd_Exists(cmd))
		Cbuf_AddText(va("%s %i\n", cmd, mouseOver - 1));
	else if (node->click && node->click->type == EA_CMD) {
		assert(node->click->data);
		Cbuf_AddText(va("%s %i\n", (const char *)node->click->data, mouseOver - 1));
	}
}

/**
 * @brief Calls the script command for a text node that is clickable via right mouse button
 * @note The node must have the rclick parameter
 * @sa MN_TextClick
 */
static void MN_TextRightClick (menuNode_t * node, int x, int y)
{
	int mouseOver = 1;
	char cmd[MAX_VAR];
	Com_sprintf(cmd, sizeof(cmd), "%s_rclick", node->name);
	if (Cmd_Exists(cmd))
		Cbuf_AddText(va("%s %i\n", cmd, mouseOver - 1));
}

static void MN_NodeTextMouseWheel (menuNode_t *node, qboolean down, int x, int y)
{
	menu_t *menu = node->menu;
	if (node->wheelUp && node->wheelDown) {
		MN_ExecuteActions(menu, (down ? node->wheelDown : node->wheelUp));
	} else {
		MN_TextScroll(node, (down ? 1 : -1));
		/* they can also have script commands assigned */
		MN_ExecuteActions(menu, node->wheel);
	}
}

void MN_RegisterNodeText (nodeBehaviour_t *behaviour)
{
	behaviour->name = "text";
	behaviour->draw = MN_DrawTextNode2;
	behaviour->leftClick = MN_TextClick;
	behaviour->rightClick = MN_TextRightClick;
	behaviour->mouseWheel = MN_NodeTextMouseWheel;
}
