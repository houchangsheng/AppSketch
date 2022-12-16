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

/* Only ever seen this on port 80 to hosts owned by Tetris Online. This
 * may not be exclusive to Tetris Online (i.e. it might be a generic flash
 * game protocol) but I can't be sure until I see it somewhere else.
 */


static inline bool match_tetrisonline(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        if (data->server_port != 80 && data->client_port != 80)
                return false;

        if (MATCH(data->payload[0], 'U', 'S', 'R', ' ')) {
                if (MATCH(data->payload[1], 'U', 'S', 'R', ' '))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_tetrisonline = {
	LPI_PROTO_TETRISONLINE,
	LPI_CATEGORY_GAMING,
	"TetrisOnline",
	5,
	match_tetrisonline
};

void register_tetrisonline(LPIModuleMap *mod_map) {
	register_protocol(&lpi_tetrisonline, mod_map);
}

