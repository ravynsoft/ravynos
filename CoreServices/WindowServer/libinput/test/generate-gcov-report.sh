#!/bin/bash -e

if [[ $# -lt 2 ]]; then
    echo "Usage: ./generate-gcov-report.sh <rel-target-dir> <srcdir> [<srcdir> ... ]"
    exit 1
fi

target_dir=$1
shift
source_dirs=$*

if [[ "${target_dir:0:1}" != '/' ]]; then
    target_dir="$PWD/$target_dir"
fi
summary_file="$target_dir/summary.txt"

mkdir -p "$target_dir"
rm -f "$target_dir"/*.gcov

for dir in $source_dirs; do
	pushd "$dir" > /dev/null
	for file in *.c; do
		find ./ -name "*${file/\.c/.gcda}" \
			\!  -path "*selftest*" \
			-exec gcov {} \; > /dev/null
	done
	find ./ -name "*.gcov" \
		\! -path "*/`basename "$target_dir"`/*" \
		-exec mv {} "$target_dir" \;
	popd > /dev/null
done

echo "========== coverage report ========" > "$summary_file"
for file in "$target_dir"/*.gcov; do
	total=`grep -v " -:" "$file" | wc -l`
	missing=`grep "#####" "$file" | wc -l`
	hit=$((total - missing));
	percent=$((($hit * 100)/$total))
	fname=`basename "$file"`
	printf "%-50s total lines: %4s not tested: %4s (%3s%%)\n" "$fname" "$total" "$missing" "$percent">> "$summary_file"
done
echo "========== =============== ========" >> "$summary_file"
