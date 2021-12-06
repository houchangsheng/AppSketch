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

static inline bool match_teamviewer_payload(uint32_t payload, uint32_t len) {

	if (len == 0)
		return true;
	if (!MATCH(payload, 0x17, 0x24, ANY, ANY))
		return false;

	if (len < 256 && (ntohl(payload) & 0xff) != len - 5)
		return false;
	
	return true;
	

}

static inline bool match_teamviewer_alt(uint32_t payload, uint32_t len) {
        if (!MATCH(payload, 0x11, 0x30, 0x36, 0x00))
                return false;
        return true;
}

static inline bool match_teamviewer_37(uint32_t payload, uint32_t len) {
        if (MATCH(payload, 0x11, 0x30, 0x37, 0x00 && len == 32))
                return true;
        return false;
}

static inline bool match_teamviewer_38(uint32_t payload) {
        if (MATCH(payload, 0x11, 0x30, 0x38, 0x00))
                return true;
        return false;
}

static inline bool match_teamviewer(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	if (match_teamviewer_payload(data->payload[0], data->payload_len[0])) {
                if (match_teamviewer_payload(data->payload[1], data->payload_len[1]))
                        return true;
                if (match_teamviewer_alt(data->payload[1], data->payload_len[1]))
                        return true;

        }

	if (match_teamviewer_payload(data->payload[1], data->payload_len[1])) {
                if (match_teamviewer_payload(data->payload[0], data->payload_len[0]))
                        return true;
                if (match_teamviewer_alt(data->payload[0], data->payload_len[0]))
                        return true;

        }

        if (match_teamviewer_37(data->payload[0], data->payload_len[0])) {
                if (match_teamviewer_38(data->payload[1]))
                        return true;
        }

        if (match_teamviewer_37(data->payload[1], data->payload_len[1])) {
                if (match_teamviewer_38(data->payload[0]))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_teamviewer = {
	LPI_PROTO_TEAMVIEWER,
	LPI_CATEGORY_REMOTE,
	"Teamviewer",
	40,
	match_teamviewer
};

void register_teamviewer(LPIModuleMap *mod_map) {
	register_protocol(&lpi_teamviewer, mod_map);
}

