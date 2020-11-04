cd ../src

make clean & make

./main -c "/dev/ttyS10" --baudrate B38400 --timeout 5 --num_transmissions 3 --alarm 3 --chunk_size 1000 --file "../files/pinguim.gif"