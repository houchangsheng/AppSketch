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

static inline bool match_notes_rpc(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	/* Notes RPC is a proprietary protocol and I haven't been able to
         * find anything to confirm or disprove any of this. 
         *
         * As a result, this rule is pretty iffy as it is based on a bunch
         * of flows observed going to 1 server using port 1352. There is
         * no documented basis for this (unlike most other rules)
         */

        if (data->server_port != 1352 && data->client_port != 1352)
                return false;

        if (!match_str_either(data, "\x78\x00\x00\x00"))
                return false;

        if (MATCH(data->payload[0], ANY, ANY, 0x00, 0x00) &&
                        MATCH(data->payload[1], ANY, ANY, 0x00, 0x00))
                return true;
	

	return false;
}

static lpi_module_t lpi_notes_rpc = {
	LPI_PROTO_NOTES_RPC,
	LPI_CATEGORY_REMOTE,
	"Lotus_Notes_RPC",
	200,	/* Don't really trust this rule that much :/ */
	match_notes_rpc
};

void register_notes_rpc(LPIModuleMap *mod_map) {
	register_protocol(&lpi_notes_rpc, mod_map);
}

