#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

enum conditions {
	complete = 0,
	sorting = 1
};


typedef struct {
	int *array;
	int left;
	int right;
	enum conditions condition; 
} container_t;


void SwapElements(int* a, int* b){ 
	int t = *a; 
	*a = *b; 
	*b = t; 
} 


void PrintArray(int array[], int size){ 
	int i; 
	for (i=0; i < size; i++) 
		printf("%d ", array[i]); 
} 


int PartitionArray(int array[], int left, int right){ 
	int pivot = array[right]; 
	int index = (left - 1);

	for (int i = left; i < right; i++){ 
		if (array[i] < pivot){ 
			index++;
			SwapElements(&array[index], &array[i]); 
		} 
	} 
	SwapElements(&array[index + 1], &array[right]); 
	return (index + 1); 
} 


void *QuickSort(void *arguments){ 
    int partition;
	pthread_t t1, t2;
	container_t *container = (container_t *) arguments;
	container_t LeftPartition, RightPartition;


	//Apply Quick Sort
	if(container->left < container->right){
		//Partition Sub-Array		
		partition = PartitionArray(container->array, container->left, container->right);

		//Same Array
		LeftPartition.array = RightPartition.array = container->array;

		//Left Sub-Array
		LeftPartition.left = container->left;
		LeftPartition.right = partition - 1;

		//Right Sub-Array
		RightPartition.left = partition + 1;
		RightPartition.right = container->right;

		//Both Parts will Be Partitioned
		LeftPartition.condition = RightPartition.condition = sorting;
	
		pthread_create(&t1, NULL, QuickSort, (void *) &LeftPartition);
		pthread_create(&t2, NULL, QuickSort, (void *) &RightPartition);
		
		while(LeftPartition.condition == sorting || RightPartition.condition == sorting);
		container->condition = complete;
	} else {
		container->condition = complete;
		return NULL;
	}

	return NULL;
} 

// Driver program to test above functions 
int main(int argc, const char *argv[]){
	container_t container; 
	int *array;  
	FILE *fp;
	long int array_length;
	int i = 0;


	if (argc  != 3){
		printf("./executable <FILE> <SIZE>\n");
		return  -1;
	}
		
	fp = fopen(argv[1], "r");
	array_length = atoi(argv[2]);

	array = (int *) malloc(sizeof(int) * array_length);
	if (!array)
		return -1;

	

  	while (!feof (fp)){
      	fscanf (fp, "%d", &array[i]);
		i++;
    }
	
	container.array = array;
	container.left  = 0;
	container.right = array_length - 1;
	container.condition = sorting;

	PrintArray(array, array_length);
	printf(" ~~~>  ");

    QuickSort((void *) &container);

	//Wait To Sort
	while(container.condition == sorting);
	PrintArray(array, array_length); 
    printf("\n");
	return 0; 
}
