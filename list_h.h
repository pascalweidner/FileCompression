#ifndef LIST_H_H_
#define LIST_H_H_

#include "huffmanTree.h"

typedef struct h_list h_list;

h_list *createHList();

void destroyHList(h_list *lst);

void add_h(h_list *lst, t_node_t *node);

size_t list_len(h_list *lst);

int delete_h(h_list *lst, int pos);

t_node_t *get_h(h_list *lst, int pos);

t_node_t *pop(h_list *lst);

void insert_h(h_list *lst, int pos, t_node_t *elem);

#endif