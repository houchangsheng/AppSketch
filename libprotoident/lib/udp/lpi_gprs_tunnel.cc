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

static inline bool match_gtp_payload(uint32_t payload, uint32_t len) {
        uint32_t swap = ntohl(payload);
        uint32_t replen = swap & 0x0000ffff;

        if (len == 0)
                return true;

        if (len != replen)
                return false;

        if ((swap & 0xf0ff0000) != 0x30010000) {
                return false;
        }

        return true;
}


static inline bool match_gprs_tunnel(lpi_data_t *data, lpi_module_t *mod UNUSED) {
        if (match_gtp_payload(data->payload[0], data->payload_len[0])) {
                if (match_gtp_payload(data->payload[1], data->payload_len[1]))
                        return true;
        }
         
        if (match_gtp_payload(data->payload[1], data->payload_len[1])) {
                if (match_gtp_payload(data->payload[0], data->payload_len[0]))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_gprs_tunnel = {
	LPI_PROTO_UDP_GPRS_TUNNEL,
	LPI_CATEGORY_TUNNELLING,
	"GPRS_Tunnelling",
	10,
	match_gprs_tunnel
};

void register_gprs_tunnel(LPIModuleMap *mod_map) {
	register_protocol(&lpi_gprs_tunnel, mod_map);
}

