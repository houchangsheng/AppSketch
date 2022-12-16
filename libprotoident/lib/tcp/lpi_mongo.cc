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

/* Mongo protocol uses the first four bytes as a length field */
static inline bool match_mongo_req(uint32_t payload, uint32_t len) {

        uint32_t mongolen = bswap_le_to_host32(payload);

        /* Most requests are very small */
        if (MATCH(payload, ANY, 0x00, 0x00, 0x00) ||
                        MATCH(payload, ANY, 0x01, 0x00, 0x00)) {
                /* Some mongo libraries manage to split requests across
                 * multiple packets, so we can't do a direct length
                 * match :(
                 */
                if (mongolen >= len)
                        return true;
        }

        return false;

}

static inline bool match_mongo_reply(uint32_t payload, uint32_t len) {

        uint32_t mongolen = bswap_le_to_host32(payload);

        /* If reply is short, mongolen should match the packet length */
        if (mongolen == len)
                return true;

        /* If mongolen is large, packet len should be MTU-sized. Here,
         * we'll assume an MTU of at least 1400 (sorry, people with
         * tons of encapsulation) */
        if (mongolen > len) {
                if (len >= 1400)
                        return true;
        }

        return false;

}

static inline bool match_mongo(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        /* Restrict to the default mongo port for now */
        if (data->server_port != 27017 && data->client_port != 27017)
                return false;

        if (match_mongo_req(data->payload[0], data->payload_len[0])) {
                if (match_mongo_reply(data->payload[1], data->payload_len[1]))
                        return true;
        }
        if (match_mongo_req(data->payload[1], data->payload_len[1])) {
                if (match_mongo_reply(data->payload[0], data->payload_len[0]))
                        return true;
        }
	return false;
}

static lpi_module_t lpi_mongo = {
	LPI_PROTO_MONGO,
	LPI_CATEGORY_DATABASES,
	"MongoDB",
        88,
	match_mongo
};

void register_mongo(LPIModuleMap *mod_map) {
	register_protocol(&lpi_mongo, mod_map);
}

