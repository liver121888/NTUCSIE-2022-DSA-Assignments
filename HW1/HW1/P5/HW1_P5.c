#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#pragma warning(push)
//#pragma warning(disable:6385)
//#pragma warning(disable:6386)
//#pragma warning(disable:6011)


typedef struct node {
	struct node* prev;
	struct node* next;
	int Group;
	int ID;
}node;


node** head;
node** tail;
node** Gtail;
char* closed;
int WCNumber, SituationNumber, GroupNumber;
int FileFlag = 1;



void enter(int g, int id, int m)
{
	node* tmpptr = tail[m];
	// check whether already a node in queue with same group
	while (tmpptr)
	{
		if (tmpptr->Group == g)
		{
			// insert
			node* n = malloc(sizeof(node));
			if (n != NULL)
			{
				n->Group = g;
				n->ID = id;
				n->next = tmpptr->next;
				n->prev = tmpptr;
				tmpptr->next = n;
				// not in tail, link the tmpptr->next
				if (n->next) n->next->prev = n;
				// if the tmpptr is in tail, update tail
				if (tmpptr == tail[m])tail[m] = n;
			}
			return;
		}
		tmpptr = tmpptr->prev;
	}
	// no node with same group, add to tail or first node in queue
	node* n = malloc(sizeof(node));
	if (n != NULL)
	{
		n->Group = g;
		n->ID = id;
	}

	if (tail[m])
	{
		tail[m]->next = n;
		if (n != NULL)
		{
			n->prev = tail[m];
			n->next = NULL;
		}
		tail[m] = n;
	}
	else
	{
		// first node in queue
		if (n != NULL)
		{
			n->next = 0;
			n->prev = NULL;
		}
		head[m] = n;
		tail[m] = n;
	}	
}



void close(int m)
{
	node* tmp = 0;
	closed[m] = 'y';
	for (int k = m; k >= -1; k--)
	{
		if (k == -1)
			k = WCNumber - 1;

		if (closed[k] == 'n')
		{
			while (tail[m] != 0)
			{
				tmp = tail[m]->prev;
				enter(tail[m]->Group, tail[m]->ID, k);
				tail[m] = tmp;
			}
			break;
		}
	}
	tail[m] = 0;
	head[m] = 0;
}

void closeingroup(int m)
{
	node* tmp = 0;
	int gnum;
	closed[m] = 'y';
	for (int k = m; k >= -1; k--)
	{
		if (k == -1)
			k = WCNumber - 1;

		if (closed[k] == 'n')
		{
			tmp = tail[k];
			for (int i = 0; i < GroupNumber; i++) 
				Gtail[i] = 0;
			gnum = -1;
			while (tmp)
			{
				if (tmp->Group != gnum)
				{
					gnum = tmp->Group;
					Gtail[gnum] = tmp;
				}
				tmp = tmp->prev;
			}
			tmp = tail[m];
			gnum = -1;

			node* groupNext = 0;
			node* last = 0, * previous = 0;
			while (tmp)
			{
				previous = tmp->prev;

				if (tmp->Group != gnum)
				{				

					gnum = tmp->Group;
					if (Gtail[gnum])
					{
						groupNext = Gtail[gnum]->next;

						Gtail[gnum]->next = tmp;
						tmp->next = tmp->prev;
						tmp->prev = Gtail[gnum];
					}
					else
					{
						groupNext = 0;
						if (tail[k])
						{
							tail[k]->next = tmp;
							tmp->next = tmp->prev;
							tmp->prev = tail[k];
						}
						else
						{

							head[k] = tmp;
							tmp->next = tmp->prev;
							tmp->prev = Gtail[gnum];
						}
					}
					last = tmp;

					//if (tmp->next == 0 || tmp->next->Group != gnum)
					//{
					//	tmp->next = groupNext;
					//	if (groupNext)
					//		groupNext->prev = tmp;
					//	else
					//		tail[k] = tmp;
					//}

				}
				else
				{
					tmp->next = tmp->prev;
					tmp->prev = last;
					//if (tmp->next == 0 || tmp->next->Group != gnum)
					//{
					//	tmp->next = groupNext;
					//	if (groupNext)
					//		groupNext->prev = tmp;
					//	else
					//		tail[k] = tmp;
					//}
					last = tmp;
				}

				if (tmp->next == 0 || tmp->next->Group != gnum)
				{
					tmp->next = groupNext;
					if (groupNext)
						groupNext->prev = tmp;
					else
						tail[k] = tmp;
				}
				tmp = previous;
			}
			break;
		}
	}
	tail[m] = 0;
	head[m] = 0;
}




void go(int m)
{
	// free?
	if (head[m])
	{
		head[m] = head[m]->next;
		if(head[m])
			head[m]->prev = 0;
		else
			tail[m] = 0;
	}
}

void leave(int m)
{
	// free?
	if (tail[m])
	{
		tail[m] = tail[m]->prev;
		if (tail[m])
			tail[m]->next = 0;
		else
			head[m] = 0;
	}
}



int main()
{
	char c1[6];
	int m, j, i;

	FILE* ptr = 0;

	if (FileFlag)
	{
		ptr = fopen("D:\\Senior_Spring\\DSA\\NTUCSIE-2022-DSA-Assignments\\HW1\\HW1\\hw1_testdata\\P5\\1.in", "r");
		int r = fscanf(ptr, "%d %d %d", &WCNumber, &SituationNumber, &GroupNumber);
	}
	else
	{
		int r = scanf("%d %d %d", &WCNumber, &SituationNumber, &GroupNumber);
	}


	head = malloc(sizeof(node*) * WCNumber);
	tail = malloc(sizeof(node*) * WCNumber);
	closed = malloc(sizeof(char) * WCNumber);
	Gtail = malloc(sizeof(node*) * GroupNumber);
	
	for (int k = 0; k < WCNumber; k++)
	{
		head[k] = 0;
		tail[k] = 0;
		closed[k] = 'n';
	}


	for (int k = 0; k < SituationNumber; k++)
	{
		if (FileFlag)
		{
			int re = fscanf(ptr, "%s", c1);
		}
		else
		{
			int re = scanf("%s", c1);
		}

		switch (c1[0])
		{
		case('e'):
			if (FileFlag)
			{
				int res = fscanf(ptr, "%d %d %d", &i, &j, &m);
			}
			else
			{
				int res = scanf("%d %d %d", &i, &j, &m);
			}
			enter(i, j, m);
			break;
		case('l'):
			if (FileFlag)
			{
				int res = fscanf(ptr, "%d", &m);
			}
			else
			{
				int res = scanf("%d", &m);
			}
			leave(m);
			break;
		case('g'):
			if (FileFlag)
			{
				int res = fscanf(ptr, "%d", &m);
			}
			else
			{
				int res = scanf("%d", &m);
			}
			go(m);
			break;
		case('c'):
			if (FileFlag)
			{
				int res = fscanf(ptr, "%d", &m);
			}
			else
			{
				int res = scanf("%d", &m);
			}
			closeingroup(m);
			break;
		default:
			break;
		}

	}

	node* tmpptr;
	for (int k = 0; k < WCNumber; k++)
	{
		tmpptr = head[k];
		while (tmpptr)
		{
			printf("%d ", tmpptr->ID);
			tmpptr = tmpptr->next;
		}
		printf("\n");
	}

	if (FileFlag && ptr != NULL)
	{
		fclose(ptr);
	}
	return 0;
}

#pragma   warning(pop)  