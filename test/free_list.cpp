#include <bits/stdc++.h>
#define endl '\n'
#define wp ' '
#define forn for (int i = 0; i < n; i++)
using namespace std;

typedef struct free_list_node
{
    int start;
    int size;
    free_list_node *next;
} free_list_node;

typedef struct free_list
{
    free_list_node *head;
    int size;
} free_list;

free_list *create(int size)
{
    free_list_node *tr = (free_list_node *)malloc(sizeof(free_list_node));
    tr->start = 0;
    tr->size = size;
    tr->next = NULL;
    free_list *node = (free_list *)malloc(sizeof(free_list));
    node->head = tr;
    node->size = size;
    return node;
}

int can_insert(free_list *l, int size)
{
    free_list_node *st = l->head;
    while (st != NULL)
    {
        if (st->size >= size)
        {
            return 1;
        }
        st = st->next;
    }
    return 0;
}

void recover_space(free_list *l, int start, int size)
{
    int done = 0;
    free_list_node *st = l->head;
    while (st != NULL)
    {
        if (st->start == start + size)
        {
            st->start -= size;
            st->size += size;
            done = 1;
            break;
        }
        if (st->start + st->size == start)
        {
            st->size += size;
            done = 1;
            break;
        }
        st = st->next;
    }
    if (!done)
    {
        free_list_node *neww = (free_list_node *)malloc(sizeof(free_list_node));
        neww->start = start;
        neww->size = size;
        neww->next = l->head;
        l->head = neww;
    }
    return;
}

int request_space(free_list *l, int size)
{
    free_list_node *prev = NULL;
    free_list_node *st = l->head;
    while (st != NULL)
    {
        if (st->size >= size)
        {
            if (st->size > size)
            {
                st->start += size;
                st->size -= size;
                return st->start - size;
            }
            else
            {
                int to_return = st->start;
                if (prev != NULL)
                {
                    prev->next = st->next;
                }
                else
                {
                    l->head = st->next;
                }
                free(st);
                return to_return;
            }
        }
        prev = st;
        st = st->next;
    }
    return -1;
}

void print_fl(free_list *l)
{
    cout << "START" << endl;
    free_list_node *st = l->head;
    while (st != NULL)
    {
        cout << st->start << wp << st->size << endl;
        st = st->next;
    }
    cout << "END" << endl;
}

int is_occupied(free_list *l, int pos)
{
    if (pos >= l->size)
    {
        return 0;
    }
    int found = 0;
    free_list_node *st = l->head;
    while (st != NULL)
    {
        if (pos >= st->start && pos < st->start + st->size)
        {
            found = 1;
            break;
        }
        st = st->next;
    }
    return !found;
}

void Solve()
{
    int q, size;
    cin >> q >> size;
    free_list *l = create(500);
    for (int i = 0; i < q; i++)
    {
        int num;
        char op;
        cin >> op >> num;
        if (op == 'i')
        {
            if (can_insert(l, num))
            {
                request_space(l, num);
            }
        }
        else if (op == 'r')
        {
            int k;
            cin >> k;
            recover_space(l, num, k);
        }
        else if (op == 'o')
        {
            cout << (is_occupied(l, num) == 1 ? 1 : 0) << endl;
        }
    }
    return;
}

int main()
{
    Solve();
}