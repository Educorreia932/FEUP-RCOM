cd ../src

make clean & make

./main -c "/dev/ttyS10" --baudrate B38400 --timeout 5 --num_transmissions 3 --alarm 3 --chunk-size 50 --file "../files/pinguim.gif"