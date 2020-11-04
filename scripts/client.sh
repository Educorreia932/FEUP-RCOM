cd ../src

make clean & make

./main -c "/dev/ttyS0" --baudrate B38400 --timeout 5 --num_transmissions 3 --alarm 3 --chunk_size 256 --file "../files/pinguim.gif"
