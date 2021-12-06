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

static inline bool match_zeroaccess_in(uint32_t payload, uint32_t len) {

	if (len != 20)
		return false;
	if (!MATCH(payload, 0xe5, 0xaa, 0xc0, 0x31))
		return false;
	return true;

}

static inline bool match_zeroaccess_out(uint32_t payload, uint32_t len) {

	if (len == 0)
		return true;
	if (!MATCH(payload, 0xe5, 0xaa, 0xc0, 0x31))
		return false;
	return true;

}

static inline bool match_trojan_zeroaccess(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	/* More info on this trojan can be found at 
	 * http://www.antivirus365.org/PCAntivirus/43465.html */

	if (match_zeroaccess_in(data->payload[0], data->payload_len[0])) {
		if (match_zeroaccess_out(data->payload[1], data->payload_len[1]))
			return true;
	}
	if (match_zeroaccess_in(data->payload[1], data->payload_len[1])) {
		if (match_zeroaccess_out(data->payload[0], data->payload_len[0]))
			return true;
	}

	return false;
}

static lpi_module_t lpi_trojan_zeroaccess = {
	LPI_PROTO_TROJAN_ZEROACCESS,
	LPI_CATEGORY_MALWARE,
	"TrojanZeroAccess",
	10,
	match_trojan_zeroaccess
};

void register_trojan_zeroaccess(LPIModuleMap *mod_map) {
	register_protocol(&lpi_trojan_zeroaccess, mod_map);
}

