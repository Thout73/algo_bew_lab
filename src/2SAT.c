#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

void copy_arr2SAT(int *arr1, int *arr2, int size)
{
    for (int i = 0; i < size; i++)
    {
        arr2[i] = arr1[i];
    }
}

// return 20 if unsat, 10 if satisfied and 0 if not clear
int check_sat2SAT(int number_of_variables, int number_of_clauses, Clause *clauses, int *assignment)
{
    int sat_all = 1;
    for (int i = 0; i < number_of_clauses; i++)
    {
        int unsat = 1;
        int sat = 0;
        for (int j = 0; j < clauses[i].size && sat == 0; j++)
        {
            if (assignment[abs(clauses[i].literals[j]) - 1] == clauses[i].literals[j] / abs(clauses[i].literals[j]))
            {
                sat = 1;
                unsat = 0;
            }
            else
            {
                if (assignment[abs(clauses[i].literals[j]) - 1] == 0)
                {
                    unsat = 0;
                }
            }
        }
        if (unsat == 1)
        {
            return 20;
        }
        if (sat != 1)
        {
            sat_all = 0;
        }
    }
    if (sat_all == 1)
    {
        return 10;
    }
    else
    {
        return 0;
    }
}

void unitpropagation2SAT(int number_of_variables, int number_of_clauses, Clause *clauses, int *assignment)
{
    int done = 1;
    do
    {
        int unassigned = 0;
        for (int i = 0; i < number_of_clauses; i++)
        {
            int number_of_unassigned = 0;
            for (int j = 0; j < clauses[i].size; j++)
            {
                if (assignment[abs(clauses[i].literals[j]) - 1] == 0)
                {
                    number_of_unassigned++;
                    unassigned = clauses[i].literals[j];
                }
                if (assignment[abs(clauses[i].literals[j]) - 1] == clauses[i].literals[j] / abs(clauses[i].literals[j]))
                {
                    number_of_unassigned = number_of_variables;
                }
            }

            if (number_of_unassigned == 1)
            {
                i = number_of_clauses;
            }
            else
            {
                unassigned = number_of_variables + 1;
            }
        }
        done = 1;
        if (unassigned != 0 && unassigned != number_of_variables + 1)
        {
            assignment[abs(unassigned) - 1] = unassigned / abs(unassigned);
            int is_sat = check_sat2SAT(number_of_variables, number_of_clauses, clauses, assignment);
            if (is_sat != 0)
            {
                return;
            }
            done = 0;
        }
    } while (done == 0);
}

int SAT2(int number_of_variables, int number_of_clauses, Clause *clauses)
{
    int *assignment = malloc(number_of_variables * sizeof(int));
    memset(assignment, 0, sizeof(int) * number_of_variables);

    // 2SAT
    int is_sat = 0;
    while (is_sat == 0)
    {
        int *assignment2 = malloc(number_of_variables * sizeof(int));
        copy_arr2SAT(assignment, assignment2, number_of_variables);

        int new_variable = number_of_variables + 1;
        for (int i = 0; i < number_of_variables; i++)
        {
            if (assignment[i] == 0)
            {
                new_variable = i;
                i = number_of_variables;
            }
        }
        if (new_variable == number_of_variables + 1)
        {
            printf("no variable found\n");
            exit(1);
        }
        assignment2[new_variable] = 1;
        unitpropagation2SAT(number_of_variables, number_of_clauses, clauses, assignment2);
        int result = check_sat2SAT(number_of_variables, number_of_clauses, clauses, assignment2);

        if (result == 20)
        {
            assignment[new_variable] = -1;
            unitpropagation2SAT(number_of_variables, number_of_clauses, clauses, assignment);
        }
        else
        {
            assignment[new_variable] = 1;
            unitpropagation2SAT(number_of_variables, number_of_clauses, clauses, assignment);
        }
        free(assignment2);
        is_sat = check_sat2SAT(number_of_variables, number_of_clauses, clauses, assignment);
    }

    free(assignment);
    return is_sat;
}
