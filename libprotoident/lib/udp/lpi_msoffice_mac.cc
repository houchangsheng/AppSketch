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


/* Protocol used by MS Office 2008 (Mac OS X version only) for license checking 
 * on a local network.
 */

static inline bool match_office_2008(lpi_data_t *data) {

	if (!match_str_either(data, "MSOP"))
		return false;
	
	if (data->payload_len[0] == 72 && data->payload_len[1] == 0)
		return true;
	if (data->payload_len[1] == 72 && data->payload_len[0] == 0)
		return true;

	return false;
}
 
static inline bool match_msoffice_mac(lpi_data_t *data, 
		lpi_module_t *mod UNUSED) {

	if (data->server_port == 2223 || data->client_port == 2223) {
		return match_office_2008(data);
	}

	return false;
}

static lpi_module_t lpi_msoffice_mac = {
	LPI_PROTO_UDP_MSOFFICE_MAC,
	LPI_CATEGORY_BROADCAST,
	"MSOffice_Mac",
	10,
	match_msoffice_mac
};

void register_msoffice_mac(LPIModuleMap *mod_map) {
	register_protocol(&lpi_msoffice_mac, mod_map);
}

