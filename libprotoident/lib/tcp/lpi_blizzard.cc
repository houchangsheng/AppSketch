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

static inline bool match_bnet_auth_req(uint32_t payload, uint32_t len) {

        if (len >= 160 && len <= 170 && MATCH(payload, 0x00, 0x0a, 0x08, 0xfe))
                return true;
        return false;

}

static inline bool match_bnet_auth_resp(uint32_t payload, uint32_t len) {

        if (len == 184 || len == 199) {
                if (MATCH(payload, 0x00, 0x09, 0x08, 0x00))
                        return true;
        }
        return false;
}

static inline bool match_blizzard(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	if (match_str_both(data, "\x10\xdf\x22\x00", "\x10\x00\x00\x00"))
                return true;

        if (MATCH(data->payload[0], 0x00, ANY, 0xed, 0x01) &&
                MATCH(data->payload[1], 0x00, 0x06, 0xec, 0x01))
                return true;
        if (MATCH(data->payload[1], 0x00, ANY, 0xed, 0x01) &&
                MATCH(data->payload[0], 0x00, 0x06, 0xec, 0x01))
                return true;


        /* More up to date battle.net authentication protocol */
        if (match_bnet_auth_req(data->payload[0], data->payload_len[0])) {
                if (match_bnet_auth_resp(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_bnet_auth_req(data->payload[1], data->payload_len[1])) {
                if (match_bnet_auth_resp(data->payload[0], data->payload_len[0]))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_blizzard = {
	LPI_PROTO_BLIZZARD,
	LPI_CATEGORY_GAMING,
	"Blizzard",
	2,
	match_blizzard
};

void register_blizzard(LPIModuleMap *mod_map) {
	register_protocol(&lpi_blizzard, mod_map);
}

