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

/* https://play.google.com/store/apps/details?id=jp.colopl.pcat */

static inline bool match_pani_42(uint32_t payload, uint32_t len) {

        if (len == 40 && MATCH(payload, 0x42, 0x00, ANY, 0x00))
                return true;
        return false;
}

static inline bool match_pani_4a(uint32_t payload, uint32_t len) {

        if (len == 1051 && MATCH(payload, 0x4a, 0x80, 0x00, 0x00))
                return true;
        return false;
}

static inline bool match_panipani(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        /* Port 7101, 7102 or 7103 */

        if (match_pani_42(data->payload[0], data->payload_len[0])) {
                if (match_pani_4a(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_pani_42(data->payload[1], data->payload_len[1])) {
                if (match_pani_4a(data->payload[0], data->payload_len[0]))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_panipani = {
	LPI_PROTO_UDP_PANIPANI,
	LPI_CATEGORY_GAMING,
	"Panipani",
	101,
	match_panipani
};

void register_panipani(LPIModuleMap *mod_map) {
	register_protocol(&lpi_panipani, mod_map);
}

