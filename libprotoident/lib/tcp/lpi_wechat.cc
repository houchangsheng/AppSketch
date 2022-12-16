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

static inline bool match_wc_pair(uint32_t payloada, uint32_t lena,
                uint32_t payloadb, uint32_t lenb) {

        if (lena == 16 && MATCH(payloada, 0x00, 0x00, 0x00, 0x10)) {
                if (lenb == 16 && MATCH(payloadb, 0x00, 0x00, 0x00, 0x10))
                        return true;
                if (lenb == 18 && MATCH(payloadb, 0x00, 0x00, 0x00, 0x12))
                        return true;
        }

        if (lena == 21 && MATCH(payloada, 0x00, 0x00, 0x00, 0x15)) {
                if (lenb == 25 && MATCH(payloadb, 0x00, 0x00, 0x00, 0x19))
                        return true;
                if (lenb == 21 && MATCH(payloadb, 0x00, 0x00, 0x00, 0x15))
                        return true;
        }

        return false;


}

static inline bool match_wc_ab_request(uint32_t payload, uint32_t len) {
        /* This is 0xab, followed by 4 bytes of length for the first
         * packet.
         */

        if (len <= 255 && MATCH(payload, 0xab, 0x00, 0x00, 0x00))
                return true;

        if (MATCH(payload, 0xab, 0x00, 0x00, 0x01))
                return true;
        return false;

}

static inline bool match_wc_ab_big02(uint32_t payload, uint32_t len) {
        /* again 0xab followed by length, except this time the length is
         * for the entire flow.
         */
        if (len < 255)
                return false;

        /* Flows are unlikely to need a full 4 bytes for length so I'm
         * going to stick 0x00 or 0x01 in the top byte for now */
        if (MATCH(payload, 0xab, 0x00, ANY, ANY)) {
                return true;
        }
        if (MATCH(payload, 0xab, 0x01, ANY, ANY)) {
                return true;
        }
        return false;
        
}

static inline bool match_wc_ab_big01(uint32_t payload, uint32_t len) {

        if (len < 100)
                return false;
        if (len <= 255 && MATCH(payload, 0xab, 0x00, 0x00, 0x00))
                return true;
        if (len > 255 && len < 512 && MATCH(payload, 0xab, 0x00, 0x00, 0x01))
                return true;
        if (len >= 512 && len < 768 && MATCH(payload, 0xab, 0x00, 0x00, 0x02))
                return true;
        if (len >= 768 && len < 1024 && MATCH(payload, 0xab, 0x00, 0x00, 0x03))
                return true;
        return false;
}

static inline bool match_wc_ab_reply(uint32_t payload, uint32_t len) {
        /* All replies appear to be 41 or 53 bytes */

        if (len != 41 && len != 53)
                return false;

        if (MATCH(payload, 0xab, 0x00, 0x00, 0x00))
                return true;
        return false;

}

/* This appears to be some sort of SSL ripoff */
static inline bool match_wc_ssl_111(uint32_t payload, uint32_t len) {
        if (len == 111 && MATCH(payload, 0x16, 0xf1, 0x03, 0x00))
                return true;
        return false;
}

static inline bool match_wc_ssl_166(uint32_t payload, uint32_t len) {
        if (len == 166 && MATCH(payload, 0x16, 0xf1, 0x03, 0x00))
                return true;
        return false;
}


static inline bool match_wechat(lpi_data_t *data, lpi_module_t *mod UNUSED) {
	bool valid_port = false;

	/* WeChat begins with a very simple 4 byte length field.
	 * This is not unique to WeChat though, so we need to be careful.
	 */

	/* Only observed on port 80, 443, 14000, 10001 or 8080. Because the payload
	 * signature is not entirely unique to WeChat, let's restrict matches
	 * to flows using those ports unless it shows up on other ports.
	 */
	if (data->server_port == 80 || data->client_port == 80)
		valid_port = true;
	if (data->server_port == 8080 || data->client_port == 8080)
		valid_port = true;
	if (data->server_port == 443 || data->client_port == 443)
		valid_port = true;
	if (data->server_port == 14000 || data->client_port == 14000)
		valid_port = true;
	if (data->server_port == 10001 || data->client_port == 10001)
		valid_port = true;

	if (!valid_port)
		return false;

	if (match_wc_pair(data->payload[0], data->payload_len[0],
                        data->payload[1], data->payload_len[1])) {
		return true;
	}
	
	if (match_wc_pair(data->payload[1], data->payload_len[1],
                        data->payload[0], data->payload_len[0])) {
		return true;
	}
	
        if (match_wc_ab_request(data->payload[0], data->payload_len[0])) {
                if (match_wc_ab_reply(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_wc_ab_request(data->payload[1], data->payload_len[1])) {
                if (match_wc_ab_reply(data->payload[0], data->payload_len[0]))
                        return true;
        }

        if (match_wc_ab_big01(data->payload[0], data->payload_len[0])) {
                if (match_wc_ab_big02(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_wc_ab_big01(data->payload[1], data->payload_len[1])) {
                if (match_wc_ab_big02(data->payload[0], data->payload_len[0]))
                        return true;
        }

        if (match_wc_ssl_111(data->payload[0], data->payload_len[0])) {
                if (match_wc_ssl_166(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_wc_ssl_111(data->payload[1], data->payload_len[1])) {
                if (match_wc_ssl_166(data->payload[0], data->payload_len[0]))
                        return true;
        }

	return false;

}

static lpi_module_t lpi_wechat = {
	LPI_PROTO_WECHAT,
	LPI_CATEGORY_CHAT,
	"WeChat",
	10, 
	match_wechat
};

void register_wechat(LPIModuleMap *mod_map) {
	register_protocol(&lpi_wechat, mod_map);
}

