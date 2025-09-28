echo before trap
trap 'echo caught sigint' 2
echo after trap

for i in 1 2 3
do
	echo $i
	sleep 5
done
