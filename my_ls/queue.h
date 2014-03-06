#include <stdio.h>
#include <string.h>


struct q_elem
{
	struct q_elem * next;
	char * name;
};

struct queue
{
	struct q_elem *begin;
	struct q_elem *end;
};

void init_queue(struct queue *Q)
{
	if (Q) {
		Q->end = NULL;
		Q->begin = NULL;
	} else {
		fprintf(stderr, "init_queue:bad pointer to queue\n");
	}
}

void push_queue(struct queue * Q, const char *name)
{
	if (Q){
		struct q_elem * tmp = (struct q_elem*)malloc(sizeof(struct q_elem));
		if (tmp) {
			tmp->next = NULL;
			tmp->name = (char *)malloc(strlen(name)+1);
			if(tmp->name == NULL){
				perror("push_queue:cann't allocate memory");
				exit(-1);
			}
			strcpy(tmp->name, name);
			if (Q->end){
				Q->end->next = tmp;
				Q->end = tmp;
			} else {
				Q->end = tmp;
				Q->begin = tmp;
			}
		} else {
			perror("push_queue:cann't allocate memory");
			exit(-1);
		}
	} else {
		fprintf(stderr, "push_queue:bad pointer to queue\n");
	}
}

int queue_is_empty(struct queue *Q)
{
	if (Q) {
		return Q->end == NULL && Q->begin == NULL;
	} else {
		fprintf(stderr, "queue_is_empty:bad pointer to queue\n");
		return 0;
	}
}

void pop_queue(struct queue* Q, char name[])
{
	if (Q) {
		if (Q->begin != NULL) {
			if (name) {
				strcpy(name, Q->begin->name);
				free(Q->begin->name);
				if (Q->end == Q->begin) {
					free(Q->end);
					Q->end = Q->begin = NULL;
				} else {
					struct q_elem * tmp = Q->begin;
					Q->begin = Q->begin->next;
					free(tmp);
				}
			} else {
				fprintf(stderr, "pop_queue:bad pointer to name\n");
			}
		} else {
			fprintf(stderr, "pop_queue:queue_is_empty\n");
		}
	} else {
		fprintf(stderr, "pop_queue:bad pointer to queue\n");
	}
		
}