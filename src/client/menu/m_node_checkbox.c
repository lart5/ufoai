
#include "m_nodes.h"
#include "m_parse.h"

void MN_DrawCheckBoxNode (const menu_t* menu, const menuNode_t* node, const char *checkBoxImage)
{
	const char *image;
	const char *ref;

	/* image set? */
	if (checkBoxImage && *checkBoxImage)
		image = checkBoxImage;
	else
		image = "menu/checkbox";

	ref = MN_GetReferenceString(menu, node->data[MN_DATA_MODEL_SKIN_OR_CVAR]);
	if (!ref)
		return;

	switch (*ref) {
	case '0':
		R_DrawNormPic(node->pos[0], node->pos[1], node->size[0], node->size[1],
			node->texh[0], node->texh[1], node->texl[0], node->texl[1], node->align, node->blend, va("%s_unchecked", image));
		break;
	case '1':
		R_DrawNormPic(node->pos[0], node->pos[1], node->size[0], node->size[1],
			node->texh[0], node->texh[1], node->texl[0], node->texl[1], node->align, node->blend, va("%s_checked", image));
		break;
	default:
		Com_Printf("Error - invalid value for MN_CHECKBOX node - only 0/1 allowed\n");
	}
}
