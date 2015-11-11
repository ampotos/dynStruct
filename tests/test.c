
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct test {
  int a;
  int b;
  int r;
  int (*op)(int, int);
  char op_sign;
};

static int mul(int a, int b) {
  return a * b;
}

static int mdiv(int a, int b) {
  return a / b;
}

static int add(int a, int b) {
  return a + b;
}

static int sub(int a, int b) {
  return a - b;
}

struct list {
  struct list *next;
  char *key;
  char *value;
};

void push(struct list **env, char *val) {
  struct list *node = malloc(sizeof(*node));

  if (node == NULL)
    return;
  node->next = *env;
  *env = node;
  if (strchr(val, '=') == NULL)
    return;
  node->key = strndup(val, (long unsigned int)strchr(val, '=') - (long unsigned int)val);
  node->value = strdup((char *)((long unsigned int)strchr(val, '=') + 1));
}

void dump_list(struct list *l) {
  if (l && l->next)
    dump_list(l->next);
  fprintf(stdout, "%s=%s\n", l->key, l->value);
}

char *concat(struct list *l) {
  char *r = NULL;

  while (l->next) {
    int a = 0;
    if (r)
      a = strlen(r);
    r = realloc(r, a + strlen(l->key) + strlen(l->value) + 2);
    if (a == 0)
      r[a] = 0;
    r = strcat(r, l->key);
    r = strcat(r, l->value);
    l = l->next;
  }
  return r;
}

void free_list(struct list *l) {
  if (l && l->next)
    free_list(l->next);
  free(l->key);
  free(l->value);
  free(l);
}

void print(char *s) {
  fprintf(stderr, "%p: [%s]\n", s, s);
}

int main(int argc, char *argv[], char *env[]) {

  int i = 0;
  struct test **calc;
  struct list *environ = NULL;
  void (*ptr)(char *) = &print;
  
  for (i = 0; env[i]; ++i) {
    if (i % 2 == 0)
      ptr(env[i]);
    push(&environ, env[i]);
  }

  char *lol = concat(environ);
  fprintf(stderr, "\nCONCAT: %s\n\n", lol);
  free(lol);
  
  dump_list(environ);
  free_list(environ);

  fprintf(stdout, "---- Separator. ----\n");
  
  calc = calloc(6, sizeof(struct test *));
  for (i = 0; i < 5; ++i) {
    calc[i] = malloc(sizeof(struct test));
    calc[i]->a = i;
    calc[i]->b = i * i;

    if (i == 0) {
      calc[i]->op = &mul;
      calc[i]->op_sign = '*';
    }
    else if (i == 1) {
      calc[i]->op = &mdiv;
      calc[i]->op_sign = '/';
    }
    else if (i == 3) {
      calc[i]->op = &add;
      calc[i]->op_sign = '+';
    }
    else if (i == 4) {
      calc[i]->op = &sub;
      calc[i]->op_sign = '-';
    }
    else {
      calc[i]->op = &add;
      calc[i]->op_sign = '+';
    } 
    
    calc[i]->r = calc[i]->op(calc[i]->b, calc[i]->a);
    fprintf(stdout, "calc: %d %c %d = %d\n", calc[i]->a, calc[i]->op_sign, calc[i]->b, calc[i]->r);
  }

  for (i = 0; i < 5; ++i) {
    free(calc[i]);
  }
  free(calc);
  return 0;
}
