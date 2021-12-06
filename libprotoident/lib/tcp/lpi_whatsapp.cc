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

static inline bool match_wa_first(uint32_t payload, uint32_t len) {

	if (MATCH(payload, 'W', 'A', 0x01, 0x02))
		return true;
	if (MATCH(payload, 'W', 'A', 0x01, 0x05))
		return true;

        if (len == 1 && MATCH(payload, 'W', 0x00, 0x00, 0x00))
                return true;
	return false;

}

static inline bool match_wa_first_20(uint32_t payload, uint32_t len) {
        /* New protocol version? 2.0? */
	if (MATCH(payload, 'W', 'A', 0x02, 0x00))
		return true;
        if (len == 1 && MATCH(payload, 'W', 0x00, 0x00, 0x00))
                return true;
	return false;
}

static inline bool match_ed_first(uint32_t payload, uint32_t len) {
        if (MATCH(payload, 'E', 'D', 0x00, 0x01))
                return true;
        if (len == 1 && MATCH(payload, 'E', 0x00, 0x00, 0x00))
                return true;
        return false;
}


static inline bool match_wa_second(uint32_t payload, uint32_t len) {
	if (len == 0)
		return true;
	
	if (MATCH(payload, 0x00, 0x00, 0x05, 0xf8))
		return true;
	return false;
}

static inline bool match_wa_second_20(uint32_t payload, uint32_t len) {
	uint32_t walen = ntohl(payload) >> 8;

        if (len == 0)
		return true;

        if (walen == len - 3) {
                if (MATCH(payload, ANY, ANY, ANY, 0x1a))
                        return true;
        }
	
	return false;
}

static inline bool match_wa_fixed_second(uint32_t payload) {
        if (MATCH(payload, 0x00, 0x00, 0x36, 0x1a)) {
                return true;
        }
        return false;
}

static inline bool match_whatsapp(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	/* WhatsApp uses a modified form of XMPP and typically runs on
	 * either port 443 or 5222
	 */

	if (match_wa_first(data->payload[0], data->payload_len[0])) {
		if (match_wa_second(data->payload[1], data->payload_len[1]))
			return true;
	}
	
	if (match_wa_first(data->payload[1], data->payload_len[1])) {
		if (match_wa_second(data->payload[0], data->payload_len[0]))
			return true;
	}

	if (match_wa_first_20(data->payload[0], data->payload_len[0])) {
		if (match_wa_second_20(data->payload[1], data->payload_len[1]))
			return true;
	}
	
	if (match_wa_first_20(data->payload[1], data->payload_len[1])) {
		if (match_wa_second_20(data->payload[0], data->payload_len[0]))
			return true;
	}

        if (match_ed_first(data->payload[0], data->payload_len[0])) {
		if (match_wa_second_20(data->payload[1], data->payload_len[1]))
			return true;
		if (match_wa_fixed_second(data->payload[1]))
			return true;
	}

        if (match_ed_first(data->payload[1], data->payload_len[1])) {
		if (match_wa_second_20(data->payload[0], data->payload_len[0]))
			return true;
		if (match_wa_fixed_second(data->payload[0]))
			return true;
	}

	return false;

}

static lpi_module_t lpi_whatsapp = {
	LPI_PROTO_WHATSAPP,
	LPI_CATEGORY_CHAT,
	"WhatsApp",
	4, 
	match_whatsapp
};

void register_whatsapp(LPIModuleMap *mod_map) {
	register_protocol(&lpi_whatsapp, mod_map);
}

