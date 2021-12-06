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

static inline bool match_bjnp(lpi_data_t *data, lpi_module_t *mod UNUSED) {

	/* Could strengthen this by requiring 16 byte packets too if needed */

	if (match_str_either(data, "BJNP"))
		return true;

	/* Apparently, there are a few other combinations that we can see */
	if (match_str_either(data, "BNJB"))
		return true;
	if (match_str_either(data, "BJNB"))
		return true;
	if (match_str_either(data, "PJNB"))
		return true;
	if (match_str_either(data, "PNJB"))
		return true;
	

	return false;
}

static lpi_module_t lpi_bjnp = {
	LPI_PROTO_UDP_BJNP,
	LPI_CATEGORY_PRINTING,
	"Canon_BJNP",
	3,
	match_bjnp
};

void register_bjnp(LPIModuleMap *mod_map) {
	register_protocol(&lpi_bjnp, mod_map);
}

