/*MPICH+VS2017+win8.1
题目描述: 将5000个整数并行排序
思路：
(1)将5000个整数，分成5个文件由5个进程并行读取并排序，每个文件中有1000个整数，以空格分开
(2)然后5个进程分别给master进程（0号进程）发一条消息(即排序后的数据)
(3)master进程接收消息后，采用五路归并对所有数据排序并写入硬盘
缺点：数据量大时，master进程没有足够内存来存储所有数据并排序
解决办法：分批发送数据，master分批接收数据并进行归并排序，边接收数据边排序，见mpiSort2

测试时：mpiexec -n 6 MPI-demo.exe(需n<10，因为测试文件没有那么多···)
*/
#include <stdio.h> 
#include<iostream>
#include <mpi.h>
#include<vector>
#include<algorithm>
#include<fstream>

using namespace std;

vector<int> v;
int *arr;

/*1~（n-1）号进程*/
int * readData(char * filename) { //读数据用vector存储
	FILE* fp;
	fopen_s(&fp, filename, "r");
	int tmp;
	while (fscanf_s(fp, "%d", &tmp, sizeof(int)) != EOF) {
		v.push_back(tmp);
	}
	sort(v.begin(), v.end());
	arr = new int[v.size()];
	for (int i = 0; i < v.size(); i++) {
		arr[i] = v[i];
	}
	return arr;
}

int main(int argc, char *argv[]) {

	int myid, numprocs, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);        // starts MPI 
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);  // get current process id 
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);      // get number of processes 
	MPI_Get_processor_name(processor_name, &namelen);

	v = *(new vector<int>());

	if (myid != 0) {
		char filename[50] = "test1.1\\1.txt";
		filename[8] = (char)('0' + (myid - 1));
		cout << filename << endl;
		int * a = readData(filename);
		MPI_Send(a, 1000, MPI_INT, 0, 1, MPI_COMM_WORLD); //调用send向0号进程发送数据
														  //cout<<"I am parent："<<myid<<endl;

	}
	else {
		int **pointArr = new int*[numprocs];
		for (int i = 0; i < numprocs; i++) {
			pointArr[i] = new int[1001];
		}
		FILE* fp;
		fopen_s(&fp, "test1.1\\m.txt", "w");

		for (int i = 1; i < numprocs; i++) {
			MPI_Recv(pointArr[i], 1001, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*接收消息*/
																										 //cout << "消息：" << i << endl;
		}

		int min;
		int *flag = new int[numprocs];
		memset(flag, 0, sizeof(int) * numprocs);
		int index = 1;
		int accout = (numprocs - 1) * 1000;
		//std:: cout << "I am child：" << myid << endl;
		while (accout>0) {
			for (int i = 1; i < numprocs; i++) {
				if (flag[i] != 1000) {
					min = pointArr[i][flag[i]];
					index = i;
					break;
				}
			}
			for (int i = 1; i < numprocs; i++) {
				if (flag[i] == 1000) {
					continue;
				}
				if (min >pointArr[i][flag[i]]) {
					min = pointArr[i][flag[i]];
					index = i;
				}
			}
			fprintf_s(fp, "%d ", min);
			flag[index]++;
			accout--;
		}
		for (int i = 0; i < numprocs; i++) {
			delete pointArr[i];
		}
		delete pointArr;
		delete flag;
	}
	MPI_Finalize();
	delete arr;
	return 0;
}