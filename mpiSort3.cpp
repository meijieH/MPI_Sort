/*环境：MPICH+VS2017+win8.1
并行奇偶交换排序，参看《并行程序设计导论》
思路：
(1)设有n个数，共有p个进程并行处理,每个进行负责处理n/p个数据，首先每个进程里的数据进行内部排序
(2)第1轮，0号进程和1号进程交换数据，2号进程和3号进程交换数据，···,2n号进程和2n+1号进程交换数据
(3)第2轮，1号进程和2号进程交换数据，3号进程和4号进程交换数据，···,2n-1号进程和2n号进程交换数据
(4)第3轮，0号进程和1号进程交换数据，2号进程和3号进程交换数据，···,2n号进程和2n+1号进程交换数据
(5)重复以上，直至第n轮，所有数据排序完毕

测试时：mpiexec -n 5 MPI-demo.exe（n可任意值）*/
#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<algorithm>
using namespace std;

//get your partner
int Compute_partner(int phase, int rank, int  numprocs) {
	int partner;
	if (phase % 2 == 0) {
		if (rank % 2 != 0)
			partner = rank - 1;
		else
			partner = rank + 1;
	}
	else {
		if (rank % 2 != 0)
			partner = rank + 1;
		else
			partner = rank - 1;
	}
	if (partner == -1 || partner == numprocs)
		partner = MPI_PROC_NULL;
	return partner;
}

void merge(int *a, int *b, int n) {
	int *array = new int[2 * n];
	int i, j, k;
	i = j = k = 0;
	while (i<n&&j<n) {
		if (a[i] <= b[j])
			array[k++] = a[i++];
		else
			array[k++] = b[j++];
	}
	while (i < n&&j == n) {
		array[k++] = a[i++];
	}
	while (i == n&&j <n) {
		array[k++] = b[j++];
	}
	for (int i = 0; i < n; i++) {
		a[i] = array[i];
		b[i] = array[n + i];
	}
}

int main(int argc, char *argv[]) {

	int myid, numprocs, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc, &argv);        // starts MPI 
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);  // get current process id 
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);      // get number of processes 
	MPI_Get_processor_name(processor_name, &namelen);
	int n = 4;  //define freely
	int *init_buffer;

	if (myid == 0) {
		init_buffer = new int[n*numprocs];
		for (int i = 0; i<n*numprocs; i++)//produce n*numprocs random numbers and print them
			init_buffer[i] = rand() % 100;
		cout << "Original: ";
		for (int i = 0; i<n * numprocs; i++)
			cout << init_buffer[i] << ((i%n == (n - 1)) ? "|" : ",");
		cout << "\n";

		int * array = new int[n];
		int * receiveBuff = new int[n];

		MPI_Scatter(init_buffer, n, MPI_INT, array, n, MPI_INT, 0, MPI_COMM_WORLD);//MPI_Scatter several processes
																				   //cout << "myid=" << myid<<"data:" << array[0] << "   " << array[1] << endl;
		sort(array, array + n);
		for (int phase = 0; phase< numprocs; phase++) {
			MPI_Status status;
			int partner = Compute_partner(phase, myid, numprocs);
			if (partner != MPI_PROC_NULL) {
				MPI_Sendrecv(array, n, MPI_INT, partner, 0, receiveBuff, n, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
				//cout <<"phase="<< phase<<"    myid=" << myid << "data:" << array[0] << "   " << array[1] << endl;
				if (myid<partner) {
					merge(array, receiveBuff, n);
				}
				else {
					merge(receiveBuff, array, n);
				}
			}
			MPI_Gather(array, n, MPI_INT, init_buffer, n, MPI_INT, 0, MPI_COMM_WORLD);
			cout << "Sorting : ";
			for (int i = 0; i< numprocs*n; i++)
				cout << init_buffer[i] << ((i%n == (n - 1)) ? "|" : ",");
			cout << "\n";
		}

		MPI_Gather(array, n, MPI_INT, init_buffer, n, MPI_INT, 0, MPI_COMM_WORLD);
		cout << "Sorted  : ";
		for (int i = 0; i<numprocs*n; i++)
			cout << init_buffer[i] << ((i%n == (n - 1)) ? "|" : ",");
		cout << "\n";
		delete[] receiveBuff;
		delete[] array;
	}
	else {
		int * array = new int[n];
		int * receiveBuff = new int[n];
		init_buffer = new int[n*numprocs];

		MPI_Scatter(init_buffer, n, MPI_INT, array, n, MPI_INT, 0, MPI_COMM_WORLD);

		//cout << "myid=" << myid<<"data:" << array[0] << "   " << array[1] << endl;
		sort(array, array + n);
		for (int phase = 0; phase< numprocs; phase++) {
			MPI_Status status;
			int partner = Compute_partner(phase, myid, numprocs);
			if (partner != MPI_PROC_NULL) {
				MPI_Sendrecv(array, n, MPI_INT, partner, 0, receiveBuff, n, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
				//cout << "phase=" << phase << "    myid=" << myid << "data:" << array[0] << "   " << array[1] << endl;
				if (myid<partner) {
					merge(array, receiveBuff, n);
				}
				else {
					merge(receiveBuff, array, n);
				}
			}
			MPI_Gather(array, n, MPI_INT, init_buffer, n, MPI_INT, 0, MPI_COMM_WORLD);
		}

		MPI_Gather(array, n, MPI_INT, init_buffer, n, MPI_INT, 0, MPI_COMM_WORLD);

		delete[] init_buffer;
		delete[] receiveBuff;
		delete[] array;
	}
	MPI_Finalize();
	return 0;
}