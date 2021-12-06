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

static inline bool match_tv_payload(uint32_t payload, uint32_t len) {

	if (len == 0)
		return true;
	if (len != 88)
		return false;
	if (!MATCH(payload, ANY, ANY, ANY, 0x17))
		return false;
	return true;

}

static inline bool match_teamviewer_classic_udp(lpi_data_t *data) {

	if (!match_tv_payload(data->payload[0], data->payload_len[0]))
		return false;
	if (!match_tv_payload(data->payload[1], data->payload_len[1]))
		return false;

	return true;
}

static inline bool match_teamviewer_96(uint32_t payload, uint32_t len) {
        if (len == 0)
                return true;
        if (len == 96 && MATCH(payload, 0x00, 0x00, 0x00, 0x00))
                return true;
        return false;
}

static inline bool match_tv_1024(uint32_t payload, uint32_t len) {
        if (len == 1024 && MATCH(payload, 0x00, 0x00, 0x00, 0x00))
                return true;
        return false;
}

static inline bool match_tv_48(uint32_t payload, uint32_t len) {
        if (len == 48 && MATCH(payload, 0x00, 0x00, 0x00, 0x00))
                return true;
        return false;
}


static inline bool match_teamviewer_udp(lpi_data_t *data, lpi_module_t *module UNUSED) {

        if (match_teamviewer_classic_udp(data))
                return true;

        if (match_teamviewer_96(data->payload[0], data->payload_len[0])) {
                if (match_teamviewer_96(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (data->server_port == 5938 || data->client_port == 5938) {
                if (match_tv_1024(data->payload[0], data->payload_len[0])) {
                        if (match_tv_48(data->payload[1], data->payload_len[1]))
                                return true;
                }
                if (match_tv_1024(data->payload[1], data->payload_len[1])) {
                        if (match_tv_48(data->payload[0], data->payload_len[0]))
                                return true;
                }
        }
        return false;
}

static lpi_module_t lpi_teamviewer = {
	LPI_PROTO_UDP_TEAMVIEWER,
	LPI_CATEGORY_REMOTE,
	"TeamViewer_UDP",
	115,
	match_teamviewer_udp
};

void register_teamviewer_udp(LPIModuleMap *mod_map) {
	register_protocol(&lpi_teamviewer, mod_map);
}

