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

static inline bool match_battlefield(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	/* Server browsing for battlefield 1942 */

        if (match_str_both(data, "ping", "Ping"))
                return true;

        if (MATCHSTR(data->payload[0], "ping")) {
                if (data->payload_len[0] != 5)
                        return false;
                if (data->payload_len[1] == 0)
                        return true;
        }

        if (MATCHSTR(data->payload[1], "ping")) {
                if (data->payload_len[1] != 5)
                        return false;
                if (data->payload_len[0] == 0)
                        return true;
        }
	

	return false;
}

static lpi_module_t lpi_battlefield = {
	LPI_PROTO_UDP_BATTLEFIELD,
	LPI_CATEGORY_GAMING,
	"Battlefield",
	7,
	match_battlefield
};

void register_battlefield(LPIModuleMap *mod_map) {
	register_protocol(&lpi_battlefield, mod_map);
}

