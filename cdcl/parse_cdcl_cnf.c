#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions_cdcl.h"

CDCL_Clause *parse(char *file_str, int *number_of_variables, int *number_of_clauses, int *maximum_length)
{
    FILE *fp = fopen(file_str, "r");

    if (!fp)
    {
        perror("fopen");
        return (void *)0;
    }

    *number_of_variables = 0;
    *number_of_clauses = 0;
    *maximum_length = 0;

    char line[1024];

    /*
        Header suchen
    */
    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == 'c')
        {
            continue;
        }

        if (line[0] == 'p')
        {
            sscanf(line, "p cnf %d %d", number_of_variables, number_of_clauses);

            break;
        }
    }

    CDCL_Clause *clauses = malloc(*number_of_clauses * sizeof(CDCL_Clause));

    for (int i = 0; i < *number_of_clauses; i++)
    {
        clauses[i].literals = NULL;
        clauses[i].size = 0;
        clauses[i].id = i;
    }

    /*
        Klauseln lesen
    */

    for (int i = 0; i < *number_of_clauses; i++)
    {
        int capacity = 4;

        clauses[i].literals =
            malloc(capacity * sizeof(int));

        int lit;

        while (1)
        {
            if (fscanf(fp, "%d", &lit) != 1)
            {
                printf("Unexpected EOF\n");
                return (void *)0;
            }

            if (lit == 0)
            {
                break;
            }

            if (clauses[i].size >= capacity)
            {
                capacity *= 2;

                clauses[i].literals = realloc(clauses[i].literals, capacity * sizeof(int));
            }

            clauses[i].literals[clauses[i].size] = lit;
            clauses[i].size++;
        }
        if (clauses[i].size > *maximum_length)
        {
            *maximum_length = clauses[i].size;
        }

        clauses[i].watch1 = 0;
        clauses[i].watch2 = clauses[i].size > 1 ? 1 : 0;
    }

    fclose(fp);

    return clauses;
}