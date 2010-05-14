/**
 * @file cl_battlescape.h
 */

/*
Copyright (C) 2002-2010 UFO: Alien Invasion.

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

#ifndef CL_BATTLESCAPE_H_
#define CL_BATTLESCAPE_H_

/** @todo There should be better places for these two macros */
/* if you increase this, you also have to change the aircraft buy/sell menu scripts */
#define MAX_ACTIVETEAM	8
#define MAX_TEAMLIST	8

typedef struct {
	char name[MAX_VAR];
	char cinfo[MAX_VAR];
} clientinfo_t;

typedef struct chr_list_s {
	character_t* chr[MAX_ACTIVETEAM];
	int num;	/* Number of entries */
} chrList_t;

/**
 * @brief This is the structure that should be used for data that is needed for tactical missions only.
 * @note the client_state_t structure is wiped completely at every server map change
 * @sa client_static_t
 */
typedef struct client_state_s {
	int time;					/**< this is the time value that the client
								 * is rendering at.  always <= cls.realtime */
	camera_t cam;

	le_t *teamList[MAX_TEAMLIST];
	int numTeamList;
	int numAliensSpotted;

	qboolean eventsBlocked;		/**< if the client interrupts the event execution, this is true */

	/** server state information */
	int pnum;			/**< player num you have on the server */
	int actTeam;		/**< the currently active team (in this round) */

	char configstrings[MAX_CONFIGSTRINGS][MAX_TOKEN_CHARS];

	/** locally derived information from server state */
	model_t *model_draw[MAX_MODELS];
	struct cBspModel_s *model_clip[MAX_MODELS];

	qboolean radarInited;		/**< every radar image (for every level [1-8]) is loaded */

	clientinfo_t clientinfo[MAX_CLIENTS]; /**< client info of all connected clients */

	int mapMaxLevel;
	int mapMaxLevelBase;

	int numMapParticles;

	int numLMs;
	localModel_t LMs[MAX_LOCALMODELS];

	int numLEs;
	le_t LEs[MAX_EDICTS];

	const char *leInlineModelList[MAX_EDICTS + 1];

	qboolean spawned;		/**< soldiers already spawned? This is only true if we are already on battlescape but
							 * our team is not yet spawned */

	chrList_t chrList;	/**< the list of characters that are used as team in the currently running tactical mission */
} client_state_t;

extern client_state_t cl;

qboolean CL_OnBattlescape(void);
qboolean CL_BattlescapeRunning(void);
int CL_GetHitProbability(const le_t* actor);
qboolean CL_OutsideMap(const vec3_t impact, const float delta);

#endif /* CL_BATTLESCAPE_H_ */
