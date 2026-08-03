#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cdcl/functions_cdcl.h"

int main(int argc, char *argv[])
{
    int number_of_variables, number_of_clauses, maximum_length;

    CDCL_Clause *clauses = parse_cdcl("./src/cdcl/debug.cnf", &number_of_variables, &number_of_clauses, &maximum_length);

    int result = CDCL_inkrementell(clauses, number_of_clauses, number_of_variables);

    for (int i = 0; i < number_of_clauses; i++)
    {
        free(clauses[i].literals);
    }

    free(clauses);

    return 0;
}