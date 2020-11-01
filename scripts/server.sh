cd ../src

make clean & make

./main -s "/dev/ttyS11" --baudrate B38400 --timeout 20 --num_transmissions 3 --alarm 3