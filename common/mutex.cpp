/*
 * Copyright(C) 2008 Laurent Defert, Romain Bignon
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
 *
 * $Id$
 */

#include <map>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "mutex.h"

void Mutex::Init(MutexType _type)
{
	mutex = (pthread_mutex_t*) calloc(sizeof(pthread_mutex_t), 1);
	type = _type;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);

	switch(type)
	{
		case NORMAL_MUTEX:
			//#ifdef DEBUG
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
			//#else
			//		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
			//#endif
			break;
		case RECURSIVE_MUTEX:
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			break;
		default:
			assert(false);
	}

	int r = pthread_mutex_init(mutex, &attr );
	assert(r == 0);

	pthread_mutexattr_destroy(&attr);
}

Mutex::Mutex(MutexType type)
{
	Init(type);
}

Mutex::Mutex(const Mutex& m)
{
	Init(m.type);
}

Mutex::~Mutex()
{
	int r = pthread_mutex_destroy(mutex);
	assert(r == 0);
	free(mutex);
}

void Mutex::Lock()
{
	int res = pthread_mutex_lock(mutex);
	assert(res == 0);
}

void Mutex::Unlock()
{
	int res = pthread_mutex_unlock(mutex);
	assert(res == 0);
}
