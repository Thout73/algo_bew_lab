#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions.h"

typedef struct
{
    int value;

    int *pos_clauses;
    int pos_size;

    int *neg_clauses;
    int neg_size;
} Variable;

int set_variable(Clause *clauses, Variable *assignment, int variable, int value)
{
    if (value == 1)
    {
        assignment[variable - 1].value = 1;
        for (int i = 0; i < assignment[variable - 1].pos_size; i++)
        {
            Clause *current_clause = &clauses[assignment[variable - 1].pos_clauses[i]];
            current_clause->number_of_unassigned--;
            current_clause->true_count++;
            current_clause->sat = 1;
        }
        for (int i = 0; i < assignment[variable - 1].neg_size; i++)
        {
            Clause *current_clause = &clauses[assignment[variable - 1].neg_clauses[i]];

            current_clause->number_of_unassigned--;

            if (current_clause->true_count == 0)
            {
                if (current_clause->number_of_unassigned == 0)
                {
                    current_clause->sat = 0;
                    return 0;
                }
                else
                {
                    current_clause->sat = 2;
                }
            }
            else
            {
                current_clause->sat = 1;
            }
        }
    }
    else
    {
        assignment[variable - 1].value = -1;
        for (int i = 0; i < assignment[variable - 1].pos_size; i++)
        {
            Clause *current_clause = &clauses[assignment[variable - 1].pos_clauses[i]];

            current_clause->number_of_unassigned--;

            if (current_clause->true_count == 0)
            {
                if (current_clause->number_of_unassigned == 0)
                {
                    current_clause->sat = 0;
                    return 0;
                }
                else
                {
                    current_clause->sat = 2;
                }
            }
            else
            {
                current_clause->sat = 1;
            }
        }
        for (int i = 0; i < assignment[variable - 1].neg_size; i++)
        {
            Clause *current_clause = &clauses[assignment[variable - 1].neg_clauses[i]];

            current_clause->number_of_unassigned--;
            current_clause->true_count++;
            current_clause->sat = 1;
        }
    }
    return 1;
}

// 0 -> no change, 1 -> change, 2 -> UNSAT
int pure_literal_elimination_neu(int number_of_variables, int number_of_clauses, Clause *clauses, Variable *assignment)
{
    int *pure_status = malloc(number_of_variables * sizeof(int));
    memset(pure_status, 0, number_of_variables * sizeof(int));

    for (int i = 0; i < number_of_clauses; i++)
    {
        if (clauses[i].sat == 1)
        {
            continue;
        }

        for (int j = 0; j < clauses[i].size; j++)
        {
            int literal = clauses[i].literals[j];
            int var_idx = abs(literal) - 1;

            if (assignment[var_idx].value != 0)
            {
                continue;
            }

            int current_sign = sign(literal);

            if (pure_status[var_idx] == 0)
            {
                pure_status[var_idx] = current_sign;
            }
            else if (pure_status[var_idx] != 99 && pure_status[var_idx] != current_sign)
            {
                pure_status[var_idx] = 99;
            }
        }
    }

    int change = 0;
    for (int i = 0; i < number_of_variables; i++)
    {
        if (pure_status[i] == 1 || pure_status[i] == -1)
        {
            change = 1;
            int set_sat = set_variable(clauses, assignment, i + 1, pure_status[i]);

            if (set_sat == 0)
            {
                free(pure_status);
                return 2;
            }
        }
    }

    free(pure_status);
    return change;
}

// 0-> no changes, 1 -> changes, 2 -> unsat
int unitpropagation_neu(int number_of_variables, int number_of_clauses, Clause *clauses, Variable *assignment)
{
    int done = 1;
    int change = 0;
    do
    {
        int unassigned_var = 0;
        int found_var = 0;
        for (int i = 0; i < number_of_clauses && found_var != 1; i++)
        {
            if (clauses[i].number_of_unassigned == 1 && clauses[i].sat == 2)
            {
                for (int j = 0; j < clauses[i].size && found_var != 1; j++)
                {
                    if (assignment[abs(clauses[i].literals[j]) - 1].value == 0)
                    {
                        unassigned_var = clauses[i].literals[j];
                        found_var = 1;
                    }
                }
            }
        }
        done = 1;
        if (unassigned_var != 0)
        {
            change = 1;
            int set_sat = set_variable(clauses, assignment, abs(unassigned_var), sign(unassigned_var));

            if (set_sat == 0)
            {
                return 2;
            }
            done = 0;
        }
    } while (done == 0);
    return change;
}

int DPLL_HELP_neu(int number_of_variables, int number_of_clauses, Clause *clauses, Variable *assignment, double *time_unit_prop)
{
    int simplify_again;
    do
    {
        simplify_again = 0;

        // 1. Unit Propagation ausführen
        struct timespec time_unit_start, time_unit_end;
        clock_gettime(CLOCK_MONOTONIC, &time_unit_start);
        int prop_res = unitpropagation_neu(number_of_variables, number_of_clauses, clauses, assignment);
        clock_gettime(CLOCK_MONOTONIC, &time_unit_end);
        *time_unit_prop += (time_unit_end.tv_sec - time_unit_start.tv_sec) + (time_unit_end.tv_nsec - time_unit_start.tv_nsec) / 1e9;

        if (prop_res == 2)
            return 0; // UNSAT
        if (prop_res == 1)
            simplify_again = 1;

        // 2. Pure Literal Elimination ausführen
        int pure_res = pure_literal_elimination_neu(number_of_variables, number_of_clauses, clauses, assignment);

        if (pure_res == 2)
            return 0; // UNSAT
        if (pure_res == 1)
            simplify_again = 1;

    } while (simplify_again); // Wiederholen, solange sich noch etwas vereinfachen lässt

    // find unassigned vars to reset them later
    int *copy_values = malloc(number_of_variables * sizeof(int));
    int branch_var = -1;
    for (int i = 0; i < number_of_variables; i++)
    {
        copy_values[i] = assignment[i].value;
        if (branch_var == -1 && assignment[i].value == 0)
        {
            branch_var = i + 1;
        }
    }

    if (branch_var == -1)
    {
        free(copy_values);
        return 1;
    }

    // copy clauses to reset them later
    Clause *copy_clauses = malloc(number_of_clauses * sizeof(Clause));
    for (int i = 0; i < number_of_clauses; i++)
    {
        copy_clauses[i].sat = clauses[i].sat;
        copy_clauses[i].true_count = clauses[i].true_count;
        copy_clauses[i].number_of_unassigned = clauses[i].number_of_unassigned;
    }

    int set_sat = set_variable(clauses, assignment, branch_var, 1);
    if (set_sat != 0)
    {
        int is_sat = DPLL_HELP_neu(number_of_variables, number_of_clauses, clauses, assignment, time_unit_prop);
        if (is_sat == 1)
        {
            free(copy_clauses);
            free(copy_values);
            return 1;
        }
    }

    // reset vars
    for (int i = 0; i < number_of_variables; i++)
    {
        assignment[i].value = copy_values[i];
    }

    // reset clauses
    for (int i = 0; i < number_of_clauses; i++)
    {
        clauses[i].sat = copy_clauses[i].sat;
        clauses[i].number_of_unassigned = copy_clauses[i].number_of_unassigned;
        clauses[i].true_count = copy_clauses[i].true_count;
    }

    set_sat = set_variable(clauses, assignment, branch_var, -1);

    free(copy_values);
    free(copy_clauses);

    if (set_sat == 0)
    {
        return 0;
    }

    return DPLL_HELP_neu(number_of_variables, number_of_clauses, clauses, assignment, time_unit_prop);
}

int DPLL_neu(int number_of_variables, int number_of_clauses, Clause *clauses)
{
    // init assignment
    Variable *assignment = malloc(number_of_variables * sizeof(Variable));

    for (int i = 0; i < number_of_variables; i++)
    {
        assignment[i].value = 0;
        assignment[i].neg_size = 0;
        assignment[i].pos_size = 0;
        assignment[i].pos_clauses = malloc(number_of_clauses * (sizeof(int)));
        assignment[i].neg_clauses = malloc(number_of_clauses * (sizeof(int)));
    }

    for (int i = 0; i < number_of_clauses; i++)
    {
        clauses[i].number_of_unassigned = clauses[i].size;
        for (int j = 0; j < clauses[i].size; j++)
        {
            int current_var = clauses[i].literals[j];
            if (sign(current_var) == 1)
            {
                assignment[abs(current_var) - 1].pos_clauses[assignment[abs(current_var) - 1].pos_size] = clauses[i].id;
                assignment[abs(current_var) - 1].pos_size++;
            }
            else
            {
                assignment[abs(current_var) - 1].neg_clauses[assignment[abs(current_var) - 1].neg_size] = clauses[i].id;
                assignment[abs(current_var) - 1].neg_size++;
            }
        }
    }

    double time_unitprop = 0;
    double time_algorithm = 0;

    struct timespec time_algorithm_start, time_algorithm_end;
    clock_gettime(CLOCK_MONOTONIC, &time_algorithm_start);

    int is_sat = DPLL_HELP_neu(number_of_variables, number_of_clauses, clauses, assignment, &time_unitprop);

    clock_gettime(CLOCK_MONOTONIC, &time_algorithm_end);

    time_algorithm = (time_algorithm_end.tv_sec - time_algorithm_start.tv_sec) + (time_algorithm_end.tv_nsec - time_algorithm_start.tv_nsec) / 1e9;

    printf("Zeit gesamt: %f\nZeit unitprop: %f\n", time_algorithm, time_unitprop);

    for (int i = 0; i < number_of_variables; i++)
    {
        free(assignment[i].pos_clauses);
        free(assignment[i].neg_clauses);
    }
    free(assignment);

    if (is_sat == 1)
    {
        return 10;
    }
    else
    {
        return 0;
    }
}