#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#define log printf

void linsert(Node *node, char *s)
{
  Node *new = (Node*)malloc(sizeof(Node));
  new->value = s;
  new->next = lnext(node);
  node->next = new;
}

Node *llast(Node *node)
{
  while (node->next != NULL)
    node = node->next;
  return node;
}

void lappend(Node *node, char *s)
{
  node = llast(node);
  linsert(node, s);
}

Node *lfind(Node *node, int j)
{
  j++;
  while (j--) {
    node = node->next;
  }
  ////log("lfind: return node containing '%s'\n", node->value);
  return node;
}

void lprint(Node *node)
{
  while (node) {
    //log("lprint: '%s'\n", node->value);
    node = node->next;
  }
}

void lerasenext(Node *node)
{
  //log("lerasenext: node is%s null\n", (node == NULL ? "" : " not"));
  Node *next = lnext(node);
  //log("lerasenext: next->value is '%s'\n", next->value);
  //log("lerasenext: node->next is%s null\n", (next == NULL ? "" : " not"));
  node->next = lnext(next);
  free(next);
  next = lnext(node);
  //log("lerasenext: after removing, node->next is%s null\n", (next == NULL ? "" : " not"));
  //log("lerasenext: done\n");
}

void lerase(Node *node, int j)
{
  node = lfind(node, j-1);
  //log("lerase: removing node with '%s' 's next node\n", node->value);
  lerasenext(node);
  //log("lerase: done\n");
}

Node *lnext(Node *node) {
  return (node == NULL ? NULL : node->next);
}

