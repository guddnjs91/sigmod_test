#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <utility>

using namespace std;

typedef unsigned int vertex;

unsigned int VERTICES_NUM = 1600000;
unsigned int MAX_EDGES_NUM = 3200000;

unsigned int QUERIES_NUM = 100;
unsigned int BATCH_SIZE = 1000;

unsigned int edge_counter;

pair<vertex, vertex> S_CLASS;
pair<vertex, vertex> A_CLASS;
pair<vertex, vertex> B_CLASS;
pair<vertex, vertex> NO_CLASS;

double s_class_per = 0.00001;
double a_class_per = 0.07;
double dec_rate = 0.45;

unsigned int *rand_arr;
unsigned int arr_index;

// Print the usage instructions for the harness
void usage()
{
	cerr << "Usage: main <scale> <init-file> <workload-file> " << endl;
}

bool InRange(vertex v, pair<vertex, vertex> c)
{
	if(v >= c.first && v <= c.second)
		return true;
	else
		return false;
}

vertex RandomVertex(vertex v1, vertex v2)
{
	return v1 + rand() % (v2-v1);
}

char RandomCmd()
{
	if(rand() % 10 > 7)
		return 'Q';
	else
	{
		if(rand() % 2 == 0)
			return 'A';
		else
			return 'D';
	}

}

unsigned int * PickUniformRandArr(unsigned int *arr, int len, vertex v1, vertex v2)
{
	unsigned int interval = (v2 - v1) / len;

	if(interval != 0)
	{
		for(int i = 0; i < len; i++)
		{	
			arr[i] = v1 + i * interval + rand()% interval;
		}
	}
	else
	{
		for(int i =0; i < len; i++)
			arr[i] = v1 + rand() % len;
	}

	return arr;
}

int main(int argc, const char * argv[]){
	
	// Check for the correct number of arguments
	if (argc != 4)
	{
		usage();
		exit(EXIT_FAILURE);
	}

	if(strcmp(argv[1], "S") == 0 || strcmp(argv[1], "small") == 0)
		;
	else if(strcmp(argv[1], "M") == 0 || strcmp(argv[1], "medium") == 0)
	{
		VERTICES_NUM *= 5;
		MAX_EDGES_NUM *= 5;
		QUERIES_NUM *= 5;

	}
	else if(strcmp(argv[1], "L") == 0 || strcmp(argv[1], "large") == 0)
	{
		VERTICES_NUM *= 10;
		MAX_EDGES_NUM *= 10;
		QUERIES_NUM *= 10;
	}
	else
	{
		puts("Scale Choose : 'S', 'M', 'L' ");
		exit(EXIT_FAILURE);
	}


	srand ( (unsigned int)time(NULL) );

	FILE *fp = fopen(argv[2], "w");
	if(fp == NULL)
	{
		puts("can't write to init-file");
		return -1;
	}

	unsigned int i, j;
	vertex v1;
	vertex v2;

	rand_arr = (unsigned int *)malloc(sizeof(unsigned int)*1000);
	
	// CLASS INIT
	S_CLASS = make_pair(0, VERTICES_NUM * s_class_per);
	A_CLASS = make_pair(S_CLASS.second + 1, S_CLASS.second + VERTICES_NUM * a_class_per);
	B_CLASS = make_pair(A_CLASS.second + 1, A_CLASS.second + A_CLASS.second * dec_rate);

	// 0번 노드에서는 A_CLASS의 모든 노드와 연결
	for(v2 = 0; v2 <= A_CLASS.second; v2++)
	{
		fprintf(fp, "%u\t%u\n", v1, v2);
		edge_counter++;
	}
	printf("0번 node edge 개수 : %u\n", edge_counter);

	rand_arr = PickUniformRandArr(rand_arr, 1000, A_CLASS.first, A_CLASS.second);
	// for(i = 0; i < 1000; i++)
	// {
	// 	printf("%u\n", rand_arr[i]);
	// }


	unsigned int edge_bound = edge_counter * dec_rate;

	// 1번 노드부터 나머지 S_CLASS 노드까지
	for(v1 = S_CLASS.first + 1 ; v1 <= S_CLASS.second; v1++)
	{
		for(i = 0; i <= edge_bound; i++)
		{
			v2 = i;

			if(v2 == v1)  // 중복 제거
				continue;

			if(InRange(v2, S_CLASS))
			{
				fprintf(fp, "%u\t%u\n", v1, v2); // S_CLASS는 전부 포함
			}
			else
			{
				if(rand()%2 == 1)
				{
					// A_CLASS에서 uniform 형태로 뽑기
					int bound = rand()%15 + 1;
					for(int j = 0; j < bound; j++)
						fprintf(fp, "%u\t%u\n", v1, v2 + j);
					i += bound - 1;
					edge_counter += bound - 1;
				}
				else
				{
					int bound = rand()%15 + 1;
					for(int j = 0; j < bound; j++)
						fprintf(fp, "%u\t%u\n", v1, B_CLASS.first + v2 + j - S_CLASS.second);
					i += bound - 2;
					edge_counter += bound - 1;
				}
			}
			edge_counter++;
		}

		B_CLASS.first = B_CLASS.first + v2;
		B_CLASS.second = B_CLASS.first + i;
		edge_bound *= dec_rate;
	}

	NO_CLASS = make_pair(B_CLASS.second + 1, VERTICES_NUM);

	i = 0;
	vertex nc_idx = NO_CLASS.first;

	for(v1 = A_CLASS.first; (edge_counter < MAX_EDGES_NUM) ; v1++)
	{

		unsigned int prob = rand() % 10;
		unsigned int bound;

		if(prob > 5)
		{
			// A_CLASS에서
			if(rand() % 2)
				v2 = rand_arr[rand()%100];
			else
				v2 = rand_arr[rand()%1000];
			bound = rand()%29 + 1;
			for(i = 0; i < bound; i++)
			{
				fprintf(fp, "%u\t%u\n", v1, v2 + i);
			}
			edge_counter += bound;
		}
		else
		{
			if(rand() % 5 > 2)
			{
				// NO_CLASS에서
				vertex temp_v1 = NO_CLASS.first + nc_idx;
				bound = rand() % 15 + 1;
				for(i = 0; i < bound; i++)
				{
					if(rand() % 5 > 2)
					{
						fprintf(fp, "%u\t%u\n", temp_v1, RandomVertex(S_CLASS.first, S_CLASS.second));
					}
					else
					{

						fprintf(fp, "%u\t%u\n", temp_v1, rand_arr[rand()%1000] + rand()% 100);
					}
					temp_v1++;
				}
				nc_idx += bound;
				edge_counter += bound;
			}
			else
			{
				// NO_CLASS 끼리
				vertex temp_v1 = NO_CLASS.first + nc_idx;
				bound = rand() % 6 + 1;
				for(i = 0; i < bound; i++)
				{
					fprintf(fp, "%u\t%u\n", temp_v1, temp_v1+1);
					temp_v1 += 2;
				}
				nc_idx += 2 * bound;
				edge_counter += bound;
			}

		}

	}


	printf("총 %u개 edge 할당\n", edge_counter);

	fprintf(fp,"S\n");	// end of initial graph

	FILE *fp2 = fopen(argv[3], "w");

	for(i = 0; i < QUERIES_NUM; i++)
	{

		for(j = 0; j < BATCH_SIZE; j++)
		{
			fprintf(fp2, "%c %u %u\n", RandomCmd(), rand() % VERTICES_NUM, rand()% VERTICES_NUM);
		}
		fprintf(fp2, "F\n");	
	}

	fclose(fp);
	fclose(fp2);

	return 0;
}
