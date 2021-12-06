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

/* TODO make sure this is not a more generic tencent games protocol? */

static inline bool match_kog_64(uint32_t payload, uint32_t len) {
        if (len == 64 && MATCH(payload, 0x33, 0x66, 0x00, 0x09))
                return true;
        return false;
}

static inline bool match_kog_other(uint32_t payload, uint32_t len) {
        if (len < 100 && MATCH(payload, 0x33, 0x66, 0x00, 0x09))
                return true;
        return false;
}

static inline bool match_kingofglory_tcp(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        if (match_kog_64(data->payload[0], data->payload_len[0])) {
                if (match_kog_other(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_kog_64(data->payload[1], data->payload_len[1])) {
                if (match_kog_other(data->payload[0], data->payload_len[0]))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_kingofglory_tcp = {
	LPI_PROTO_KINGOFGLORY,
	LPI_CATEGORY_GAMING,
	"KingOfGlory_TCP",
	99,
	match_kingofglory_tcp
};

void register_kingofglory_tcp(LPIModuleMap *mod_map) {
	register_protocol(&lpi_kingofglory_tcp, mod_map);
}

