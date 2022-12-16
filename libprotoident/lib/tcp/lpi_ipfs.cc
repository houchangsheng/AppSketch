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

/* self-described "P2P hypermedia protocol" -- https://ipfs.io */

static inline bool match_ipfs_mu(uint32_t payload, uint32_t len) {
        if (len == 20 && MATCH(payload, 0x13, 0x2f, 'm', 'u')) {
                return true;
        }
        if (len == 34 && MATCH(payload, 0x13, 0x2f, 'm', 'u')) {
                return true;
        }
        return false;
}

static inline bool match_ipfs_single(uint32_t payload, uint32_t len) {
        if (len == 1 && MATCH(payload, 0x13, 0x00, 0x00, 0x00))
                return true;
        return false;
}

static inline bool match_ipfs_length(uint32_t payload, uint32_t len) {
        uint32_t plen = ntohl(payload);

        /* Starting to see IPFS replies with a four byte length field */
        if (plen + 4 == len) {
                return true;
        }
        return false;
}

static inline bool match_ipfs(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        /* Port 4001 by default, but probably changeable */
        if (match_ipfs_mu(data->payload[0], data->payload_len[0])) {
                if (match_ipfs_single(data->payload[1], data->payload_len[1]))
                        return true;
                if (match_ipfs_mu(data->payload[1], data->payload_len[1]))
                        return true;
                if (match_ipfs_length(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_ipfs_mu(data->payload[1], data->payload_len[1])) {
                if (match_ipfs_single(data->payload[0], data->payload_len[0]))
                        return true;
                if (match_ipfs_length(data->payload[0], data->payload_len[0]))
                        return true;
        }

	return false;
}

static lpi_module_t lpi_ipfs = {
	LPI_PROTO_IPFS,
	LPI_CATEGORY_P2P,
	"IPFS",
	10,
	match_ipfs
};

void register_ipfs(LPIModuleMap *mod_map) {
	register_protocol(&lpi_ipfs, mod_map);
}

