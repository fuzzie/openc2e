/*
 *  caosVM_vehicles.cpp
 *  openc2e
 *
 *  Created by Alyssa Milburn on 02/02/2005.
 *  Copyright 2005 Alyssa Milburn. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 */

#include "caosVM.h"
#include <iostream>
#include "openc2e.h"
#include "Vehicle.h"

/**
 CABN (command) left (integer) top (integer) right (integer) bottom (integer)
*/
void caosVM::c_CABN() {
	VM_VERIFY_SIZE(4)
	VM_PARAM_INTEGER(bottom)
	VM_PARAM_INTEGER(right)
	VM_PARAM_INTEGER(top)
	VM_PARAM_INTEGER(left)

	assert(targ); // TODO: check it's a Vehicle
	((Vehicle *)targ)->setCabinRect(left, top, right, bottom);
}

/**
 CABW (command) cap (integer)
*/
void caosVM::c_CABW() {
	VM_VERIFY_SIZE(1)
	VM_PARAM_INTEGER(cap)

	assert(targ); // TODO: check it's a Vehicle
	((Vehicle *)targ)->setCapacity(cap);
}

/**
 SPAS (command) vehicle (agent) passenger (agent)

 make specified vehicle agent pick up specified passenger
*/
void caosVM::c_SPAS() {
	VM_VERIFY_SIZE(2)
	VM_PARAM_AGENT(passenger)
	VM_PARAM_AGENT(vehicle)

	// TODO
}

/**
 GPAS (command) family (integer) genus (integer) species (integer) options (integer)

 pick up all nearby agents matching classifier, as passengers to target vehicle
 options = 0 to pick up based on agent bounding rect, or 1 to pick up based on cabin rect
*/
void caosVM::c_GPAS() {
	VM_VERIFY_SIZE(4)
	VM_PARAM_INTEGER(options)
	// TODO: assert ranges for these
	VM_PARAM_INTEGER(species)
	VM_PARAM_INTEGER(genus)
	VM_PARAM_INTEGER(family)

	// TODO
}

/**
 DPAS (command) family (integer) genus (integer) species (integer)

 drop all agents matching classifier from target vehicle
*/
void caosVM::c_DPAS() {
	VM_VERIFY_SIZE(3)
	// TODO: assert ranges for these
	VM_PARAM_INTEGER(species)
	VM_PARAM_INTEGER(genus)
	VM_PARAM_INTEGER(family)

	// TODO
}
