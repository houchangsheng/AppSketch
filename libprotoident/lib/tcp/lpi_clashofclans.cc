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

static inline bool match_coc_login(uint32_t payload, uint32_t len) {

        /* First two bytes are 10101 (0x2775) or 10100 (0x2774)
         * Next three bytes are a length field, usually 250-330 bytes */

        if (MATCH(payload, 0x27, 0x75, 0x00, 0x01))
                return true;
        if (MATCH(payload, 0x27, 0x75, 0x00, 0x00))
                return true;

        if (MATCH(payload, 0x27, 0x74, 0x00, 0x01))
                return true;
        if (MATCH(payload, 0x27, 0x74, 0x00, 0x00))
                return true;

        return false;

}

static inline bool match_coc_encrypt(uint32_t payload, uint32_t len) {

        /* First two bytes are 20000 (0x4e20) or 20100 (0x4e84)
         * Next three bytes are a length field, always seems to be just
         * under 256 bytes */

        if (MATCH(payload, 0x4e, 0x20, 0x00, 0x00))
                return true;
        if (MATCH(payload, 0x4e, 0x84, 0x00, 0x00))
                return true;

        return false;

}

static inline bool match_clashofclans(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        /* Could limit this to port 9339, but the pattern is probably strong
         * enough by itself */
        /* Actually, port 1863 is also used... */

        if (match_coc_login(data->payload[0], data->payload_len[0])) {
                if (match_coc_encrypt(data->payload[1], data->payload_len[1]))
                        return true;
        }
        
        if (match_coc_login(data->payload[1], data->payload_len[1])) {
                if (match_coc_encrypt(data->payload[0], data->payload_len[0]))
                        return true;
        }


	return false;
}

static lpi_module_t lpi_clashofclans = {
	LPI_PROTO_CLASH_OF_CLANS,
	LPI_CATEGORY_GAMING,
	"ClashOfClans",
	4,
	match_clashofclans
};

void register_clashofclans(LPIModuleMap *mod_map) {
	register_protocol(&lpi_clashofclans, mod_map);
}

