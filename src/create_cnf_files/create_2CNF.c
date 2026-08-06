#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// fisher-yates shuffle
void shuffle(int *array, int n)
{
    if (n > 1)
    {
        int i;
        for (i = n - 1; i > 0; i--)
        {
            int j = (rand() % (i + 1));
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void BubbleSort(int a[], int array_size)
{
    int i, j, temp;
    for (i = 0; i < (array_size - 1); ++i)
    {
        for (j = 0; j < array_size - 1 - i; ++j)
        {
            if (a[j] > a[j + 1])
            {
                temp = a[j + 1];
                a[j + 1] = a[j];
                a[j] = temp;
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <variables> <clauses> <output-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int NUMBER_OF_VARIABLES = atoi(argv[1]);
    int NUMBER_OF_CLAUSES = atoi(argv[2]);
    int LENGTH_OF_CLAUSES = 2;
    const char *output_path = argv[3];

    if (NUMBER_OF_VARIABLES < LENGTH_OF_CLAUSES || NUMBER_OF_CLAUSES < 1)
    {
        fprintf(stderr, "Usage: %s <variables> <clauses> <output-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int variable_array[NUMBER_OF_VARIABLES];

    for (int i = 0; i < NUMBER_OF_VARIABLES; i++)
    {
        variable_array[i] = i + 1;
    }

    int cnf_array[NUMBER_OF_CLAUSES][LENGTH_OF_CLAUSES];

    srand(time(NULL));

    for (int k = 0; k < NUMBER_OF_CLAUSES; k++)
    {
        shuffle(variable_array, NUMBER_OF_VARIABLES);

        int temp_arr[LENGTH_OF_CLAUSES];
        for (int i = 0; i < LENGTH_OF_CLAUSES; i++)
        {
            temp_arr[i] = variable_array[i];
        }
        BubbleSort(temp_arr, LENGTH_OF_CLAUSES);

        for (int i = 0; i < LENGTH_OF_CLAUSES; i++)
        {
            if (rand() % 2 == 1)
            {
                cnf_array[k][i] = temp_arr[i];
            }
            else
            {
                cnf_array[k][i] = -temp_arr[i];
            }
        }

        /* Checking for duplicates */
        for (int i = 0; i < k; i++)
        {
            int found = 1;

            for (int h = 0; h < LENGTH_OF_CLAUSES; h++)
            {
                if (cnf_array[k][h] != cnf_array[i][h])
                {
                    found = 0;
                    break;
                }
            }

            if (found)
            {
                k--;
                break;
            }
        }
    }

    /* Write CNF file */
    FILE *fptr = fopen(output_path, "w");

    if (fptr == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }

    fprintf(fptr,
            "p cnf %d %d\n",
            NUMBER_OF_VARIABLES,
            NUMBER_OF_CLAUSES);

    for (int i = 0; i < NUMBER_OF_CLAUSES; i++)
    {
        for (int k = 0; k < LENGTH_OF_CLAUSES; k++)
        {
            fprintf(fptr, "%d ", cnf_array[i][k]);
        }

        fprintf(fptr, "0\n");
    }

    fclose(fptr);

    return EXIT_SUCCESS;
}