#include <stdio.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[])
{

	int array_size = 1000000;
	int iterations = 1000;

	MPI_Init(NULL, NULL);

	int rank, size, reorder = 0, neighbour_left, neighbour_right, neighbour_up, neighbour_down, tag = 0, i, j;
	double local_data[array_size+2], start_time, time, total = 0, checksum, average_time;

	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);

	neighbour_left = (rank + size - 1) % size;
	neighbour_right = (rank + 1) % size;

	MPI_Status status_left, status_right;
	MPI_Request request_left, request_right;

	printf("Neighbours of %i are %i,%i\n",rank,neighbour_left,neighbour_right);

	srand(rank);

	for(i=1;i<=array_size;i++)
	{
		local_data[i] = (double)(rand()%100);
	}

	if(rank == 0)
	{
		start_time = MPI_Wtime();
	}
	for(j=0;j<iterations;j++)
	{
		MPI_Issend(&local_data[1],1,MPI_DOUBLE,neighbour_left,tag,comm,&request_left);
		MPI_Issend(&local_data[array_size],1,MPI_DOUBLE,neighbour_right,tag,comm,&request_right);

		MPI_Recv(&local_data[array_size+1],1,MPI_DOUBLE,neighbour_right,tag,comm,&status_right);
		MPI_Recv(&local_data[0],1,MPI_DOUBLE,neighbour_left,tag,comm,&status_left);

		MPI_Wait(&request_left,&status_left);
		MPI_Wait(&request_right,&status_right);

		for(i=1;i<=array_size;i++)
		{
			local_data[i] = (local_data[i-1] + local_data[i] + local_data[i+1])/3;
		}
	}

	
	for(i=1;i<=array_size;i++)
	{
		total += local_data[i];
	}

	MPI_Reduce(&total,&checksum,1,MPI_DOUBLE,MPI_SUM,0,comm);

	if (rank == 0)
	{
		time = MPI_Wtime() - start_time;
		checksum = checksum/(size*array_size);
		printf("Checksum = %lf\n",checksum);
		printf("Time taken = %lf seconds\n",time);
	}

	MPI_Finalize();

	return;
}
