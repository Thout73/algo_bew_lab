#include <stdlib.h>
#include <stdio.h>
#include "functions_cdcl.h"

void trail_init(Trail *trail)
{
    trail->size = 0;
    trail->capacity = 16;
    trail->b = 1;

    trail->data = malloc(trail->capacity * sizeof(Assignment));

    if (trail->data == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }
}

void trail_push(Trail *trail,
                int literal,
                int value,
                CDCL_Clause *reason,
                int decision_lvl)
{
    if (trail->size == trail->capacity)
    {
        trail->capacity *= 2;

        Assignment *tmp =
            realloc(trail->data,
                    trail->capacity * sizeof(Assignment));

        if (tmp == NULL)
        {
            fprintf(stderr, "realloc failed\n");
            exit(EXIT_FAILURE);
        }

        trail->data = tmp;
    }

    trail->data[trail->size].literal = literal;
    trail->data[trail->size].value = value;
    trail->data[trail->size].reason = reason;
    trail->data[trail->size].decision_lvl = decision_lvl;

    trail->size++;
}

void trail_backtrack(Trail *trail, size_t target_size)
{
    if (target_size < trail->size)
        trail->size = target_size;
}

void trail_destroy(Trail *trail)
{
    free(trail->data);

    trail->data = NULL;
    trail->size = 0;
    trail->capacity = 0;
}