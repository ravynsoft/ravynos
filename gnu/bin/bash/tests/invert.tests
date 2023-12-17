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
# tests of return value inversion
# placeholder for future expansion

# user subshells (...) did this wrong in bash versions before 2.04

! ( echo hello | grep h >/dev/null 2>&1 ); echo $?
! echo hello | grep h >/dev/null 2>&1 ; echo $?

! true ; echo $?
! false; echo $?

! (false) ; echo $?
! (true); echo $?

! true | false ; echo $?
! false | true ; echo $?

! (true | false) ; echo $?
! (false | true) ; echo $?
