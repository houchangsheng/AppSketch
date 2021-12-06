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

static inline bool match_smb_payload(uint32_t payload, uint32_t len) {

        if (len == 0)
                return true;

        if (match_payload_length(payload, len))
                return true;

        /* Some stupid systems send the NetBIOS header separately, which
         * makes this a lot harder to detect :( 
         *
         * Instead, look for common payload sizes. */

        if (MATCH(payload, 0x00, 0x00, 0x00, 0x85))
                return true;

        /* Also, sometimes we just forget the NetBIOS header, or the 
         * connection fails before it is retransmitted */
        if (MATCH(payload, 0xff, 'S', 'M', 'B'))
                return true;

        return false;

}


static inline bool match_smb(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	/* SMB is often prepended with a NetBIOS session service header.
         * It's easiest for us to treat it as a four byte length field (it
         * is actually a bit more complicated than that, but all other fields
         * tend to be zero anyway)
         *
         * More details at http://lists.samba.org/archive/samba-technical/2003-January/026283.html
         */

	/* Only match on port 445 to avoid clashing with other 4 byte length
	 * fields */
        if (data->server_port != 445 && data->client_port != 445)
                return false;

        if (!match_smb_payload(data->payload[0], data->payload_len[0]))
                return false;

        if (!match_smb_payload(data->payload[1], data->payload_len[1]))
                return false;
        return true;


}

static lpi_module_t lpi_smb = {
	LPI_PROTO_SMB,
	LPI_CATEGORY_FILES,
	"SMB",
	3,
	match_smb
};

void register_smb(LPIModuleMap *mod_map) {
	register_protocol(&lpi_smb, mod_map);
}

