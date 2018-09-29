my_rttmake: my_rtt.c
	 mpicc -O3 -o my_rtt my_rtt.c my_mpi.c -lm
