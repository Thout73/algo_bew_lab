#include <stdio.h>
#include <stdlib.h>
#include "functions_dpll_etc.h"

int sign(int x)
{
    if (x < 0)
        return -1;
    return 1;
}

void print_assignment(Variable_GWSAT *assignment, int number_of_variables)
{
    for (int i = 0; i < number_of_variables; i++)
    {
        printf("%d", assignment[i].value * (i + 1));

        if (i < number_of_variables - 1)
            printf(" ");
    }

    printf("\n");
}

int main(void)
{
    int number_of_variables;
    int number_of_clauses;
    int maximum_length;

    Clause *clauses = parse(
        "cnf_files/random6.cnf",
        &number_of_variables,
        &number_of_clauses,
        &maximum_length);

    if (clauses == NULL)
    {
        printf("Fehler beim Parsen\n");
        return 1;
    }

    // init assignment
    Variable_GWSAT *assignment = malloc(number_of_variables * sizeof(Variable_GWSAT));

    for (int i = 0; i < number_of_variables; i++)
    {
        assignment[i].pos_clauses = malloc(number_of_clauses * sizeof(int));
        assignment[i].neg_clauses = malloc(number_of_clauses * sizeof(int));
    }

    int result = GWSAT(number_of_variables, number_of_clauses, clauses, 100, 1000, 0.7, assignment);

    if (result == 0)
    {
        printf("s UNKNWON\n");
    }
    else
    {
        printf("s SATISFIABLE\n");
        printf("v ");
        print_assignment(assignment, number_of_variables);
    }

    for (int i = 0; i < number_of_clauses; i++)
        free(clauses[i].literals);

    free(clauses);

    for (int i = 0; i < number_of_variables; i++)
    {
        free(assignment[i].pos_clauses);
        free(assignment[i].neg_clauses);
    }
    free(assignment);

    return 0;
}