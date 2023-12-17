#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
aflag=
bflag=

while getopts ab: name -a -b 1 -a -a -a -b 5 -b 3 -a one two three four five
do
	case $name in
	a)	aflag=1 ;;
	b)	bflag=1
		bval=$OPTARG;;
	?)	echo Usage: $0 [-a] [-b value] args
		exit 2;;
	esac

done

if [ ! -z "$aflag" ] ; then echo -a specified ; fi
if [ ! -z "$bflag" ] ; then echo -b $bval specified ; fi

set -- -a -b 1 -a -a -a -b 5 -b 3 -a one two three four five
if [ "$OPTIND" -gt 1 ]
then
	shift $(( $OPTIND - 1 ))
fi

echo remaining args: "$*"

exit 0
