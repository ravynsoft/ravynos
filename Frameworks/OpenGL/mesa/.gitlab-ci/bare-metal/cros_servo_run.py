#!/usr/bin/env python3
#
# Copyright © 2020 Google LLC
# SPDX-License-Identifier: MIT

import argparse
import re
import sys

from custom_logger import CustomLogger
from serial_buffer import SerialBuffer


class CrosServoRun:
    def __init__(self, cpu, ec, test_timeout, logger):
        self.cpu_ser = SerialBuffer(
            cpu, "results/serial.txt", "R SERIAL-CPU> ")
        # Merge the EC serial into the cpu_ser's line stream so that we can
        # effectively poll on both at the same time and not have to worry about
        self.ec_ser = SerialBuffer(
            ec, "results/serial-ec.txt", "R SERIAL-EC> ", line_queue=self.cpu_ser.line_queue)
        self.test_timeout = test_timeout
        self.logger = logger

    def close(self):
        self.ec_ser.close()
        self.cpu_ser.close()

    def ec_write(self, s):
        print("W SERIAL-EC> %s" % s)
        self.ec_ser.serial.write(s.encode())

    def cpu_write(self, s):
        print("W SERIAL-CPU> %s" % s)
        self.cpu_ser.serial.write(s.encode())

    def print_error(self, message):
        RED = '\033[0;31m'
        NO_COLOR = '\033[0m'
        print(RED + message + NO_COLOR)
        self.logger.update_status_fail(message)

    def run(self):
        # Flush any partial commands in the EC's prompt, then ask for a reboot.
        self.ec_write("\n")
        self.ec_write("reboot\n")

        bootloader_done = False
        self.logger.create_job_phase("boot")
        tftp_failures = 0
        # This is emitted right when the bootloader pauses to check for input.
        # Emit a ^N character to request network boot, because we don't have a
        # direct-to-netboot firmware on cheza.
        for line in self.cpu_ser.lines(timeout=120, phase="bootloader"):
            if re.search("load_archive: loading locale_en.bin", line):
                self.cpu_write("\016")
                bootloader_done = True
                break

            # The Cheza firmware seems to occasionally get stuck looping in
            # this error state during TFTP booting, possibly based on amount of
            # network traffic around it, but it'll usually recover after a
            # reboot. Currently mostly visible on google-freedreno-cheza-14.
            if re.search("R8152: Bulk read error 0xffffffbf", line):
                tftp_failures += 1
                if tftp_failures >= 10:
                    self.print_error(
                        "Detected intermittent tftp failure, restarting run.")
                    return 1

            # If the board has a netboot firmware and we made it to booting the
            # kernel, proceed to processing of the test run.
            if re.search("Booting Linux", line):
                bootloader_done = True
                break

            # The Cheza boards have issues with failing to bring up power to
            # the system sometimes, possibly dependent on ambient temperature
            # in the farm.
            if re.search("POWER_GOOD not seen in time", line):
                self.print_error(
                    "Detected intermittent poweron failure, abandoning run.")
                return 1

        if not bootloader_done:
            self.print_error("Failed to make it through bootloader, abandoning run.")
            return 1

        self.logger.create_job_phase("test")
        for line in self.cpu_ser.lines(timeout=self.test_timeout, phase="test"):
            if re.search("---. end Kernel panic", line):
                return 1

            # There are very infrequent bus errors during power management transitions
            # on cheza, which we don't expect to be the case on future boards.
            if re.search("Kernel panic - not syncing: Asynchronous SError Interrupt", line):
                self.print_error(
                    "Detected cheza power management bus error, abandoning run.")
                return 1

            # If the network device dies, it's probably not graphics's fault, just try again.
            if re.search("NETDEV WATCHDOG", line):
                self.print_error(
                    "Detected network device failure, abandoning run.")
                return 1

            # These HFI response errors started appearing with the introduction
            # of piglit runs.  CosmicPenguin says:
            #
            # "message ID 106 isn't a thing, so likely what happened is that we
            # got confused when parsing the HFI queue.  If it happened on only
            # one run, then memory corruption could be a possible clue"
            #
            # Given that it seems to trigger randomly near a GPU fault and then
            # break many tests after that, just restart the whole run.
            if re.search("a6xx_hfi_send_msg.*Unexpected message id .* on the response queue", line):
                self.print_error(
                    "Detected cheza power management bus error, abandoning run.")
                return 1

            if re.search("coreboot.*bootblock starting", line):
                self.print_error(
                    "Detected spontaneous reboot, abandoning run.")
                return 1

            if re.search("arm-smmu 5040000.iommu: TLB sync timed out -- SMMU may be deadlocked", line):
                self.print_error("Detected cheza MMU fail, abandoning run.")
                return 1

            result = re.search("hwci: mesa: (\S*)", line)
            if result:
                if result.group(1) == "pass":
                    self.logger.update_dut_job("status", "pass")
                    return 0
                else:
                    self.logger.update_status_fail("test fail")
                    return 1

        self.print_error(
            "Reached the end of the CPU serial log without finding a result")
        return 1


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cpu', type=str,
                        help='CPU Serial device', required=True)
    parser.add_argument(
        '--ec', type=str, help='EC Serial device', required=True)
    parser.add_argument(
        '--test-timeout', type=int, help='Test phase timeout (minutes)', required=True)
    args = parser.parse_args()

    logger = CustomLogger("job_detail.json")
    logger.update_dut_time("start", None)
    servo = CrosServoRun(args.cpu, args.ec, args.test_timeout * 60, logger)
    retval = servo.run()

    # power down the CPU on the device
    servo.ec_write("power off\n")
    logger.update_dut_time("end", None)
    servo.close()

    sys.exit(retval)


if __name__ == '__main__':
    main()
