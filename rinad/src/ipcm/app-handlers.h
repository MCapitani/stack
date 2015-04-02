/*
 * APP transaction states
 *
 *    Marc Sune <marc.sune (at) bisdn.de>
 *    Eduard Grasa          <eduard.grasa@i2cat.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __APP_H__
#define __APP_H__

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <utility>

#include <librina/common.h>
#include <librina/ipc-manager.h>
#include <librina/patterns.h>

#include "ipcp-handlers.h"

namespace rinad {

/**
* Standard APP related transaction state
*/
class APPregTransState: public IPCPTransState{

public:
	APPregTransState(Promise* promise, int ipcp_id,
			const rina::ApplicationRegistrationRequestEvent& _req)
				: IPCPTransState(promise, ipcp_id),
				  req(_req){};
	virtual ~APPregTransState(){};

	//Request information
	rina::ApplicationRegistrationRequestEvent req;
};

/**
* APP unregister related transaction state
*/
class APPUnregTransState: public IPCPTransState{

public:
	APPUnregTransState(Promise* promise, int ipcp_id,
			const rina::ApplicationUnregistrationRequestEvent& _req)
				: IPCPTransState(promise, ipcp_id),
						req(_req){};
	virtual ~APPUnregTransState(){};

	//Request event
	rina::ApplicationUnregistrationRequestEvent req;
};
}//rinad namespace

#endif  /* __APP_H__ */
