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

static inline bool match_naver_magic(uint32_t payload) {
        if (MATCH(payload, 0xcf, 0x10, 0x00, 0x1f))
                return true;
        return false;
}

static inline bool match_naverp2p(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        if (match_naver_magic(data->payload[0])) {
                if (match_naver_magic(data->payload[1])) {
                        return true;
                }
        }

	return false;
}

static lpi_module_t lpi_naverp2p = {
	LPI_PROTO_NAVER_P2P,
	LPI_CATEGORY_P2P,
	"NaverP2P",
	29,
	match_naverp2p
};

void register_naverp2p(LPIModuleMap *mod_map) {
	register_protocol(&lpi_naverp2p, mod_map);
}

