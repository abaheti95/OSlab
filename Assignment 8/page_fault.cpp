#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>

#define N 3
#define INFINITY 1000000000

using namespace std;

int mat1[N][N];
int mat2[N][N];
int out[N][N];

vector<int> input;

void print_vector(vector<int> &v)
{
	int i;			// Iterating Variable
	for(i = 0; i < (int)v.size(); i++)
		printf("%d ",v[i]);
	printf("\n");
}

void print_matrix(int mat[N][N],int n)
{
	int i,j;		// Iterating Variables
	for(i = 0; i < n; i++)
	{
		for(j = 0; j < n; j++)
			printf("%d ",mat[i][j]);
		printf("\n");
	}
}

void initialize(int n)
{
	// function that initializes the matrics
	int i,j,k;		// iterating variables
	
	// initializing matrix 1
	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			scanf("%d",&mat1[i][j]);

	// initializing matrix 2
	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			scanf("%d",&mat2[i][j]);

	// initializing output matrix
	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			scanf("%d",&out[i][j]);

	// generating input string for the page table
	
	for(i = 0; i < n; i++)
	{
		for(j = 0; j < n; j++)
		{
			// mat1 row and mat 2 column and output index
			for(k = 0; k < n; k++)
			{
				input.push_back(mat1[i][k]);
				input.push_back(mat2[k][j]);
			}
			input.push_back(out[i][j]);
		}
	}
}

int fifo(int d)
{
	int i;				// Iterating Variable
	vector<int> v;		// Vector that will be used as FIFO queue
	int count = 0;		// Counter for number of page faults

	// giving the input vector of pages as input to FIFO algorithm
	for(i = 0; i < (int)input.size(); i++)
	{
		// search for the input page in queue
		if(find(v.begin(),v.end(),input[i]) == v.end())
		{
			// not found in page table
			if((int)v.size() == d)
			{
				// pop the first element as the queue is full
				v.erase(v.begin());
			}
			v.push_back(input[i]);
			count++;
		}
	}
	return count;
}

int lru(int d)
{
	int i;				// Iterating Variable
	vector<int> v;		// Vector that will be used as FIFO queue
	int count = 0;		// Counter for number of page faults

	vector<int>::iterator it;

	// giving the input vector of pages as input to FIFO algorithm
	for(i = 0; i < (int)input.size(); i++)
	{
		// search for the input page in queue
		if((it = find(v.begin(),v.end(),input[i])) == v.end())
		{
			// not found in page table
			if((int)v.size() == d)
			{
				// pop the first element as the queue is full
				v.erase(v.begin());
			}
			v.push_back(input[i]);
			count++;
		}
		else
		{
			// page found so change priority
			int temp = *it;
			v.erase(it);
			v.push_back(temp);
		}
	}
	return count;
}

int lfu(int d)
{
	int i,j;				// iterating variable
	int count = 0;			// Counter for number of page faults
	int frequency[d][2];	// first element is the page and second element is the frequency of page

	// initialize frequency
	for(j = 0; j < d; j++)
	{
		frequency[j][0] = -1;
		frequency[j][1] = 0;
	}
	// giving the input vector of pages as input to FIFO algorithm
	for(i = 0; i < (int)input.size(); i++)
	{
		for(j = 0; j < d; j++)
		{
			if(frequency[j][0] == input[i])
			{
				// page found
				// change frequency
				frequency[j][1]++; 
				break;
			}
		}
		if(j == d)
		{
			// page fault
			// replace page of least frequency with this
			int min = INFINITY;
			int p = 0;
			for(j = 0; j < d; j++)
				if(min > frequency[j][1] || frequency[j][0] == -1)
				{
					min = frequency[j][0];
					p = j;
				}
			frequency[p][0] = input[i];
			frequency[p][1] = 0;
			count++;
		}
	}
	return count;
}

int second_chance(int d)
{
	int i,j;				// iterating variable
	int count = 0;			// Counter for number of page faults
	int queue[d][2];		// queue is page id and reference bit
	int curr = 0;
	// initialize frequency
	for(j = 0; j < d; j++)
	{
		queue[j][0] = -1;
		queue[j][1] = 0;
	}
	// giving the input vector of pages as input to FIFO algorithm
	for(i = 0; i < (int)input.size(); i++)
	{
		for(j = 0; j < d; j++)
		{
			if(queue[j][0] == input[i])
			{
				// page found
				// change page reference bit
				queue[j][1] = 1; 
				break;
			}
		}
		if(j == d)
		{
			// page fault
			// replace page using second chance logic
			do
			{
				if(queue[curr][0] == -1)
				{
					// filling the queue and adding for the first time
					queue[curr][0] = input[i];
					queue[curr][1] = 0;
				}
				else if(queue[curr][1] == 1)	// page referenced
				{
					// give second chance
					queue[curr][1] = 0;
					curr = (curr+1)%d;
				}
				else
				{
					// replace this page
					queue[curr][0] = input[i];
					queue[curr][1] = 0;
					curr = (curr+1)%d;
					break;
				}
			}while(true);
			count++;
		}
	}
	return count;
}

int main()
{
	int n,d;
	// n is the size of the matrix
	// d is the number of pages that can be assigned

	scanf("%d",&n);
	scanf("%d",&d);
	initialize(n);
	printf("n = %d d = %d\n",n,d);
	printf("First Matrix :\n");
	print_matrix(mat1,n);
	printf("Second Matrix :\n");
	print_matrix(mat2,n);
	printf("Output Matrix :\n");
	print_matrix(out,n);
	printf("Size of Input = %lu\n",input.size());
	printf("Input String\n");
	print_vector(input);
	printf("Page faults in FIFO : %d\n",fifo(d));
	printf("Page faults in LFU : %d\n",lfu(d));
	printf("Page faults in LRU : %d\n",lru(d));
	printf("Page faults in Second Chance Algorithm : %d\n",second_chance(d));
	return 0;
}