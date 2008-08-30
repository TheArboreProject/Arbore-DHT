/*
 * Copyright(C) 2008 Romain Bignon
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
 * This file contains some code from the Chimera's Distributed Hash Table,
 * written by CURRENT Lab, UCSB.
 *
 */

/*
** $Id: key.h,v 1.16 2006/06/07 09:21:28 krishnap Exp $
**
** Matthew Allen
** description:
*/

#ifndef _CHIMERA_KEY_H_
#define _CHIMERA_KEY_H_

#include <limits.h>
#include <stdio.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdint.h>

#include "pf_log.h"

#define KEY_SIZE 160
#define N_SIZE KEY_SIZE/sizeof(uint32_t)

#define BASE_B 4		/* Base representation of key digits */

#define BASE_16_KEYLENGTH 40

class Key
{
	static const size_t nlen = KEY_SIZE / (8 * sizeof(uint32_t));
	uint32_t t[nlen];
	std::string key_str;

	Key operator+(const Key& op2) const;
	Key operator-(const Key& k2) const;

	void set_key_str ();

public:

	/** Static function which initialize the global keys. */
	static void Init();

	/** Create a Key from an uint32_t.
	 *
	 * @param ul this is the last 32 bits of the key
	 */
	Key(uint32_t ul = 0);

	/** The copy constructor.
	 *
	 * @param k2 copy key from the other key
	 */
	Key(const Key& k2);

	/** Copy a key from a string
	 *
	 * @param str take the hexadecimal string to create the key.
	 * @return a new Key object.
	 */
	Key& operator=(const char* str);

	/** Copy a key from a string
	 *
	 * @param str take the hexadecimal string to create the key.
	 * @return a new Key object.
	 */
	Key& operator=(std::string str);

	/** Copy a key from an other key
	 *
	 * @param k2 key used to be copied from.
	 * @return a new Key object.
	 */
	Key& operator=(const Key& k2);

	/** Copy a key from an uin32_t
	 *
	 * @param ul this is the last 32 bits of the key
	 * @param a new Key object.
	 */
	Key& operator=(uint32_t ul);

	/** Comparaison between two keys.
	 *
	 * @param k2 other key which is compared to.
	 * @return true if two keys are equals.
	 */
	bool operator==(const Key& k2) const;

	/** Comparaison with an uin32_t
	 *
	 * @param ul the last 32 bits of the key.
	 * @return true if I equal to this key.
	 */
	bool operator==(uint32_t ul) const;

	/** Comparaison with an other key.
	 *
	 * @param k2 other key
	 * @return true if two keys are *NOT* equals.
	 */
	bool operator!=(const Key& k2) const { return !(*this == k2); }

	/** Comparaison with an other key.
	 *
	 * @param k2 other key which is compared to.
	 * @return true if I'm superior than k2.
	 */
	bool operator>(const Key& k2) const;
	bool operator<(const Key& k2) const;

	/** Return the string hexadecimal representation of key. */
	std::string str() const { return key_str; }

	/** Assign sha1 hash of the string
	 *
	 * @param s hashed string.
	 */
	void key_make_hash (char *s);

	/** Asign sha1 hash of the string
	 *
	 * @param s hashed string
	 * @param size size of the string (if this isn't a 0 terminated string).
	 */
	void key_make_hash (char *s, size_t size);

	/* key_distance:k1,k2
	** calculate the distance between k1 and k2 in the keyspace and assign that to #diff# */
	Key distance(const Key& k2) const;

	//returns the size of the interval starting from this to upperBound
	Key intervalSize(const Key& upperBound) const;

	/* key_between: test, left, right
	** check to see if the value in #test# falls in the range from #left# clockwise
	** around the ring to #right#. */
	bool between (const Key& left, const Key& right) const;

	/** Check if the key is null.
	 *
	 * @return true if the key is equal to 0.
	 */
	bool operator!() const;
	operator bool() const;

	/* key_midpoint: mid, key
	** calculates the midpoint of the namespace from the #key#  */
	Key midpoint () const;

	/* key_index: mykey, key
	** returns the lenght of the longest prefix match between #mykey# and #k# */
	size_t key_index (Key k) const;
};

template<>
inline Log::flux& Log::flux::operator<< <Key> (Key key)
{
	str += key.str();
	return *this;
}

/* global variables!! that are set in key_init function */
extern Key Key_Max;
extern Key Key_Half;

#endif /* _CHIMERA_KEY_H_ */
