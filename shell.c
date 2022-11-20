#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXBUF 2048
#define MAXLINE 4096
#define DELIM " "

typedef struct {
    int argc;
    char **argv;
} Command;

typedef struct {
    char *name;
    char *value;
} Variable;

Variable var[MAXBUF];

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
    return cmd;
}

void dispatch(Command cmd)
{
    if (cmd.argc == 0) return;
    char *first = cmdget(cmd, 0);
    printf("dispatch: first is '%s'\n", first);
    char **argv = cmdgetall(cmd);
    if (strcmp(first, "echo") == 0) {
        cmdprint_rest(cmd, 1);
    } else if (strcmp(first, "exit") == 0) {
        exit(0); // TODO
    } else if (strcmp(first, "set") == 0) {
        set(cmd);
    } else if (strcmp(first, "alias") == 0) {

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
    // parse in buffer
    char *buf[MAXBUF];
    cmd.argc = 0;
    buf[0] = strtok(line, DELIM);
    if (buf[0] == NULL) {
        return cmd;
    }
    printf("parsecmd: buf[0] is '%s'\n", buf[0]);
    while (buf[++cmd.argc] = strtok(NULL, DELIM)) {
        printf("parsecmd: buf[%d] is '%s'\n", cmd.argc, buf[cmd.argc]);
    }
    printf("parsecmd: argc is %d\n", cmd.argc);

    cmd.argv = (char**)malloc((cmd.argc + 1) * sizeof(char*));
    cmd.argv[cmd.argc] = NULL;
    for (int i = 0; i < cmd.argc; i++) {
        cmd.argv[i] = buf[i];
        //printf("parsecmd: argv[0] is '%s'\n", cmd.argv[0]);
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
    return cmd.argv[j];
}

char **cmdgetall(Command cmd)
{
    return cmd.argv;
}

void cmdprint_rest(Command cmd, int from)
{
    for (int i = from; i < cmd.argc; i++) {
        printf("%s ", cmd.argv[i]);
    }
    puts("");
}

void set(Command cmd)
{
    if (cmd.argc == 1) {
        for (int i = 0; i < MAXBUF; i++) {
            if (var[i].name == NULL) continue;
            printf("%s\t%s\n", var[i].name, var[i].value);
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
        if (var[i].name == NULL) {
            return &var[i];
        }
    }
    return NULL;
}
