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


static inline bool match_rdp_init(uint32_t payload, uint32_t len) {

        if (len == 1232 && MATCHSTR(payload, "\xff\xff\xff\xff"))
                return true;
        return false;

}

static inline bool match_rdp_udp(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        /* Seen both 3389 and 4732 used as ports */

        if (match_rdp_init(data->payload[0], data->payload_len[0])) {
                if (match_rdp_init(data->payload[1], data->payload_len[1]))
                        return true;

                if (data->server_port == 4732 || data->client_port == 4732) {
                        if (data->payload[1] != 0 && data->payload_len[1] == 1232)
                                return true;
                }
                if (data->server_port == 3389 || data->client_port == 3389) {
                        if (data->payload[1] != 0 && data->payload_len[1] == 1232)
                                return true;
                }
        }

        if (match_rdp_init(data->payload[1], data->payload_len[1])) {
                if (data->server_port == 4732 || data->client_port == 4732) {
                        if (data->payload[0] != 0 && data->payload_len[0] == 1232)
                                return true;
                }
                if (data->server_port == 3389 || data->client_port == 3389) {
                        if (data->payload[0] != 0 && data->payload_len[0] == 1232)
                                return true;
                }
        }

	return false;
}

static lpi_module_t lpi_rdp_udp = {
	LPI_PROTO_UDP_RDP,
	LPI_CATEGORY_REMOTE,
	"RDP_UDP",
	200,
	match_rdp_udp
};

void register_rdp_udp(LPIModuleMap *mod_map) {
	register_protocol(&lpi_rdp_udp, mod_map);
}

