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
        printf("Wrong number of inputs.");
        return 0;
    }
    int NUMBER_OF_VARIABLES = atoi(argv[1]);
    int NUMBER_OF_CLAUSES = atoi(argv[2]);
    int LENGTH_OF_CLAUSES = 2;
    const char *output_path = argv[3];

    if (LENGTH_OF_CLAUSES > NUMBER_OF_VARIABLES)
    {
        printf("Wrong length of clauses or wrong number of variables");
        return 0;
    }

    int variable_array[NUMBER_OF_VARIABLES];
    for (int i = 0; i < NUMBER_OF_VARIABLES; i++)
    {
        variable_array[i] = i + 1;
    }

    int cnf_array[NUMBER_OF_CLAUSES][LENGTH_OF_CLAUSES];

    char cnf_start_string[11] = "p cnf ";

    char str_number_of_variables[20];
    sprintf(str_number_of_variables, "%d", NUMBER_OF_VARIABLES);
    char str_number_of_clauses[20];
    sprintf(str_number_of_clauses, "%d", NUMBER_OF_CLAUSES);

    strcat(cnf_start_string, str_number_of_variables);
    strcat(cnf_start_string, " ");
    strcat(cnf_start_string, str_number_of_clauses);
    strcat(cnf_start_string, "\n");

    srand(time(NULL));

    for (int k = 0; k < NUMBER_OF_CLAUSES; k++)
    {

        // generating random variables
        shuffle(variable_array, NUMBER_OF_VARIABLES);

        // sorting the variables
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
                cnf_array[k][i] = -1 * temp_arr[i];
            }
        }

        // checking for duplicates if found --> repeat
        for (int i = 0; i < k; i++)
        {
            int found = 1;
            for (int h = 0; h < LENGTH_OF_CLAUSES; h++)
            {
                if (cnf_array[k][h] != cnf_array[i][h])
                {
                    found = 0;
                }
            }
            if (found == 1)
            {
                k--;
            }
        }
    }

    // Convert Array to String
    FILE *fptr;
    fptr = fopen(output_path, "w");
    fprintf(fptr, "%s", cnf_start_string);
    char help_str[3 * LENGTH_OF_CLAUSES];

    for (int i = 0; i < NUMBER_OF_CLAUSES; i++)
    {
        memset(help_str, 0, strlen(help_str));
        for (int k = 0; k < LENGTH_OF_CLAUSES; k++)
        {
            char variable_string[3];
            sprintf(variable_string, "%d ", cnf_array[i][k]);
            strcat(help_str, variable_string);
        }
        fprintf(fptr, "%s0\n", help_str);
    }

    fclose(fptr);
    return 1;
}