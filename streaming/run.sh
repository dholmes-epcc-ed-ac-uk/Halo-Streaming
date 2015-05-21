#!/bin/bash
rm out.bin out.dat
make
mpiexec -n $1 ./halo_streaming
hexdump -v -e '10/8 "%f "' -e '"\n"' < out.bin > out.dat
cd ../haloswap_test
rm halo_out.bin halo_out.dat
make
mpiexec -n $1 ./halo_swapping
hexdump -v -e '10/8 "%f "' -e '"\n"' < halo_out.bin > halo_out.dat
cd ../streaming
diff out.dat ../haloswap_test/halo_out.dat
