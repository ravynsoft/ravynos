# Copyright © 2019 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from unittest import mock

import pytest

from . import post_version


@mock.patch('bin.post_version.subprocess.run', mock.Mock())
class TestUpdateCalendar:

    @pytest.fixture(autouse=True)
    def mock_sideffects(self) -> None:
        """Mock out side effects."""
        with mock.patch('bin.post_version.subprocess.run', mock.Mock()), \
                mock.patch('bin.post_version.pathlib', mock.MagicMock()):
            yield

    def test_basic(self):
        data = [
            ['20.3', '2021-01-13', '20.3.3', 'Dylan Baker', None],
            [None,   '2021-01-27', '20.3.4', 'Dylan Baker', None],
        ]

        m = mock.Mock()
        with mock.patch('bin.post_version.csv.reader', mock.Mock(return_value=data.copy())), \
                mock.patch('bin.post_version.csv.writer', mock.Mock(return_value=m)):
            post_version.update_calendar('20.3.3')

            m.writerows.assert_called_with([data[1]])

    def test_two_releases(self):
        data = [
            ['20.3', '2021-01-13', '20.3.3', 'Dylan Baker', None],
            [None,   '2021-01-27', '20.3.4', 'Dylan Baker', None],
            ['21.0', '2021-01-13', '21.0.0', 'Dylan Baker', None],
            [None,   '2021-01-13', '21.0.1', 'Dylan Baker', None],
        ]

        m = mock.Mock()
        with mock.patch('bin.post_version.csv.reader', mock.Mock(return_value=data.copy())), \
                mock.patch('bin.post_version.csv.writer', mock.Mock(return_value=m)):
            post_version.update_calendar('20.3.3')

            d = data.copy()
            del d[0]
            d[0][0] = '20.3'
            m.writerows.assert_called_with(d)
