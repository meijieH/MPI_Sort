/*MPICH+VS2017+win8.1
题目描述: 将5000个整数并行排序
思路：master分批接收数据并进行归并排序，边接收数据边排序
（1）1-10进程分别给master（0号进程）发10条消息，每条消息中包含100个数
（2）master先分别接收其他进程1条消息，然后10路归并排序，
当某个进程发来的数据全部排完序写到硬盘后，又去读取该进程发的下一条消息
边接收边排序可使master只需要1000个数的内存存储数据。

测试时：mpiexec -n 11 MPI-demo.exe（n<11）*/
#include <stdio.h> 
#include <mpi.h>
#include<algorithm>
#include<iostream>

using namespace std;

int *arr;
int * readData(char * filename);
bool isFinished(int *flag, int *proFlag);
int main(int argc, char *argv[]) {

	int myid, numprocs, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	//cout << "start" << endl;
	MPI_Init(&argc, &argv);        // starts MPI 
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);  // get current process id 
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);      // get number of processes 
	MPI_Get_processor_name(processor_name, &namelen);

	if (myid != 0) {
		//cout << "child start: " << myid << endl;
		char filename[50] = "test1.2\\4.txt";//假设每个文件里有100个数-->之后再修改为1000个
		filename[8] = (char)('0' + (myid - 1));
		//cout << filename << endl;
		int * a = readData(filename);
		for (int i = 1; i < 11; i++) {
			MPI_Send(a + (i - 1) * 100, 100, MPI_INT, 0, i, MPI_COMM_WORLD);
		}
	}
	else {
		//cout << "parent start"  << endl;
		int **pointArr = new int*[numprocs];
		for (int i = 0; i < numprocs; i++) {
			pointArr[i] = new int[101];
		}
		FILE* fp;
		fopen_s(&fp, "test1.2\\n.txt", "w");
		int *flag = new int[numprocs];//记录i号进程指针拨到第几位
		memset(flag, 0, sizeof(int)*numprocs);
		int *proFlag = new int[numprocs];

		for (int i = 0; i < numprocs; i++) {
			proFlag[i] = 1;
		}

		for (int i = 1; i < numprocs; i++) {
			MPI_Recv(pointArr[i], 101, MPI_INT, i, proFlag[i], MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*接收消息*/
																									//cout << "第：" << proFlag[i] <<"来自:"<<i<< endl;
		}

		int min;
		int index = 1;
		int k = 0;
		while (k != 1000 * (numprocs - 1)) {
			for (int i = 1; i < numprocs; i++) {
				if (flag[i] != 100) {
					min = pointArr[i][flag[i]];
					index = i;
					break;
				}
				else {
					if (proFlag[i] == 10) {
						continue;
					}
					else {
						proFlag[i]++;
						MPI_Recv(pointArr[i], 101, MPI_INT, i, proFlag[i], MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*接收消息*/
																												//cout << "第：" << proFlag[i] << "来自:" << i << endl;
						flag[i] = 0;
						min = pointArr[i][flag[i]];
						index = i;
					}
				}
			}
			for (int i = 1; i < numprocs; i++) {
				if (flag[i] == 100) {
					if (proFlag[i] == 10) {
						continue;
					}
					else {
						proFlag[i]++;
						MPI_Recv(pointArr[i], 101, MPI_INT, i, proFlag[i], MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*接收消息*/
																												//cout << "第：" << proFlag[i] << "来自:" << i << endl;
						flag[i] = 0;
					}
				}
				if (min > pointArr[i][flag[i]]) {
					min = pointArr[i][flag[i]];
					index = i;
				}
			}
			fprintf_s(fp, "%d ", min);
			//cout << k << endl;
			flag[index]++;
			k++;
		}
		for (int i = 0; i < numprocs; i++) {
			delete pointArr[i];
		}
		delete pointArr;
		delete flag;
		delete proFlag;
	}
	MPI_Finalize();
	delete arr;

	return 0;
}

/*1~（n-1）号进程*/
int * readData(char * filename) { //数据个数已知用数组存储
	FILE* fp;
	fopen_s(&fp, filename, "r");
	arr = new int[1000];
	int i = 0;
	while (fscanf_s(fp, "%d", &arr[i], sizeof(int)) != EOF) {
		i++;
	}
	sort(arr, arr + 1000);
	return arr;
}