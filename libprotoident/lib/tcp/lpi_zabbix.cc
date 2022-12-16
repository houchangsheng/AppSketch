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

static inline bool match_zabbix(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	if (match_str_either(data, "ZBXD"))
		return true;

        /* Everything below this line requires one of the ports to be the
         * default zabbix port */
        if (data->server_port != 10050 && data->client_port != 10050)
                return false;

        /* Zabbix Windows performance counters 
         * TODO capture some genuine responses and match on those too */
        if (MATCH(data->payload[0], 'p', 'e', 'r', 'f'))
                return true;
        if (MATCH(data->payload[1], 'p', 'e', 'r', 'f'))
                return true;

        if (MATCH(data->payload[0], 's', 'y', 's', 't'))
                return true;
        if (MATCH(data->payload[1], 's', 'y', 's', 't'))
                return true;

	return false;
}

static lpi_module_t lpi_zabbix = {
	LPI_PROTO_ZABBIX,
	LPI_CATEGORY_MONITORING,
	"Zabbix",
	5,
	match_zabbix
};

void register_zabbix(LPIModuleMap *mod_map) {
	register_protocol(&lpi_zabbix, mod_map);
}

