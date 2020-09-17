# https://github.com/zaibon/py-dmidecode/
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0

import re
import subprocess


class DMIParse:
    """
    dmidecode information parsing object
    requires dmidecode output as input string
    """

    def __init__(self, str, default="n/a"):
        self.default = default
        self.data = self.dmidecode_parse(str)

    def get(self, type_id):
        if isinstance(type_id, str):
            for type_num, type_str in self.type2str.items():
                if type_str == type_id:
                    type_id = type_num

        result = list()
        for entry in self.data.values():
            if entry["DMIType"] == type_id:
                result.append(entry)
        return result

    def manufacturer(self):
        return self.get("System")[0].get("Manufacturer", self.default)

    def model(self):
        return self.get("System")[0].get("Product Name", self.default)

    def serial_number(self):
        return self.get("System")[0].get("Serial Number", self.default)

    def cpu_type(self):
        cpu_version = self.default
        for cpu in self.get("Processor"):
            if cpu.get("Core Enabled"):
                cpu_version = cpu.get("Version", self.default)
        return cpu_version

    def cpu_num(self):
        cpus = 0
        for cpu in self.get("Processor"):
            if cpu.get("Core Enabled"):
                cpus += 1
        return cpus

    def total_enabled_cores(self):
        cores = 0
        for cpu in self.get("Processor"):
            cores += int(cpu.get("Core Enabled", 0))
        return cores

    def total_ram(self):
        """Returns total memory in GB"""
        return sum([self.size_to_gb(slot["Size"]) for slot in self.get("Memory Device")])

    def firmware(self):
        return self.get("BIOS")[0].get("Firmware Revision", self.default)

    handle_re = re.compile("^Handle\\s+(.+),\\s+DMI\\s+type\\s+(\\d+),\\s+(\\d+)\\s+bytes$")
    in_block_re = re.compile("^\\t\\t(.+)$")
    record_re = re.compile("\\t(.+):\\s+(.+)$")
    record2_re = re.compile("\\t(.+):$")

    type2str = {
        0: "BIOS",
        1: "System",
        2: "Baseboard",
        3: "Chassis",
        4: "Processor",
        5: "Memory Controller",
        6: "Memory Module",
        7: "Cache",
        8: "Port Connector",
        9: "System Slots",
        10: "On Board Devices",
        11: "OEM Strings",
        12: "System Configuration Options",
        13: "BIOS Language",
        14: "Group Associations",
        15: "System Event Log",
        16: "Physical Memory Array",
        17: "Memory Device",
        18: "32-bit Memory Error",
        19: "Memory Array Mapped Address",
        20: "Memory Device Mapped Address",
        21: "Built-in Pointing Device",
        22: "Portable Battery",
        23: "System Reset",
        24: "Hardware Security",
        25: "System Power Controls",
        26: "Voltage Probe",
        27: "Cooling Device",
        28: "Temperature Probe",
        29: "Electrical Current Probe",
        30: "Out-of-band Remote Access",
        31: "Boot Integrity Services",
        32: "System Boot",
        33: "64-bit Memory Error",
        34: "Management Device",
        35: "Management Device Component",
        36: "Management Device Threshold Data",
        37: "Memory Channel",
        38: "IPMI Device",
        39: "Power Supply",
        40: "Additional Information",
        41: "Onboard Devices Extended Information",
        42: "Management Controller Host Interface",
    }

    def dmidecode_parse(self, buffer):  # noqa: C901
        data = {}
        #  Each record is separated by double newlines
        split_output = buffer.split("\n\n")

        for record in split_output:
            record_element = record.splitlines()

            #  Entries with less than 3 lines are incomplete / inactive
            #  skip them
            if len(record_element) < 3:
                continue

            handle_data = DMIDecode.handle_re.findall(record_element[0])

            if not handle_data:
                continue
            handle_data = handle_data[0]

            dmi_handle = handle_data[0]

            data[dmi_handle] = {}
            data[dmi_handle]["DMIType"] = int(handle_data[1])
            data[dmi_handle]["DMISize"] = int(handle_data[2])

            #  Okay, we know 2nd line == name
            data[dmi_handle]["DMIName"] = record_element[1]

            in_block_elemet = ""
            in_block_list = ""

            #  Loop over the rest of the record, gathering values
            for i in range(2, len(record_element), 1):
                if i >= len(record_element):
                    break
                #  Check whether we are inside a \t\t block
                if in_block_elemet != "":

                    in_block_data = DMIDecode.in_block_re.findall(record_element[1])

                    if in_block_data:
                        if not in_block_list:
                            in_block_list = in_block_data[0][0]
                        else:
                            in_block_list = in_block_list + "\t\t"
                            +in_block_data[0][1]

                        data[dmi_handle][in_block_elemet] = in_block_list
                        continue
                    else:
                        # We are out of the \t\t block; reset it again, and let
                        # the parsing continue
                        in_block_elemet = ""

                record_data = DMIDecode.record_re.findall(record_element[i])

                #  Is this the line containing handle identifier, type, size?
                if record_data:
                    data[dmi_handle][record_data[0][0]] = record_data[0][1]
                    continue

                #  Didn't findall regular entry, maybe an array of data?
                record_data2 = DMIDecode.record2_re.findall(record_element[i])

                if record_data2:
                    #  This is an array of data - let the loop know we are
                    #  inside an array block
                    in_block_elemet = record_data2[0][0]
                    continue
        return data

    def size_to_gb(self, str):
        """Convert dmidecode memory size description to GB"""
        nb = re.search("[0-9]+", str)
        if nb:
            nb = int(re.search("[0-9]+", str).group())
        else:
            return 0
        if "MB" in str:
            return nb / 1024 if nb else 0
        elif "GB" in str:
            return nb
        else:
            return 0


class DMIDecode(DMIParse):
    """Wrapper over DMIParse which runs dmidecode locally"""

    def __init__(self, command="dmidecode"):
        self.dmidecode = command
        raw = self._run()
        super().__init__(raw)

    def _run(self):
        # let subprocess merge stderr with stdout
        proc = subprocess.Popen(self.dmidecode, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
        stdout, stderr = proc.communicate()
        if proc.returncode > 0:
            raise RuntimeError("{} failed with an error:\n{}".format(self.dmidecode, stdout.decode()))
        else:
            return stdout.decode()
