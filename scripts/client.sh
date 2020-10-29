cd ../src

make clean & make

./main -c "/dev/ttyS10" --baudrate B38400 --timeout 20 --num_transmissions 3 --alarm 3 --file "../files/pinguim.gif"