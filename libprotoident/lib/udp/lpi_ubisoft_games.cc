/*
 *
 * Copyright (c) 2011-2016 The University of Waikato, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This file is part of libprotoident.
 *
 * This code has been developed by the University of Waikato WAND
 * research group. For further information please see http://www.wand.net.nz/
 *
 * libprotoident is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * libprotoident is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#include <string.h>

#include "libprotoident.h"
#include "proto_manager.h"
#include "proto_common.h"

/* A variety of Ubisoft games seem to use a similar protocol for
 * multiplayer. Usually seen on UDP port 3074, but not always.
 */

static inline bool match_ubisoft_pat(uint32_t payload, uint32_t len) {

        if (len == 71 && MATCH(payload, 0x00, 0x00, 0x00, 0x00))
                return true;
        return false;

}

static inline bool match_ubisoft_games(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        if (match_ubisoft_pat(data->payload[0], data->payload_len[0])) {
                if (match_ubisoft_pat(data->payload[1], data->payload_len[1]))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_ubisoft_games = {
	LPI_PROTO_UDP_UBISOFT_GAMES,
	LPI_CATEGORY_GAMING,
	"UbisoftGames",
	199,
	match_ubisoft_games
};

void register_ubisoft_games(LPIModuleMap *mod_map) {
	register_protocol(&lpi_ubisoft_games, mod_map);
}

