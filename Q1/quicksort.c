#include "main.h"

int main(void) {
    srand(time(NULL));
    int n;
    printf("Enter the number of elements: ");
    scanf("%d", &n);

    printf("Enter the elements: ");
    int* arr = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    struct timespec t;
    long double t_start, t_end;
    long double t_normal, t_proc, t_thread;

    // NORMAL QUICKSORT

    // Copy Array
    int* a = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        a[i] = arr[i];
    }
    printf("Running normal quicksort.\n");

    // Start Time
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    t_start = t.tv_sec + t.tv_nsec / (1e9);

    // Run Quicksort
    normal_quick_sort(a, 0, n - 1);

    // End Time
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    t_end = t.tv_sec + t.tv_nsec / (1e9);
    
    // Print Array
    for (int i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    printf("\nThis took %Lf seconds.\n\n", t_end - t_start);
    t_normal = t_end - t_start;

    // MULTIPROCESS QUICKSORT

    // Copy Array
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, sizeof(int) * n, IPC_CREAT | 0666);
    int* b =  (int*)shmat(shm_id, NULL, 0);
    
    for (int i = 0; i < n; i++) {
        b[i] = arr[i];
    }
    printf("Running multiprocessing quicksort.\n");

    // Start Time
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    t_start = t.tv_sec + t.tv_nsec / (1e9);

    // Run Quicksort
    multiproc_quick_sort(b, 0, n - 1);

    // End Time
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    t_end = t.tv_sec + t.tv_nsec / (1e9);

    // Print Array
    for (int i = 0; i < n; i++) {
        printf("%d ", b[i]);
    }
    printf("\nThis took %Lf seconds.\n\n", t_end - t_start);
    t_proc = t_end - t_start;

    // MULTITHREADED QUICKSORT

    printf("Running multithreaded quicksort.\n");
    // Start Time
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    t_start = t.tv_sec + t.tv_nsec / (1e9);

    // Run Quicksort
    args input;
    input.start = 0;
    input.end = n - 1;
    input.arr = arr;
    multithread_quick_sort((void*)&input);

    // End Time
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    t_end = t.tv_sec + t.tv_nsec / (1e9);

    // Print Array
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\nThis took %Lf seconds.\n\n", t_end - t_start);
    t_thread = t_end - t_start;

    printf("For n = %d:\n\nNormal quicksort was:\n", n);
    printf("%Lf times faster than multiprocess quicksort.\n", t_proc / t_normal);
    printf("%Lf times faster than multithread quicksort.\n\n", t_thread / t_normal);
    printf("Multithreaded quicksort was %Lf times faster than multiprocess quicksort.\n", t_proc / t_thread);

    // Detach Shared Memory
    if (shmdt(b) == -1) {
        perror("shmdt");
        exit(1);
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
}