#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>

using namespace std;

constexpr int COUNT = 52;
constexpr int SCREEN_SIZE = 16;

struct card
{
    int value;
    bool visible;
};

vector<card> deck;
vector<card> stacks[6];
vector<card> final[4];

int get_suit(int t)
{
    return t / 13;
}

int get_num(int t)
{
    return (t % 13) + 1;
}

// Display card value
string display_card(card c)
{
    if (c.visible)
    {
        int t = c.value;

        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%i:%2d", get_suit(t), get_num(t));
        string result = buffer;
        return result;
    }

    return "X:XX";
}

string check_card(vector<card> &stack, int i)
{
    if (i < stack.size())
    {
        if (i == stack.size() - 1)
        {
            stack.at(i).visible = true;
        }

        return display_card(stack.at(i));
    }
    else
    {
        return "    ";
    }
}

void display()
{
    // Four solution decks
    cout << "0      1      2      3      " << endl;
    for (int t = 0; t < 4; t++)
    {
        if (final[t].size() > 0)
        {
            cout << "[" << display_card(final[t].back()) << "] ";
        }
        else
        {
            cout << "[    ] ";
        }
    }

    deck.back().visible = true;
    cout << "/ " << display_card(deck.back()) << endl
         << endl;
    cout << "0     1     2     3     4     5" << endl;

    for (int y = 0; y < 6; y++)
    {
        for (int x = 0; x < 11; x++)
        {
            int offset = 0;
            if (stacks[x].size() > 6)
            {
                offset = stacks[x].size() - 6;
            }

            cout << check_card(stacks[x], y + offset) << "  ";
        }
        cout << endl;
    }
}

void display_code(int c)
{
    if (c == 1)
    {
        printf("Not compatible value");
    }
    else if (c == 2)
    {
        printf("Not compatible suit");
    }
    else if (c == 4)
    {
        printf("Not compatible value (2)");
    }
    else if (c == 3)
    {
        printf("Not compatible suit (2)");
    }
}

int main()
{
    // Initial deck
    for (int t = 0; t < COUNT; t++)
    {
        card c = {t, false};
        deck.push_back(c);
    }

    // Shuffle
    for (int t = 0; t < COUNT; t++)
    {
        int move = rand() % COUNT;
        card c = deck.at(move);
        deck.erase(deck.begin() + move);
        deck.push_back(c);
    }

    for (int x = 0; x < 6; x++)
    {
        for (int t = 0; t < x + 1; t++)
        {
            stacks[x].push_back(deck.back());
            deck.pop_back();
        }
    }

    int code = 0;

    while (true)
    {
        for (int t = 0; t < SCREEN_SIZE; t++)
        {
            cout << endl;
        }

        display_code(code);
        code = 0;

        display();
        char com;
        int from, to;
        printf("?");
        scanf("%c", &com);

        if (com == 'm')
        {
            printf(">>");
            scanf(" %i %i", &from, &to);

            int c1 = stacks[from].back().value;
            int c2 = stacks[to].back().value;

            if ((get_suit(c1) + 1) % 2 == get_suit(c2) % 2)
            {
                if (get_num(c2) - get_num(c1) == 1)
                {
                    stacks[from].pop_back();
                    stacks[to].push_back({c1, true});
                }
                else
                {
                    code = 1;
                }
            }
            else
            {
                code = 2;
            }
        }
        else if (com == 'M')
        {
            printf(">>");
            scanf(" %i %i", &from, &to);

            cout << endl;

            for (int t = 0; t < stacks[from].size(); t++)
            {
                cout << t << "    ";
            }
            cout << endl;
            for (int t = 0; t < stacks[from].size(); t++)
            {
                cout << display_card(stacks[from].at(t)) << " ";
            }
            cout << endl;

            for (int t = 0; t < stacks[to].size(); t++)
            {
                cout << t << "    ";
            }
            cout << endl;
            for (int t = 0; t < stacks[to].size(); t++)
            {
                cout << display_card(stacks[to].at(t)) << " ";
            }
            cout << endl;

            int loc1, loc2;
            printf(">>");
            scanf(" %i %i", &loc1, &loc2);

            int c1 = stacks[from].at(loc1).value;
            int c2 = stacks[to].at(loc2).value;

            if ((get_suit(c1) + 1) % 2 == get_suit(c2) % 2)
            {
                if (get_num(c2) - get_num(c1) == 1)
                {
                    int count = stacks[from].size() - loc1;
                    while (stacks[from].size() > loc1)
                    {
                        card f = stacks[from].at(loc1);
                        stacks[to].insert(stacks[to].begin() + loc2, f);
                        stacks[from].erase(stacks[from].begin() + loc1);
                    }
                }
            }
        }

        else if (com == 'n')
        {
            card v = deck.back();
            deck.pop_back();
            deck.insert(deck.begin(), v);
        }
        else if (com == 'p')
        {
            printf(">");
            scanf(" %i", &to);
            int c1 = (deck.back().value);
            int c2 = (stacks[to].back().value);

            if ((get_suit(c1) + 1) % 2 == get_suit(c2) % 2)
            {
                if (get_num(c2) - get_num(c1) == 1)
                {
                    deck.pop_back();
                    stacks[to].push_back({c1, true});
                    printf("moved!");
                }
                else
                {
                    code = 4;
                }
            }
            else
            {
                code = 3;
            }
        }
        else if (com == 'P')
        {
            printf(">>");
            scanf(" %i %i", &from, &to);

            int c1 = stacks[from].back().value;

            if (final[to].size() == 0)
            {
                if (get_num(c1) == 1)
                {
                    card c = stacks[from].back();
                    final[to].push_back(c);
                    stacks[from].pop_back();
                }
            }
            else
            {
                int c2 = final[to].back().value;

                if (get_num(c1) == get_num(c2) + 1 && get_suit(c1) == get_suit(c2))
                {
                    card c = stacks[from].back();
                    final[to].push_back(c);
                    stacks[from].pop_back();
                }
            }
        }
        else if (com == 'Q')
        {
            printf(">");
            scanf(" %i", &to);

            int c1 = deck.back().value;

            if (final[to].size() == 0)
            {
                if (get_num(c1) == 1)
                {
                    card c = deck.back();
                    final[to].push_back(c);
                    deck.pop_back();
                }
            }
            else
            {
                int c2 = final[to].back().value;

                if (get_num(c1) == get_num(c2) + 1 && get_suit(c1) == get_suit(c2))
                {
                    card c = deck.back();
                    final[to].push_back(c);
                    deck.pop_back();
                }
            }
        }
        else if (com == '\n')
        {
        }
    }

    return 0;
}