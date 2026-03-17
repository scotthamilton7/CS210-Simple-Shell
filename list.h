typedef struct node {
    char *value;
    struct node *next;
} Node;

typedef Node** List;

void push(List list, char* value);
List new_list();
char* rem(List list);
int is_empty(List list);
int size(List list);
void clear(List list);
int index_of(List list, char* value);
int contains(List list, char* value);