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

/* Punkbuster: anti-cheating software for online games. Uses its own
 * protocol for keeping the local PB software up to date. */

static inline bool match_pb_req(uint32_t payload, uint32_t len) {

        /* length 60-65 bytes */
        if (MATCH(payload, 'k', 'e', 'y', 0x20))
                return true;
        return false;

}

static inline bool match_pb_resp(uint32_t payload, uint32_t len) {

        /* length 49-51 bytes */

        if (MATCHSTR(payload, "\xff\xff\xff\xff"))
                return true;
        return false;

}


static inline bool match_punkbuster(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        if (match_pb_req(data->payload[1], data->payload_len[1])) {
                if (match_pb_resp(data->payload[0], data->payload_len[0]))
                        return true;
        }

        if (match_pb_req(data->payload[0], data->payload_len[0])) {
                if (match_pb_resp(data->payload[1], data->payload_len[1]))
                        return true;
        }



	return false;
}

static lpi_module_t lpi_punkbuster = {
	LPI_PROTO_UDP_PUNKBUSTER,
	LPI_CATEGORY_GAMING,
	"Punkbuster",
	9,
	match_punkbuster
};

void register_punkbuster(LPIModuleMap *mod_map) {
	register_protocol(&lpi_punkbuster, mod_map);
}

