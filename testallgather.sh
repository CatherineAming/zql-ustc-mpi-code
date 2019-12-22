mpicc allgather.c -o allgather -lm -std=c99
scp -r ./* pp11@node1:/home/pp11/SA18011052/homework/source/
scp -r ./* pp11@node2:/home/pp11/SA18011052/homework/source/
scp -r ./* pp11@node4:/home/pp11/SA18011052/homework/source/
mpirun -f ./mpi_config ./allgather
