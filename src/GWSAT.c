#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "functions_dpll_etc.h"

typedef struct
{
    int value;

    int *pos_clauses;
    int pos_size;

    int *neg_clauses;
    int neg_size;

    int make_score;
    int break_score;
} Variable;

static int is_satisfied(int number_of_clauses, Clause *clauses)
{
    for (int i = 0; i < number_of_clauses; i++)
    {
        if (clauses[i].true_count == 0)
        {
            return 0;
        }
    }
    return 1;
}
static void update_clause(Clause *clause, Variable *assignment, int delta, int variable)
{
    int old_count = clause->true_count;
    int new_count = old_count + delta;

    clause->true_count = new_count;

    // Clause becomes unsatisfied
    if (new_count == 0)
    {
        for (int j = 0; j < clause->size; j++)
            assignment[abs(clause->literals[j]) - 1].make_score++;

        assignment[variable].break_score--;
    }
    // Clause becomes satisfied with exactly one true literal
    else if (new_count == 1 && delta == -1)
    {
        for (int j = 0; j < clause->size; j++)
        {
            int v = abs(clause->literals[j]) - 1;

            if (assignment[v].value == sign(clause->literals[j]))
                assignment[v].break_score++;
        }
    }
    // Clause becomes satisfied
    else if (new_count == 1 && delta == 1)
    {
        for (int j = 0; j < clause->size; j++)
            assignment[abs(clause->literals[j]) - 1].make_score--;

        assignment[variable].break_score++;
    }
    // Clause goes from 1 to 2 true literals
    else if (new_count == 2)
    {
        for (int j = 0; j < clause->size; j++)
        {
            int v = abs(clause->literals[j]) - 1;

            if (assignment[v].value == sign(clause->literals[j]))
                assignment[v].break_score--;
        }
    }
}

static void flip_variable(int variable, Clause *clauses, Variable *assignment)
{
    int old_value = assignment[variable].value;

    int *pos = assignment[variable].pos_clauses;
    int pos_size = assignment[variable].pos_size;

    int *neg = assignment[variable].neg_clauses;
    int neg_size = assignment[variable].neg_size;

    // Positive clauses: true -> false or false -> true
    int pos_delta = (old_value == 1) ? -1 : 1;

    // Negative clauses: false -> true or true -> false
    int neg_delta = -pos_delta;

    for (int i = 0; i < pos_size; i++)
        update_clause(&clauses[pos[i]], assignment, pos_delta, variable);

    for (int i = 0; i < neg_size; i++)
        update_clause(&clauses[neg[i]], assignment, neg_delta, variable);

    assignment[variable].value = -old_value;
}

int GWSAT(int number_of_variables, int number_of_clauses, Clause *clauses, int max_tries, int max_steps, double propability)
{
    srand(time(NULL));
    for (int iterations = 0; iterations < max_tries; iterations++)
    {

        Variable *assignment = malloc(number_of_variables * sizeof(Variable));

        // random start assignment
        for (int i = 0; i < number_of_variables; i++)
        {
            assignment[i].pos_clauses = malloc(number_of_clauses * sizeof(int));
            assignment[i].neg_clauses = malloc(number_of_clauses * sizeof(int));
            assignment[i].pos_size = 0;
            assignment[i].neg_size = 0;
            assignment[i].make_score = 0;
            assignment[i].break_score = 0;

            int random = (int)rand() % 2;
            if (random == 0)
            {
                assignment[i].value = -1;
            }
            else
            {
                assignment[i].value = 1;
            }
        }

        // SAT für clauses und make und break für variablen berechnen
        for (int i = 0; i < number_of_clauses; i++)
        {
            clauses[i].true_count = 0;
            for (int j = 0; j < clauses[i].size; j++)
            {
                int current_variable = clauses[i].literals[j];
                if (sign(current_variable) == 1)
                {
                    assignment[current_variable - 1].pos_clauses[assignment[current_variable - 1].pos_size] = i;
                    assignment[current_variable - 1].pos_size++;
                }
                else
                {
                    assignment[abs(current_variable) - 1].neg_clauses[assignment[abs(current_variable) - 1].neg_size] = i;
                    assignment[abs(current_variable) - 1].neg_size++;
                }
                if (assignment[abs(current_variable) - 1].value == sign(current_variable))
                {
                    clauses[i].true_count++;
                }
            }
            if (clauses[i].true_count == 0)
            {
                for (int j = 0; j < clauses[i].size; j++)
                {
                    int current_variable = clauses[i].literals[j];
                    assignment[abs(current_variable) - 1].make_score++;
                }
            }
            else
            {
                if (clauses[i].true_count == 1)
                {
                    for (int j = 0; j < clauses[i].size; j++)
                    {
                        int current_variable = clauses[i].literals[j];
                        if (assignment[abs(current_variable) - 1].value == sign(current_variable))
                        {
                            assignment[abs(current_variable) - 1].break_score++;
                        }
                    }
                }
            }
        }
        if (is_satisfied(number_of_clauses, clauses) == 1)
        {

            for (int i = 0; i < number_of_variables; i++)
            {
                free(assignment[i].pos_clauses);
                free(assignment[i].neg_clauses);
            }

            free(assignment);
            return 1;
        }
        for (int step = 0; step < max_steps; step++)
        {
            int variable_to_flip;
            if ((double)rand() / RAND_MAX < propability)
            {
                int done = 0;
                int random_clause = rand() % number_of_clauses;
                int rand_iter = 0;
                int iterator = 0;
                while (done == 0)
                {
                    if (clauses[iterator].true_count == 0)
                    {
                        rand_iter++;
                        if (rand_iter > random_clause)
                        {
                            done = 1;
                            variable_to_flip = abs(clauses[iterator].literals[(int)rand() % clauses[iterator].size]) - 1;
                        }
                    }
                    iterator = (iterator + 1) % number_of_clauses;
                }
            }
            else
            {
                int max_score = 0;
                for (int i = 0; i < number_of_variables; i++)
                {
                    if (assignment[i].make_score - assignment[i].break_score > max_score)
                    {
                        max_score = assignment[i].make_score - assignment[i].break_score;
                        variable_to_flip = i;
                    }
                }
            }
            flip_variable(variable_to_flip, clauses, assignment);

            if (is_satisfied(number_of_clauses, clauses) == 1)
            {

                for (int i = 0; i < number_of_variables; i++)
                {
                    free(assignment[i].pos_clauses);
                    free(assignment[i].neg_clauses);
                }

                free(assignment);
                return 1;
            }
        }
        for (int i = 0; i < number_of_variables; i++)
        {
            free(assignment[i].pos_clauses);
            free(assignment[i].neg_clauses);
        }

        free(assignment);
    }
    return 0;
}