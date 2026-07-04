#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions.h"

void copy_arr(int *arr1, int *arr2, int size)
{
    for (int i = 0; i < size; i++)
    {
        arr2[i] = arr1[i];
    }
}

// return 20 if unsat, 10 if satisfied and 0 if not clear
int check_sat(int number_of_variables, int number_of_clauses, Clause *clauses, int *assignment)
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

int pure_literal_elimination(int number_of_variables, int number_of_clauses, Clause *clauses, int *assignment)
{
    int *pure_lst = malloc(number_of_variables * sizeof(int));
    memset(pure_lst, 0, number_of_variables * sizeof(int));

    for (int i = 0; i < number_of_clauses; i++)
    {

        int is_sat = 0;
        for (int j = 0; j < clauses[i].size; j++)
        {
            if (assignment[abs(clauses[i].literals[j]) - 1] == sign(clauses[i].literals[j]))
            {
                is_sat = 1;
                j = clauses[i].size;
            }
        }

        if (is_sat == 0)
        {
            for (int j = 0; j < clauses[i].size; j++)
            {
                if (assignment[abs(clauses[i].literals[j]) - 1] == 0)
                {
                    if (pure_lst[abs(clauses[i].literals[j]) - 1] == 0)
                    {
                        pure_lst[abs(clauses[i].literals[j]) - 1] = sign(clauses[i].literals[j]);
                    }
                    else
                    {
                        if (pure_lst[abs(clauses[i].literals[j]) - 1] != sign(clauses[i].literals[j]))
                        {
                            pure_lst[abs(clauses[i].literals[j]) - 1] = number_of_variables + 1;
                        }
                    }
                }
            }
        }
    }

    int change = 0;
    for (int i = 0; i < number_of_variables; i++)
    {
        if (pure_lst[i] != 0 && pure_lst[i] != number_of_variables + 1)
        {
            assignment[i] = pure_lst[i];
            change = 1;
        }
    }

    free(pure_lst);
    return change;
};

// 0-> no changes, 1 -> changes, 2 -> changes led to sat or unsat
int unitpropagation(int number_of_variables, int number_of_clauses, Clause *clauses, int *assignment)
{
    int done = 1;
    int change = 0;
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
            change = 1;
            assignment[abs(unassigned) - 1] = unassigned / abs(unassigned);
            int is_sat = check_sat(number_of_variables, number_of_clauses, clauses, assignment);
            if (is_sat != 0)
            {
                return 2;
            }
            done = 0;
        }
    } while (done == 0);
    return change;
}

int DPLL_HELP(int number_of_variables, int number_of_clauses, Clause *clauses, int *assignment, int *end_assignment, double *time_unit_prop)
{
    int change = 1;
    while (change == 1)
    {
        change = 0;
        struct timespec time_unit_start, time_unit_end;
        clock_gettime(CLOCK_MONOTONIC, &time_unit_start);
        int change_prop = unitpropagation(number_of_variables, number_of_clauses, clauses, assignment);

        clock_gettime(CLOCK_MONOTONIC, &time_unit_end);

        *time_unit_prop += (time_unit_end.tv_sec - time_unit_start.tv_sec) + (time_unit_end.tv_nsec - time_unit_start.tv_nsec) / 1e9;
        // int change_pure = pure_literal_elimination(number_of_variables, number_of_clauses, clauses, assignment);
        if ((change_prop == 1) && change_prop != 2)
        {
            change = 1;
        }
    }

    int is_sat = check_sat(number_of_variables, number_of_clauses, clauses, assignment);
    if (is_sat != 0)
    {
        if (is_sat == 20)
        {
            return 0;
        }
        else
        {
            copy_arr(assignment, end_assignment, number_of_variables);
            return 1;
        }
    }
    else
    {
        int *unanssigned = malloc(number_of_variables * sizeof(int));
        int number_of_unassigned = 0;

        for (int i = 0; i < number_of_variables; i++)
        {
            unanssigned[i] = 0;
            if (assignment[i] == 0)
            {
                unanssigned[number_of_unassigned] = i + 1;
                number_of_unassigned++;
            }
        }
        if (unanssigned[0] == 0)
        {
            printf("no variable found\n");
            exit(1);
        }
        assignment[unanssigned[0] - 1] = 1;

        int is_sat = DPLL_HELP(number_of_variables, number_of_clauses, clauses, assignment, end_assignment, time_unit_prop);

        if (is_sat == 0)
        {
            for (int i = 0; i < number_of_unassigned; i++)
            {
                if (unanssigned[i] != 0)
                {
                    assignment[unanssigned[i] - 1] = 0;
                }
            }

            assignment[unanssigned[0] - 1] = -1;
            free(unanssigned);
            is_sat = DPLL_HELP(number_of_variables, number_of_clauses, clauses, assignment, end_assignment, time_unit_prop);
            if (is_sat == 0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            free(unanssigned);
            return 1;
        }
    }
}

int DPLL(int number_of_variables, int number_of_clauses, Clause *clauses)
{
    int *assignment = malloc(number_of_variables * sizeof(int));
    memset(assignment, 0, sizeof(int) * number_of_variables);

    int *end_assignment = malloc(number_of_variables * sizeof(int));
    memset(end_assignment, 0, sizeof(int) * number_of_variables);

    double time_unitprop = 0;
    double time_algorithm = 0;

    struct timespec time_algorithm_start, time_algorithm_end;
    clock_gettime(CLOCK_MONOTONIC, &time_algorithm_start);

    int is_sat = DPLL_HELP(number_of_variables, number_of_clauses, clauses, assignment, end_assignment, &time_unitprop);

    clock_gettime(CLOCK_MONOTONIC, &time_algorithm_end);

    time_algorithm = (time_algorithm_end.tv_sec - time_algorithm_start.tv_sec) + (time_algorithm_end.tv_nsec - time_algorithm_start.tv_nsec) / 1e9;

    printf("Zeit gesamt: %f\nZeit unitprop: %f\n", time_algorithm, time_unitprop);
    if (is_sat == 1)
    {
        print_arr(end_assignment, number_of_variables);
    }
    free(assignment);
    free(end_assignment);

    return is_sat;
}