#!/usr/bin/python
#
# Copyright 2018 Simon McVittie
#
# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

import os
import os.path
import sys
import unittest
from collections import OrderedDict

sys.path.insert(0, os.path.join(os.path.dirname(__file__), os.pardir))

import avahi
import dbus

class TestUtilityMethods(unittest.TestCase):
    def test_byte_array_to_string(self):
        self.assertEqual(
            avahi.byte_array_to_string([1, 2, 127, 128]),
            '....')
        self.assertEqual(
            avahi.byte_array_to_string([ord('a'), ord(' '), ord('b')]),
            'a b')

    def test_txt_array_to_string_array(self):
        self.assertEqual(
            avahi.txt_array_to_string_array([[1, 2], [ord('a'), ord('b')]]),
            ['..', 'ab'])

    def test_string_to_byte_array(self):
        self.assertEqual(
            avahi.string_to_byte_array('abc'),
            [dbus.Byte(97), dbus.Byte(98), dbus.Byte(99)])
        self.assertIsInstance(
            avahi.string_to_byte_array('abc')[0],
            dbus.Byte)
        self.assertEqual(
            avahi.string_to_byte_array(b'\x01\xff'),
            [dbus.Byte(0x01), dbus.Byte(0xff)])
        self.assertEqual(
            avahi.string_to_byte_array(u'\u00e1'),
            [dbus.Byte(0xc3), dbus.Byte(0xa1)])

    def test_string_array_to_txt_array(self):
        self.assertEqual(
            avahi.string_array_to_txt_array(['abc', b'\x01', u'\u00e1']),
            [
                [dbus.Byte(97), dbus.Byte(98), dbus.Byte(99)],
                [dbus.Byte(0x01)],
                [dbus.Byte(0xc3), dbus.Byte(0xa1)]])
        self.assertIsInstance(
            avahi.string_array_to_txt_array(['abc'])[0][0],
            dbus.Byte)

    def test_dict_to_txt_array(self):
        self.assertEqual(
            avahi.dict_to_txt_array(
                OrderedDict((('a', 'abc'), ('b', b'\x01'), ('c', u'\u00e1')))),
            [
                [dbus.Byte(97), dbus.Byte(ord('=')), dbus.Byte(97), dbus.Byte(98), dbus.Byte(99)],
                [dbus.Byte(98), dbus.Byte(ord('=')), dbus.Byte(0x01)],
                [dbus.Byte(99), dbus.Byte(ord('=')), dbus.Byte(0xc3), dbus.Byte(0xa1)]])
        self.assertIsInstance(
            avahi.dict_to_txt_array({'a': 'abc'})[0][0],
            dbus.Byte)

if __name__ == '__main__':
    unittest.main()
