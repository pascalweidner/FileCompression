#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "huffmanTree.h"

typedef struct node node;

struct node
{
    node *next;
    t_node_t *elem;
};

typedef struct
{
    node *head;
    int size;
    node *last;
} h_list;

h_list *createHList()
{
    h_list *list = malloc(sizeof(h_list));
    list->size = 0;
    list->head = NULL;
    list->last = list->head;
}

void destroyNode(node *nd);

void destroyHList(h_list *lst)
{
    if (lst == NULL)
    {
        perror("Error message");
        exit(-errno);
    }

    if (lst->head != NULL)
    {
        destroyNode(lst->head);
    }
    free(lst);
}

void destroyNode(node *nd)
{
    if (nd->next != NULL)
    {
        destroyNode(nd->next);
    }
    free(nd->elem);
    free(nd);
}

void add_h(h_list *lst, t_node_t *elem)
{
    node *new = malloc(sizeof(node));
    new->next = NULL;
    new->elem = elem;

    if (lst->last == NULL)
    {
        lst->head = new;
        lst->last = lst->head;
        lst->size++;
        return;
    }
    lst->last->next = new;
    lst->last = new;
    lst->size++;
}

void delete_h(h_list *lst, int pos)
{

    if (pos >= lst->size)
    {
        printf("Error: Index out of range");
        exit(-1);
    }

    if (pos == 0)
    {
        node *temp = lst->head;
        lst->head = lst->head->next;
        free(temp);

        if (lst->head == NULL)
        {
            lst->last = NULL;
        }
        lst->size--;
        return;
    }

    node *curr = lst->head;
    for (int i = 0; i < pos; i++)
    {
        curr = curr->next;
    }

    node *temp = curr;

    lst->size--;
    curr->next = temp->next;
    free(temp);
}

size_t list_len(h_list *lst)
{
    return lst->size;
}

t_node_t *get_h(h_list *lst, int pos)
{
    if (pos >= lst->size)
    {
        printf("Error: Index out of range1");
        exit(-1);
    }

    node *curr = lst->head;
    for (int i = 0; i < pos; i++)
    {
        curr = curr->next;
    }

    return curr->elem;
}

t_node_t *pop(h_list *lst)
{
    t_node_t *elem = get_h(lst, 0);
    delete_h(lst, 0);
    return elem;
}

void insert_h(h_list *lst, int pos, t_node_t *elem)
{

    if (pos > lst->size)
    {
        printf("Error: Index out of range");
        exit(-1);
    }
    if (pos == lst->size)
    {
        add_h(lst, elem);
        return;
    }

    node *new = (node *)malloc(sizeof(node));
    new->elem = elem;

    node *curr = lst->head;
    for (int i = 0; i < pos - 1; i++)
    {
        curr = curr->next;
    }

    new->next = curr->next;
    curr->next = new;
    lst->size++;
}