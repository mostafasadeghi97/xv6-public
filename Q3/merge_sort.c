// C program to implement concurrent merge sort 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 

void insertionSort(int arr[], int n); 
void merge(int a[], int l1, int h1, int h2); 



void merge2(int arr[], int l, int m, int r) 
{ 
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 =  r - m; 
  
    /* create temp arrays */
    int L[n1], R[n2]; 
  
    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++) 
        L[i] = arr[l + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = arr[m + 1+ j]; 
  
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray 
    j = 0; // Initial index of second subarray 
    k = l; // Initial index of merged subarray 
    while (i < n1 && j < n2) 
    { 
        if (L[i] <= R[j]) 
        { 
            arr[k] = L[i]; 
            i++; 
        } 
        else
        { 
            arr[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
  
    /* Copy the remaining elements of L[], if there 
       are any */
    while (i < n1) 
    { 
        arr[k] = L[i]; 
        i++; 
        k++; 
    } 
  
    /* Copy the remaining elements of R[], if there 
       are any */
    while (j < n2) 
    { 
        arr[k] = R[j]; 
        j++; 
        k++; 
    } 
} 

void mergeSort2(int arr[], int l, int r) 
{ 
    if (l < r) 
    { 
        // Same as (l+r)/2, but avoids overflow for 
        // large l and h 
        int m = l+(r-l)/2; 
  
        // Sort first and second halves 
        mergeSort2(arr, l, m); 
        mergeSort2(arr, m+1, r); 
  
        merge2(arr, l, m, r); 
    } 
} 


void mergeSort(int a[], int l, int h) 
{ 
	int i, len=(h-l+1); 

	// Using insertion sort for small sized array 
	if (len<=5) 
	{ 
		mergeSort2(a, l ,h);
		return; 
	} 

	pid_t lpid,rpid; 
	lpid = fork(); 
	if (lpid<0) 
	{ 
		// Lchild proc not created 
		perror("Left Child Proc. not created\n"); 
		_exit(-1); 
	} 
	else if (lpid==0) 
	{ 
		mergeSort(a,l,l+len/2-1); 
		_exit(0); 
	} 
	else
	{ 
		rpid = fork(); 
		if (rpid<0) 
		{ 
			// Rchild proc not created 
			perror("Right Child Proc. not created\n"); 
			_exit(-1); 
		} 
		else if(rpid==0) 
		{ 
			mergeSort(a,l+len/2,h); 
			_exit(0); 
		} 
	} 

	int status; 

	// Wait for child processes to finish 
	waitpid(lpid, &status, 0); 
	waitpid(rpid, &status, 0); 

	// Merge the sorted subarrays 
	merge(a, l, l+len/2-1, h); 
} 


// Method to merge sorted subarrays 
void merge(int a[], int l1, int h1, int h2) 
{ 
	// We can directly copy the sorted elements 
	// in the final array, no need for a temporary 
	// sorted array. 
	int count=h2-l1+1; 
	int sorted[count]; 
	int i=l1, k=h1+1, m=0; 
	while (i<=h1 && k<=h2) 
	{ 
		if (a[i]<a[k]) 
			sorted[m++]=a[i++]; 
		else if (a[k]<a[i]) 
			sorted[m++]=a[k++]; 
		else if (a[i]==a[k]) 
		{ 
			sorted[m++]=a[i++]; 
			sorted[m++]=a[k++]; 
		} 
	} 

	while (i<=h1) 
		sorted[m++]=a[i++]; 

	while (k<=h2) 
		sorted[m++]=a[k++]; 

	int arr_count = l1; 
	for (i=0; i<count; i++,l1++) 
		a[l1] = sorted[i]; 
} 

// To check if array is actually sorted or not 
void isSorted(int arr[], int len) 
{ 
	if (len==1) 
	{ 
		printf("Sorting Done Successfully\n"); 
		return; 
	} 

	int i; 
	for (i=1; i<len; i++) 
	{ 
		if (arr[i]<arr[i-1]) 
		{ 
			printf("Sorting Not Done\n"); 
			return; 
		} 
	} 
	printf("Sorting Done Successfully\n"); 
	return; 
} 

// To fill randome values in array for testing 
// purpise 
void fillData(int a[], int len) 
{ 
	// Create random arrays 
	int i; 
	for (i=0; i<len; i++) 
		a[i] = rand(); 
	return; 
} 

// Driver code 
int main() 
{ 
	int shmid; 
	key_t key = IPC_PRIVATE; 
	int *shm_array; 


	// Using fixed size array. We can uncomment 
	// below lines to take size from user 
	int length = 1000; 

	/* printf("Enter No of elements of Array:"); 
	scanf("%d",&length); */

	// Calculate segment length 
	size_t SHM_SIZE = sizeof(int)*length; 

	// Create the segment. 
	if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) 
	{ 
		perror("shmget"); 
		_exit(1); 
	} 

	// Now we attach the segment to our data space. 
	if ((shm_array = shmat(shmid, NULL, 0)) == (int *) -1) 
	{ 
		perror("shmat"); 
		_exit(1); 
	} 

	// Create a random array of given length 
	srand(time(NULL)); 
	fillData(shm_array, length); 

	// Sort the created array 
	mergeSort(shm_array, 0, length-1); 

	// Check if array is sorted or not 
	isSorted(shm_array, length); 

	/* Detach from the shared memory now that we are 
	done using it. */
	if (shmdt(shm_array) == -1) 
	{ 
		perror("shmdt"); 
		_exit(1); 
	} 

	/* Delete the shared memory segment. */
	if (shmctl(shmid, IPC_RMID, NULL) == -1) 
	{ 
		perror("shmctl"); 
		_exit(1); 
	} 

	return 0; 
} 
