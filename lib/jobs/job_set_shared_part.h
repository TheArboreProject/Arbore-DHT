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
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 *
 */

#ifndef JOB_SET_SHARED_PART_H
#define JOB_SET_SHARED_PART_H
#include "job.h"
#include "pf_types.h"

class JobSetSharedPart : public Job
{
	std::string filename;
	pf_id sharer;
	off_t offset;
	off_t size;
public:
	JobSetSharedPart(std::string _filename, pf_id _sharer, off_t _offset, off_t _size) :
	Job(0, REPEAT_NONE), filename(_filename), sharer(_sharer), offset(_offset), size(_size) {}

	bool Start();
};
#endif						  /* JOB_SET_SHARED_PART_H */
