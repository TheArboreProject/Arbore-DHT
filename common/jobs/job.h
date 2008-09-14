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

#ifndef JOB_H
#define JOB_H
#include <list>
#include <time.h>
#include <string>

class Job
{
protected:
	typedef enum
	{
		REPEAT_NONE,
		REPEAT_PERIODIC,
		REPEAT_LESS_AND_LESS
	} repeat_type_t;

private:
	double start_time;
	repeat_type_t repeat_type;
	double repeat_delta;

protected:
	// Start the job, returns true if the job needs to be restarted later
	virtual bool Start() = 0;
public:
	Job(double start_at, repeat_type_t _repeat_type, double _repeat_delta = 0.0) : start_time(start_at), repeat_type(_repeat_type), repeat_delta(_repeat_delta) {}
	virtual ~Job() {}

	// Start the job, returns true if the job needs to be restarted later
	bool DoStart();
	double GetStartTime() const { return start_time; }
};
#endif						  /* JOB_H */
