/*MPICH+VS2017+win8.1
��Ŀ����: ��5000��������������
˼·��
(1)��5000���������ֳ�5���ļ���5�����̲��ж�ȡ������ÿ���ļ�����1000���������Կո�ֿ�
(2)Ȼ��5�����̷ֱ��master���̣�0�Ž��̣���һ����Ϣ(������������)
(3)master���̽�����Ϣ�󣬲�����·�鲢��������������д��Ӳ��
ȱ�㣺��������ʱ��master����û���㹻�ڴ����洢�������ݲ�����
����취�������������ݣ�master�����������ݲ����й鲢���򣬱߽������ݱ����򣬼�mpiSort2

����ʱ��mpiexec -n 6 MPI-demo.exe(��n<10����Ϊ�����ļ�û����ô�ࡤ����)
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

/*1~��n-1���Ž���*/
int * readData(char * filename) { //��������vector�洢
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
		MPI_Send(a, 1000, MPI_INT, 0, 1, MPI_COMM_WORLD); //����send��0�Ž��̷�������
														  //cout<<"I am parent��"<<myid<<endl;

	}
	else {
		int **pointArr = new int*[numprocs];
		for (int i = 0; i < numprocs; i++) {
			pointArr[i] = new int[1001];
		}
		FILE* fp;
		fopen_s(&fp, "test1.1\\m.txt", "w");

		for (int i = 1; i < numprocs; i++) {
			MPI_Recv(pointArr[i], 1001, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*������Ϣ*/
																										 //cout << "��Ϣ��" << i << endl;
		}

		int min;
		int *flag = new int[numprocs];
		memset(flag, 0, sizeof(int) * numprocs);
		int index = 1;
		int accout = (numprocs - 1) * 1000;
		//std:: cout << "I am child��" << myid << endl;
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