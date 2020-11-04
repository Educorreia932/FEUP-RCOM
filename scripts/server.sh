cd ../src

make clean & make

./main -s "/dev/ttyS11" --baudrate B38400 --timeout 5 --num_transmissions 3 --alarm 3 --chunk_size 1000