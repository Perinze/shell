#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAXBUF 2048
#define MAXLINE 4096
#define DELIM " "

typedef struct _Node {
  char *value;
  struct _Node *next;
} Node;

typedef struct {
  int argc;
  char **argv;
  Node node;
} Command;

typedef struct {
  char *name;
  char *value;
} Variable;

typedef struct {
  char *name;
  char *line;
} Alias;

Variable variables[MAXBUF];
Alias aliases[MAXBUF];

void prompt();
Command readcmd();
void dispatch(Command);
char *readline();
Command parsecmd(char*);
char *cmdget(Command, int);
char **cmdgetall(Command);
void cmdprint_rest(Command, int);
int counttok(char*);
void set(Command);
Variable *findfreevar();
char *cmdconcat_rest(Command, int, char*);
void alias(Command);
Alias *findfreealias();
Command checkcmd(Command);
void linsert(Node*, char*);
Node *llast(Node*);
void lappend(Node*, char*);
Node *lfind(Node*, int);

int main()
{
  for (;;) {
    prompt();
    Command cmd = readcmd();
    dispatch(cmd);
  }
}

void prompt()
{
  printf("$ ");
}

Command readcmd()
{
  char *line = readline();
  printf("readcmd: readline() returns '%s'\n", line);
  Command cmd = parsecmd(line);
  cmd = checkcmd(cmd);
  return cmd;
}

void dispatch(Command cmd)
{
  printf("dispatch: enter\n");
  if (cmd.argc == 0) return;
  char *first = cmdget(cmd, 0);
  printf("dispatch: first is '%s'\n", first);
  char **argv = cmdgetall(cmd);
  for (int i = 0; i < cmd.argc; i++) {
    printf("dispatch: argv[%d] is '%s'\n", i, argv[i]);
  }
  if (strcmp(first, "echo") == 0) {
    cmdprint_rest(cmd, 1);
  } else if (strcmp(first, "exit") == 0) {
    exit(0); // TODO
  } else if (strcmp(first, "set") == 0) {
    set(cmd);
  } else if (strcmp(first, "alias") == 0) {
    alias(cmd);
  } else {
    pid_t pid;
    if ((pid = fork()) == 0) {
      if (execve(first, argv, NULL) == -1) {
        perror("exec");
      }
    } else {
      int stat;
      waitpid(pid, &stat, 0);
      if (WIFEXITED(stat) && WEXITSTATUS(stat) != 0) {
        fprintf(stderr, "exit code %d\n", WEXITSTATUS(stat));
      }
    }
  }
}

char *readline()
{
  char *line = malloc(MAXLINE);
  fgets(line, MAXLINE, stdin); // TODO error handling
  line[strlen(line)-1] = '\0';
  return line;
}

Command parsecmd(char *line)
{
  Command cmd;
  cmd.node.next = NULL;
  // parse in buffer
  //char *buf[MAXBUF];
  char *buf;
  cmd.argc = 0;
  //buf[0] = strtok(line, DELIM);
  buf = strtok(line, DELIM);
  //if (buf[0] == NULL) {
  if (buf == NULL) {
    return cmd;
  }
  cmd.argc++;
  lappend(&cmd.node, buf);
  printf("parsecmd: buf[0] is '%s'\n", buf);
  //while (buf[++cmd.argc] = strtok(NULL, DELIM)) {
  while (buf = strtok(NULL, DELIM)) {
    ++cmd.argc;
    printf("parsecmd: buf[%d] is '%s'\n", cmd.argc, buf);
    lappend(&cmd.node, buf);
  }
  printf("parsecmd: argc is %d\n", cmd.argc);

  /*
  cmd.argv = (char**)malloc((cmd.argc + 1) * sizeof(char*));
  cmd.argv[cmd.argc] = NULL;
  */
  Node *node = &cmd.node;
  for (int i = 0; i < cmd.argc; i++) {
    //cmd.argv[i] = buf[i];
    node = node->next;
    printf("parsecmd: argv[0] is '%s'\n", node->value);
  }
  return cmd;
}

int counttok(char *line)
{
  int n = 1;
  strtok(line, DELIM);
  while (strtok(NULL, DELIM))
    n++;
  return n;
}

char *cmdget(Command cmd, int j)
{
  return lfind(&cmd.node, j)->value;
}

char **cmdgetall(Command cmd)
{
  char **argv = (char**)malloc(cmd.argc * sizeof(char*));
  Node *node = cmd.node.next;
  for (int i = 0; i < cmd.argc; i++) {
    argv[i] = node->value;
    node = node->next;
  }
  return argv;
}

void cmdprint_rest(Command cmd, int from)
{
  for (int i = from; i < cmd.argc; i++) {
    //printf("%s ", cmd.argv[i]);
    printf("%s ", cmdget(cmd, i));
  }
  puts("");
}

char *cmdconcat_rest(Command cmd, int from, char *delim)
{
  int space = cmd.argc - from - 1;
  int len = space + 1;
  for (int i = from; i < cmd.argc; i++) {
    //len += strlen(cmd.argv[i]);
    len += strlen(cmdget(cmd, i));
  }
  char *line = (char*)malloc(len);
  strcpy(line, cmdget(cmd, from));
  for (int i = from+1; i < cmd.argc; i++) {
    strcat(line, delim);
    strcat(line, cmdget(cmd, i));
  }
  return line;
}

void set(Command cmd)
{
  if (cmd.argc == 1) {
    for (int i = 0; i < MAXBUF; i++) {
      if (variables[i].name == NULL) continue;
      printf("%s\t%s\n", variables[i].name, variables[i].value);
    }
  } else {
    char assign[256];
    strncpy(assign, cmdget(cmd, 1), 256);
    printf("set: assign %s\n", assign);
    char *name = strtok(assign, "=");
    char *value = strtok(NULL, "=");
    printf("set: name %s\n", name);
    printf("set: value %s\n", value);
    Variable *v = findfreevar();
    v->name = malloc(strlen(name)+1);
    strcpy(v->name, name);
    v->value = malloc(strlen(value)+1);
    strcpy(v->value, value);
    printf("set: v->name %s\n", v->name);
    printf("set: v->value %s\n", v->value);
  }
}

Variable *findfreevar()
{
  for (int i = 0; i < MAXBUF; i++) {
    if (variables[i].name == NULL) {
      return &variables[i];
    }
  }
  return NULL;
}

void alias(Command cmd)
{
  if (cmd.argc == 1) {
    for (int i = 0; i < MAXBUF; i++) {
      if (aliases[i].name == NULL) continue;
      printf("%s\t%s\n", aliases[i].name, aliases[i].line);
    }
  } else {
    char *name = cmdget(cmd, 1);
    char *line = cmdconcat_rest(cmd, 2, " ");
    printf("alias: name %s\n", name);
    printf("alias: line %s\n", line);
    Alias *a = findfreealias();
    a->name = malloc(strlen(name)+1);
    strcpy(a->name, name);
    a->line = malloc(strlen(line)+1);
    strcpy(a->line, line);
    printf("alias: a->name %s\n", a->name);
    printf("alias: a->line %s\n", a->line);
  }
}

Alias *findfreealias()
{
  for (int i = 0; i < MAXBUF; i++) {
    if (aliases[i].name == NULL) {
      return &aliases[i];
    }
  }
  return NULL;
}

Command checkcmd(Command cmd)
{
  for (int i = 1; i < cmd.argc; i++) {
    if (cmdget(cmd, i)[0] == '*') {
      char *postfix = cmdget(cmd, i)[1];
      int l = strlen(postfix);
      char cwd[MAXLINE];
      getcwd(cwd, sizeof(cwd));
      DIR *dir = opendir(cwd);
      int dfd = dirfd(dir);
      struct dirent *ent;
      while ((ent = readdir(dir)) != NULL) {
        char *name = ent->d_name;
        int len = strlen(name);
        if (strcmp(&name[len-l], postfix) == 0) {
          linsert(lfind(&cmd.node, i), name);
          i++;
          cmd.argc++;
        }
      }
    }
  }
}

void linsert(Node *node, char *s)
{
  Node *new = (Node*)malloc(sizeof(Node));
  new->value = s;
  new->next = node->next;
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
  //printf("lfind: return node containing '%s'\n", node->value);
  return node;
}
