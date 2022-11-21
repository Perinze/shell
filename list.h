typedef struct _Node {
  char *value;
  struct _Node *next;
} Node;

void linsert(Node*, char*);
Node *llast(Node*);
void lappend(Node*, char*);
Node *lfind(Node*, int);
void lprint(Node*);
void lerasenext(Node*);
void lerase(Node*, int);
Node *lnext(Node*);
