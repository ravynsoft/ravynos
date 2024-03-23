#!/bin/bash

# Must be run from inside a test/output directory after a cairo
# test has finished running.

grep -h ^TEST *.log > results.txt
echo "Results written to results.txt"

total=$(grep "RESULT:" results.txt | wc -l)
total_pass=$(grep "RESULT: PASS" results.txt | wc -l)
total_fail=$(grep "RESULT: FAIL" results.txt | wc -l)
total_xfail=$(grep "RESULT: XFAIL" results.txt | wc -l)
total_error=$(grep "RESULT: ERROR" results.txt | wc -l)
total_crash=$(grep "RESULT: CRASHED" results.txt | wc -l)
total_untested=$(grep "RESULT: UNTESTED" results.txt | wc -l)
total_calc=$((total_pass + total_fail + total_error + total_xfail + total_crash + total_untested))

echo "#######"                             | tee -a results.txt
echo "# Tests run:        $total"          | tee -a results.txt
echo "# Passed:           $total_pass"     | tee -a results.txt
echo "# Failed:           $total_fail"     | tee -a results.txt
echo "# Expected Failed:  $total_xfail"    | tee -a results.txt
echo "# Error:            $total_error"    | tee -a results.txt
echo "# Crashed:          $total_crash"    | tee -a results.txt
echo "# Untested:         $total_untested" | tee -a results.txt
echo "# Total:            $total_calc"     | tee -a results.txt
echo "#######"                             | tee -a results.txt


