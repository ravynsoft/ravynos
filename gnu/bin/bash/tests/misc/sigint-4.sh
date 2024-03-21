trap 'echo sigint' 2

sleep 5 &
sleep 5 &
sleep 5 &

echo wait 1
wait

echo wait 2
wait

exit
