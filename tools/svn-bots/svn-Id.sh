#!/bin/bash
# Copyright(C) 2008 Laurent Defert, Romain Bignon
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# $Id$
#

cd /home/p2pfs-buidlbot/peerfuse/propset/trunk
prop_count=0

(find . -name \*.cpp; find -name \*.h; find -name \*.sh |grep -v 'tools/svn-bots/svn-Id\.sh') | while read file
do
	if grep -q '\$Id.*\$' "$file" && ! (svn propget svn:keywords "$file" | grep -q '^Id$')
	then
		svn propset svn:keywords Id "$file" > /dev/null
		prop_count=$(($prop_count+1))
	fi
done

if [ "$prop_count" != "0" ]
then
	svn ci --username=bot-propset -m "$prop_count properties set."
fi

