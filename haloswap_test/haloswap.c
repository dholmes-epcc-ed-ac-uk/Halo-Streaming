#include <stdio.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[])
{

	int array_size = 100;
	int iterations = 10;

	MPI_Init(NULL, NULL);

	int rank, size, reorder = 0, neighbour_left, neighbour_right, neighbour_up, neighbour_down, tag = 0, i, j;
	double local_data[array_size+2], new[array_size], start_time, time, total = 0, checksum;

	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	int amode = MPI_MODE_CREATE | MPI_MODE_WRONLY;
	MPI_File fh;

	neighbour_left = (rank + size - 1) % size;
	neighbour_right = (rank + 1) % size;

	MPI_Status status_left, status_right, file_status;
	MPI_Request request_left, request_right;

	srand(rank);

	for(i=1;i<=array_size;i++)
	{
		local_data[i] = (double)i-1;
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

		for(i=0;i<array_size;i++)
		{
			new[i] = (local_data[i] + local_data[i+1] + local_data[i+2])/3;
		}

		for(i=1;i<=array_size;i++)
		{
			local_data[i] = new[i-1];
		}
	}

	
/*	for(i=1;i<=array_size;i++)
	{
		total += local_data[i];
	}

	MPI_Reduce(&total,&checksum,1,MPI_DOUBLE,MPI_SUM,0,comm);*/

	if (rank == 0)
	{
		time = MPI_Wtime() - start_time;
//		checksum = checksum/(size*array_size);
//		printf("Checksum = %.10lf\n",checksum);
		printf("Halo exchange time taken = %lf seconds\n",time);
	}

	MPI_File_open(comm,"halo_out.bin",amode,MPI_INFO_NULL,&fh);
	MPI_Datatype filetype;
	MPI_Offset disp = rank*array_size*sizeof(double);
	MPI_Type_contiguous(array_size,MPI_DOUBLE,&filetype);
	MPI_Type_commit(&filetype);

	MPI_File_set_view(fh,disp,MPI_DOUBLE,filetype,"native",MPI_INFO_NULL);

	MPI_File_write_all(fh,&local_data[1],array_size,MPI_DOUBLE,&file_status);
	MPI_File_close(&fh);

	MPI_Finalize();

	return;
}
