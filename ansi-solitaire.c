#include <stdio.h>
#include <time.h>   /* for time() */

#define COUNT 52
#define SCREEN_SIZE 16

/* Minimal bool substitute for pre-C99. */
#define bool int
#define true 1
#define false 0

/*
 * A very simple linear congruential generator
 * for rand()/srand() so we don't depend on <stdlib.h>.
 */
static unsigned long seed_value = 1UL;

void srand(unsigned int seed) {
    seed_value = (unsigned long)seed;
}

int rand(void) {
    /* ANSI C–style LCG from K&R, typical example. */
    seed_value = seed_value * 1103515245UL + 12345UL;
    return (int)((seed_value >> 16) & 0x7FFF);
}

/* A card has a 'value' (0..51) and a 'visible' flag */
typedef struct {
    int value;
    bool visible;
} card;

/* A stack is basically an array of cards plus a size counter */
typedef struct {
    card cards[COUNT];
    int  size;
} stack_t;

/* We have 1 deck, 6 "tableau" stacks, and 4 "final" stacks */
static stack_t deck;
static stack_t stacks[6];
static stack_t final_stacks[4];

/* Forward declarations */
int  get_suit(int t);
int  get_num(int t);
char *display_card(const card *c, char *buf, int buf_size);
char *check_card(stack_t *s, int i, char *buf, int buf_size);
void display_code(int c);
void display(int code);
void push_card(stack_t *s, card c);
card pop_card(stack_t *s);
card *stack_back(stack_t *s);
card *stack_at(stack_t *s, int index);
void stack_insert(stack_t *s, int index, card c);
void stack_erase(stack_t *s, int index);

/* --- Utility functions to get suit and value --- */
int get_suit(int t) {
    return t / 13;
}

int get_num(int t) {
    /* Range 1..13 */
    return (t % 13) + 1;
}

/* --- Display the card value as "suit:num" or "X:XX" if hidden --- */
char *display_card(const card *c, char *buf, int buf_size) {
    (void)buf_size; /* unused, to keep signature the same */

    if (c->visible) {
        /* In old ANSI C, we don't have snprintf, so we use sprintf. */
        sprintf(buf, "%i:%2d", get_suit(c->value), get_num(c->value));
    } else {
        sprintf(buf, "X:XX");
    }
    return buf;
}

/* --- Safely display a card in a stack index if within range --- */
char *check_card(stack_t *s, int i, char *buf, int buf_size) {
    (void)buf_size; /* unused, to keep signature the same */

    if (i < s->size) {
        /* If it's the top card, make it visible */
        if (i == s->size - 1) {
            s->cards[i].visible = true;
        }
        return display_card(&s->cards[i], buf, buf_size);
    }
    /* If out of range, print blanks */
    sprintf(buf, "    ");
    return buf;
}

/* --- Print an error code message --- */
void display_code(int c) {
    /*
       c == 1 => "Not compatible value"
       c == 2 => "Not compatible suit"
       c == 3 => "Not compatible suit (2)"
       c == 4 => "Not compatible value (2)"
    */
    if (c == 1) {
        printf("Not compatible value");
    } else if (c == 2) {
        printf("Not compatible suit");
    } else if (c == 3) {
        printf("Not compatible suit (2)");
    } else if (c == 4) {
        printf("Not compatible value (2)");
    }
}

/* --- Helper to push a card onto a stack --- */
void push_card(stack_t *s, card c) {
    if (s->size < COUNT) {
        s->cards[s->size] = c;
        s->size++;
    }
}

/* --- Helper to pop the top card from a stack --- */
card pop_card(stack_t *s) {
    card c;
    c.value = 0;
    c.visible = false;
    if (s->size > 0) {
        c = s->cards[s->size - 1];
        s->size--;
    }
    return c;
}

/* --- Access the top card of a stack (like vector.back()) --- */
card *stack_back(stack_t *s) {
    if (s->size > 0) {
        return &s->cards[s->size - 1];
    }
    return (card*)0;
}

/* --- Access a card at index --- */
card *stack_at(stack_t *s, int index) {
    if (index >= 0 && index < s->size) {
        return &s->cards[index];
    }
    return (card*)0;
}

/* --- Insert a card at position index (0-based) --- */
void stack_insert(stack_t *s, int index, card c) {
    /* Insert before 'index', shifting the rest */
    if (index < 0) index = 0;
    if (index > s->size) index = s->size;

    if (s->size < COUNT) {
        /* Shift everything to the right from the end */
        int i;
        for (i = s->size; i > index; i--) {
            s->cards[i] = s->cards[i - 1];
        }
        s->cards[index] = c;
        s->size++;
    }
}

/* --- Erase a card at position index --- */
void stack_erase(stack_t *s, int index) {
    int i;
    if (index < 0 || index >= s->size) return;
    for (i = index; i < s->size - 1; i++) {
        s->cards[i] = s->cards[i + 1];
    }
    s->size--;
}

/* --- Display the entire game state --- */
void display(int code) {
    int t, x, y;

    /* Clear screen lines */
    for (t = 0; t < SCREEN_SIZE; t++) {
        printf("\n");
    }
    /* Display code message if needed */
    display_code(code);
    if (code != 0) {
        printf("\n");
    }
    printf("\n");

    /* Output final decks */
    printf("0      1      2      3      \n");
    for (t = 0; t < 4; t++) {
        char buf[16];
        if (final_stacks[t].size > 0) {
            card *topf = stack_back(&final_stacks[t]);
            printf("[%s] ", display_card(topf, buf, sizeof(buf)));
        } else {
            printf("[    ] ");
        }
    }

    /* Make top of the deck visible */
    if (deck.size > 0) {
        card *top = stack_back(&deck);
        if (top) {
            top->visible = true;
            {
                char buf[16];
                printf("/ %s\n\n", display_card(top, buf, sizeof(buf)));
            }
        }
    } else {
        printf("/ [    ]\n\n");
    }

    printf("0     1     2     3     4     5\n");
    for (y = 0; y < 6; y++) {
        for (x = 0; x < 6; x++) {
            char buf[16];
            int offset = 0;
            /* If a stack is tall, show last 6 */
            if (stacks[x].size > 6) {
                offset = stacks[x].size - 6;
            }
            printf("%s  ", check_card(&stacks[x], y + offset, buf, sizeof(buf)));
        }
        printf("\n");
    }
}

/* --- MAIN --- */
int main(void) {
    int t, x;
    int code = 0;

    /* For "random" seed */
    srand((unsigned int)time((time_t*)0));

    /* Initialize deck */
    deck.size = 0;
    for (t = 0; t < COUNT; t++) {
        card c;
        c.value   = t;
        c.visible = false;
        push_card(&deck, c);
    }

    /* Shuffle */
    for (t = 0; t < COUNT; t++) {
        int move = rand() % COUNT;
        /* swap deck.cards[t] and deck.cards[move] */
        card temp       = deck.cards[t];
        deck.cards[t]   = deck.cards[move];
        deck.cards[move]= temp;
    }

    /* Initialize stacks */
    for (x = 0; x < 6; x++) {
        stacks[x].size = 0;
    }
    for (x = 0; x < 4; x++) {
        final_stacks[x].size = 0;
    }

    /* Deal to the 6 stacks */
    for (x = 0; x < 6; x++) {
        for (t = 0; t < x + 1; t++) {
            card c = pop_card(&deck);
            push_card(&stacks[x], c);
        }
    }

    /* Main loop */
    while (1) {
        /* Show everything */
        display(code);
        code = 0;

        printf("?");
        fflush(stdout);

        {
            char com;
            if (scanf(" %c", &com) != 1) {
                /* If user input fails, just continue */
                continue;
            }

            if (com == 'm') {
                int from, to;
                printf(">>");
                fflush(stdout);
                if (scanf(" %d %d", &from, &to) != 2) {
                    continue;
                }
                /* Move top card from "from" stack to "to" stack if valid */
                if (stacks[from].size > 0 && stacks[to].size > 0) {
                    card *c1 = stack_back(&stacks[from]);
                    card *c2 = stack_back(&stacks[to]);
                    if (c1 && c2) {
                        if (((get_suit(c1->value) + 1) % 2) == (get_suit(c2->value) % 2)) {
                            if (get_num(c2->value) - get_num(c1->value) == 1) {
                                card moved = pop_card(&stacks[from]);
                                moved.visible = true;
                                push_card(&stacks[to], moved);
                            } else {
                                code = 1;
                            }
                        } else {
                            code = 2;
                        }
                    }
                }
            }
            else if (com == 'M') {
                int from, to;
                printf(">>");
                fflush(stdout);
                if (scanf(" %d %d", &from, &to) != 2) {
                    continue;
                }
                printf("\n");

                /* Display indexes in 'from' stack */
                for (t = 0; t < stacks[from].size; t++) {
                    printf("%d    ", t);
                }
                printf("\n");
                /* Display cards in 'from' stack */
                for (t = 0; t < stacks[from].size; t++) {
                    char buf[16];
                    display_card(&stacks[from].cards[t], buf, sizeof(buf));
                    printf("%s ", buf);
                }
                printf("\n\n");

                /* Display indexes in 'to' stack */
                for (t = 0; t < stacks[to].size; t++) {
                    printf("%d    ", t);
                }
                printf("\n");
                /* Display cards in 'to' stack */
                for (t = 0; t < stacks[to].size; t++) {
                    char buf[16];
                    display_card(&stacks[to].cards[t], buf, sizeof(buf));
                    printf("%s ", buf);
                }
                printf("\n");

                {
                    int loc1, loc2;
                    printf(">>");
                    fflush(stdout);
                    if (scanf(" %d %d", &loc1, &loc2) != 2) {
                        continue;
                    }

                    if (loc1 >= 0 && loc1 < stacks[from].size &&
                        loc2 >= 0 && loc2 < stacks[to].size) {
                        card *pc1 = &stacks[from].cards[loc1];
                        card *pc2 = &stacks[to].cards[loc2];
                        if (((get_suit(pc1->value) + 1) % 2) == (get_suit(pc2->value) % 2)) {
                            if (get_num(pc2->value) - get_num(pc1->value) == 1) {
                                /* Move from [loc1..end] to position loc2 in the 'to' stack */
                                while (stacks[from].size > loc1) {
                                    card f = stacks[from].cards[loc1];
                                    stack_insert(&stacks[to], loc2, f);
                                    stack_erase(&stacks[from], loc1);
                                }
                            }
                        }
                    }
                }
            }
            else if (com == 'n') {
                /* Rotate deck (top card goes to bottom) */
                if (deck.size > 0) {
                    card v = pop_card(&deck);
                    /* Insert at the front (deck.begin()) */
                    if (deck.size < COUNT) {
                        int i;
                        for (i = deck.size; i > 0; i--) {
                            deck.cards[i] = deck.cards[i - 1];
                        }
                        deck.cards[0] = v;
                        deck.size++;
                    }
                }
            }
            else if (com == 'p') {
                int to;
                printf(">");
                fflush(stdout);
                if (scanf(" %d", &to) != 1) {
                    continue;
                }
                if (deck.size > 0 && stacks[to].size > 0) {
                    card *pc1 = stack_back(&deck);
                    card *pc2 = stack_back(&stacks[to]);
                    if (pc1 && pc2) {
                        if (((get_suit(pc1->value) + 1) % 2) == (get_suit(pc2->value) % 2)) {
                            if (get_num(pc2->value) - get_num(pc1->value) == 1) {
                                card moved = pop_card(&deck);
                                moved.visible = true;
                                push_card(&stacks[to], moved);
                                printf("moved!");
                            } else {
                                code = 4;
                            }
                        } else {
                            code = 3;
                        }
                    }
                }
            }
            else if (com == 'P') {
                int from, to;
                printf(">>");
                fflush(stdout);
                if (scanf(" %d %d", &from, &to) != 2) {
                    continue;
                }
                if (stacks[from].size > 0) {
                    card *c1 = stack_back(&stacks[from]);
                    if (c1) {
                        /* If final stack empty, accept only A(1) */
                        if (final_stacks[to].size == 0) {
                            if (get_num(c1->value) == 1) {
                                card moved = pop_card(&stacks[from]);
                                push_card(&final_stacks[to], moved);
                            }
                        } else {
                            card *c2 = stack_back(&final_stacks[to]);
                            if (c2) {
                                if (get_num(c1->value) == get_num(c2->value) + 1 &&
                                    get_suit(c1->value) == get_suit(c2->value)) {
                                    card moved = pop_card(&stacks[from]);
                                    push_card(&final_stacks[to], moved);
                                }
                            }
                        }
                    }
                }
            }
            else if (com == 'Q') {
                /* Move from deck to final stack */
                int to;
                printf(">");
                fflush(stdout);
                if (scanf(" %d", &to) != 1) {
                    continue;
                }
                if (deck.size > 0) {
                    card *c1 = stack_back(&deck);
                    if (c1) {
                        if (final_stacks[to].size == 0) {
                            if (get_num(c1->value) == 1) {
                                card moved = pop_card(&deck);
                                push_card(&final_stacks[to], moved);
                            }
                        } else {
                            card *c2 = stack_back(&final_stacks[to]);
                            if (c2) {
                                if ((get_num(c1->value) == get_num(c2->value) + 1) &&
                                    (get_suit(c1->value) == get_suit(c2->value))) {
                                    card moved = pop_card(&deck);
                                    push_card(&final_stacks[to], moved);
                                }
                            }
                        }
                    }
                }
            }
            else if (com == '\n') {
                /* Ignore blank input */
            }
        }
    }

    return 0;
}
