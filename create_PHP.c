#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

int create_PHP(int n)
{

    for (int num_pigeons = 1; num_pigeons <= n; num_pigeons++)
    {
        char file_str[1024];
        sprintf(file_str, "cnf_files/php%d.cnf", num_pigeons);
        FILE *f = fopen(file_str, "w");

        if (!f)
        {
            perror("fopen");
            return 1;
        }

        int num_var = num_pigeons * (num_pigeons + 1);
        int num_clauses = num_pigeons + 1 + ((num_pigeons + 1) * num_pigeons / 2) * num_pigeons;

        fprintf(f,
                "p cnf %d %d\n",
                num_var,
                num_clauses);

        for (int i = 1; i <= num_pigeons + 1; i++)
        {
            for (int j = 1; j <= num_pigeons; j++)
            {
                fprintf(f, "%d ", (i - 1) * num_pigeons + j);
            }
            fprintf(f, "0\n");
        }

        for (int i1 = 1; i1 <= num_pigeons + 1; i1++)
        {
            for (int i2 = i1 + 1; i2 <= num_pigeons + 1; i2++)
            {
                for (int j = 1; j <= num_pigeons; j++)
                {
                    fprintf(f, "-%d -%d 0\n", (i1 - 1) * num_pigeons + j, (i2 - 1) * num_pigeons + j);
                }
            }
        }
        fclose(f);
    }
    return 0;
}

int main(int argc, char **argv)
{
    create_PHP(atoi(argv[1]));
    return 0;
}