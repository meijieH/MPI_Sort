/*MPICH+VS2017+win8.1
��Ŀ����: ��5000��������������
˼·��master�����������ݲ����й鲢���򣬱߽������ݱ�����
��1��1-10���̷ֱ��master��0�Ž��̣���10����Ϣ��ÿ����Ϣ�а���100����
��2��master�ȷֱ������������1����Ϣ��Ȼ��10·�鲢����
��ĳ�����̷���������ȫ��������д��Ӳ�̺���ȥ��ȡ�ý��̷�����һ����Ϣ
�߽��ձ������ʹmasterֻ��Ҫ1000�������ڴ�洢���ݡ�

����ʱ��mpiexec -n 11 MPI-demo.exe��n<11��*/
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
		char filename[50] = "test1.2\\4.txt";//����ÿ���ļ�����100����-->֮�����޸�Ϊ1000��
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
		int *flag = new int[numprocs];//��¼i�Ž���ָ�벦���ڼ�λ
		memset(flag, 0, sizeof(int)*numprocs);
		int *proFlag = new int[numprocs];

		for (int i = 0; i < numprocs; i++) {
			proFlag[i] = 1;
		}

		for (int i = 1; i < numprocs; i++) {
			MPI_Recv(pointArr[i], 101, MPI_INT, i, proFlag[i], MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*������Ϣ*/
																									//cout << "�ڣ�" << proFlag[i] <<"����:"<<i<< endl;
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
						MPI_Recv(pointArr[i], 101, MPI_INT, i, proFlag[i], MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*������Ϣ*/
																												//cout << "�ڣ�" << proFlag[i] << "����:" << i << endl;
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
						MPI_Recv(pointArr[i], 101, MPI_INT, i, proFlag[i], MPI_COMM_WORLD, MPI_STATUSES_IGNORE);/*������Ϣ*/
																												//cout << "�ڣ�" << proFlag[i] << "����:" << i << endl;
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

/*1~��n-1���Ž���*/
int * readData(char * filename) { //���ݸ�����֪������洢
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