#! /bin/bash
#make clean
#make

for i in 5 10 20 50 100 #200
do
    for j in 10 50 100 500 1000
    do
	# i = num clients, j = num iteracions
	
	    echo "$i clients i $j iteracions\n"
	    ./launchClient $i $j localhost 5000 tests/prova-$i-$j
    done
done 