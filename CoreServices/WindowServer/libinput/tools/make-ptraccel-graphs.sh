#!/bin/bash

tool=`dirname $0`/../build/ptraccel-debug
gnuplot=/usr/bin/gnuplot

if [[ -e '$tool' ]]; then
	echo "Unable to find $tool"
	exit 1
fi
speeds="-1 -0.75 -0.5 -0.25 0 0.5 1"

outfile="ptraccel-linear"
for speed in $speeds; do
	$tool --mode=accel --dpi=1000 --filter=linear --speed=$speed > $outfile-$speed.gnuplot
done
$gnuplot <<EOF
set terminal svg enhanced background rgb 'white'
set output "$outfile.svg"
set xlabel "speed in mm/s"
set ylabel "accel factor"
set style data lines
set yrange [0:3]
set xrange [0:400]
speeds="$speeds"
fname(s)=sprintf("$outfile-%s.gnuplot", s)
plot for [s in speeds] fname(s) using 1:2 title s, \

EOF

outfile="ptraccel-low-dpi"
dpis="200 400 800 1000"
for dpi in $dpis; do
	$tool --mode=accel --dpi=$dpi --filter=low-dpi > $outfile-$dpi.gnuplot
done

$gnuplot <<EOF
set terminal svg enhanced background rgb 'white'
set output "$outfile.svg"
set xlabel "speed in mm/s"
set ylabel "accel factor"
set style data lines
set yrange [0:5]
set xrange [0:400]

dpis="$dpis"
fname(d)=sprintf("$outfile-%s.gnuplot", d)
lname(d)=sprintf("%sdpi", d)
plot for [dpi in dpis] fname(dpi) using 1:2 title lname(dpi), \

EOF


outfile="ptraccel-touchpad"
for speed in $speeds; do
	$tool --mode=accel --dpi=1000 --filter=touchpad --speed=$speed> $outfile-$speed.gnuplot
done

$gnuplot <<EOF
set terminal svg enhanced background rgb 'white'
set output "$outfile.svg"
set xlabel "speed in mm/s"
set ylabel "accel factor"
set style data lines
set xrange [0:400]
speeds="$speeds"
fname(s)=sprintf("$outfile-%s.gnuplot", s)
plot for [s in speeds] fname(s) using 1:2 title s, \

EOF

outfile="ptraccel-trackpoint"
for speed in $speeds; do
	$tool --mode=accel --speed=$speed --filter=trackpoint > $outfile-$speed.gnuplot
done
$gnuplot <<EOF
set terminal svg enhanced background rgb 'white'
set output "$outfile.svg"
set xlabel "delta (units/ms)"
set ylabel "accel factor"
set style data lines
set yrange [0:5]
set xrange [0:1]
speeds="$speeds"
fname(s)=sprintf("$outfile-%s.gnuplot", s)
plot for [s in speeds] fname(s) using 4:2 title s, \

EOF
