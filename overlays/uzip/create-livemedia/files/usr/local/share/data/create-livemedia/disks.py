import re
import subprocess

def call(command, **kw):
    """
    Similar to ``subprocess.Popen`` with the following changes:
    * returns stdout, stderr, and exit code (vs. just the exit code)
    * logs the full contents of stderr and stdout (separately) to the file log
    By default, no terminal output is given, not even the command that is going
    to run.
    Useful when system calls are needed to act on output, and that same output
    shouldn't get displayed on the terminal.
    Optionally, the command can be displayed on the terminal and the log file,
    and log file output can be turned off. This is useful to prevent sensitive
    output going to stderr/stdout and being captured on a log file.
    :param terminal_verbose: Log command output to terminal, defaults to False, and
                             it is forcefully set to True if a return code is non-zero
    :param logfile_verbose: Log stderr/stdout output to log file. Defaults to True
    :param verbose_on_failure: On a non-zero exit status, it will forcefully set logging ON for
                               the terminal. Defaults to True
    """
    executable = command.pop(0)
    command.insert(0, executable)
    terminal_verbose = kw.pop('terminal_verbose', False)
    logfile_verbose = kw.pop('logfile_verbose', True)
    verbose_on_failure = kw.pop('verbose_on_failure', True)
    show_command = kw.pop('show_command', False)
    command_msg = "Running command: %s" % ' '.join(command)
    stdin = kw.pop('stdin', None)
    print(command_msg)

    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdin=subprocess.PIPE,
        close_fds=True,
        **kw
    )

    if stdin:
        stdout_stream, stderr_stream = process.communicate(as_bytes(stdin))
    else:
        stdout_stream = process.stdout.read()
        stderr_stream = process.stderr.read()
    returncode = process.wait()
    if not isinstance(stdout_stream, str):
        stdout_stream = stdout_stream.decode('utf-8')
    if not isinstance(stderr_stream, str):
        stderr_stream = stderr_stream.decode('utf-8')
    stdout = stdout_stream.splitlines()
    stderr = stderr_stream.splitlines()

    if returncode != 0:
        if verbose_on_failure:
            terminal_verbose = True
    return stdout, stderr, returncode


def geom_disk_parser(block):
    pairs = block.split(';')
    parsed = {}
    for pair in pairs:
        if 'Providers' in pair:
            continue
        try:
            column, value = pair.split(':')
        except ValueError:
            continue
        # fixup
        column = re.sub("\s+", "", column)
        column= re.sub("^[0-9]+\.", "", column)
        value = value.strip()
        value = re.sub('\([0-9A-Z]+\)', '', value)
        parsed[column.lower()] = value
    return parsed

def get_disk(diskname):
    """
    Captures all available info from geom
    along with interesting metadata like sectors, size, vendor,
    solid/rotational, etc...

    Returns a dictionary, with all the geom fields as keys.
    """

    command = ['/sbin/geom', 'disk', 'list', re.sub('/dev/', '', diskname)]
    out, err, rc = call(command)
    geom_block = "" 
    for line in out:
        line.strip()
        geom_block += ";" + line
    disk = geom_disk_parser(geom_block)
    return disk

def get_disks():
    command = ['/sbin/geom', 'disk', 'status', '-s']
    out, err, rc = call(command)
    disks = {}
    for path in out:
        dsk, rest1, rest2 = path.split()
        disk = get_disk(dsk)
        disks['/dev/'+dsk] = disk
    return disks

class Disks(object):

    def __init__(self, path=None):
        self.disks = {}


class Disk(object):

    def __init__(self, path):
        self.abspath = path
        self.path = path
        self.reject_reasons = []
        self.available = True


if __name__ == "__main__":
    ds = get_disks()
    for d in ds:

        di = get_disk(d)
        print(di)
        if int(di.get("mediasize")) >= 6*1024*1024:
          print(di.get("descr"))