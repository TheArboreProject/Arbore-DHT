/*
 * Copyright(C) 2012 Benoît Saccomano
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "libArbore_network_Network.h"
#include <net/network.h>
#include <net/hosts_list.h>
#include <chimera/chimera.h>

/*
 * Class:     libArbore_network_Network
 * Method:    N_getHost_List
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_network_Network_N_1getHost_1List
  (JNIEnv *, jobject, jlong instance)
	{
		Network* net = (Network*) instance;
		return (long) &hosts_list;
	}

/*
 * Class:     libArbore_network_Network
 * Method:    initCppSide
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_network_Network_initCppSide
  (JNIEnv *, jobject, jlong chimera_instance)
	{
		Chimera* chimera = (Chimera*) chimera_instance;
		return (jlong) new Network(chimera);
	}

/*
 * Class:     libArbore_network_Network
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_network_Network_destroyCppSide
  (JNIEnv *, jobject, jlong instance)
	{
		Network* net = (Network*) instance;
		delete net;
	}
