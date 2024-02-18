#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HEAP_SIZE	(1024 * 1024 * 16)
#define ALLOCATED	((struct node *) 1)

struct node
{
	_Alignas(max_align_t)
	struct node *	free;
	struct node **	link;
	struct node *	prev;
	struct node *	next;
};

union heap
{
	struct node	node;
	char		data[HEAP_SIZE];
};

static struct node *	list;
static union heap	heap		=
{
			.node.link	= &list,
};
static struct node *	list		= &heap.node;

static inline size_t node_get_size(struct node *node)
{
	void *next = node->next;

	if (next == NULL)
	{
		next = &heap.data[sizeof(heap.data)];
	}

	return (char *) next - (char *) node;
}

static inline size_t pad_size(size_t size)
{
	size_t	algn;

	algn = _Alignof(struct node);
	size = (size + sizeof(struct node));
	size = (size + algn - 1) / algn * algn;

	return size;
}

void *my_malloc(size_t size)
{
	struct node *	node;
	size_t		node_size;
	size_t		splt_size;

	if (size == 0)
	{
		return NULL;
	}

	node = list;

	size = pad_size(size);
	splt_size = size + pad_size(1);

	while (node != NULL)
	{
		node_size = node_get_size(node);

		if (node_size < size)
		{
			node = node->free;

			continue;
		}

		if (node_size < splt_size)
		{
			*node->link = node->free;

			if (node->free != NULL)
			{
				node->free->link = node->link;
			}

			node->free = ALLOCATED;
		}
		else
		{
			char *		splt	= (char *) node + splt_size;
			struct node *	next	= (void *) splt;

			next->free = node->free;
			node->free = ALLOCATED;

			if (next->free != NULL)
			{
				next->free->link = &next->free;
			}

			next->link = node->link;
			*next->link = next;

			next->prev = node;
			next->next = node->next;

			if (next->next != NULL)
			{
				next->next->prev = next;
			}

			node->next = next;
		}

		break;
	}

	if (node != NULL)
	{
		return node + 1;
	}
	else
	{
		return NULL;
	}
}

void my_free(void *ptr)
{
	struct node *	node;

	if (ptr == NULL)
	{
		return;
	}

	node = ptr;
	node = node - 1;

	if (node->prev != NULL && node->prev->free != ALLOCATED)
	{
		node->prev->next = node->next;

		if (node->next != NULL)
		{
			node->next->prev = node->prev;
		}

		node = node->prev;
	}
	else
	{
		node->free = list;

		if (node->free != NULL)
		{
			node->free->link = &node->free;
		}

		node->link = &list;
		list = node;
	}

	if (node->next != NULL && node->next->free != ALLOCATED)
	{
		*node->next->link = node->next->free;

		if (node->next->free != NULL)
		{
			node->next->free->link = node->next->link;
		}

		node->next = node->next->next;

		if (node->next != NULL)
		{
			node->next->prev = node;
		}
	}
}

static void print_heap(void)
{
	int		err;
	struct node *	node;
	size_t		size;

	err = 0;

	//fprintf(stderr, "node list\n");

	node = &heap.node;

	while (node != NULL)
	{
		size = node_get_size(node);

		if (node->free == ALLOCATED)
		{
			//fprintf(stderr, "    A 0x%08lx @ %p\n", size, node);
		}
		else
		{
			//fprintf(stderr, "    F 0x%08lx @ %p\n", size, node);
		}

		//fprintf(stderr, "        free  %p\n", node->free);
		//fprintf(stderr, "        link  %p\n", node->link);
		//fprintf(stderr, "        prev  %p\n", node->prev);
		//fprintf(stderr, "        next  %p\n", node->next);

		if (node->free != ALLOCATED && node->free != NULL)
		{
			if (node->free->link != &node->free)
			{
				fprintf(stderr, "ERR  (free)\n");

				err = 1;
			}
		}

		if (node->free != ALLOCATED)
		{
			if (*node->link != node)
			{
				fprintf(stderr, "ERR  (link)\n");

				err = 1;
			}
		}

		if (node->prev != NULL)
		{
			if (node->prev->next != node)
			{
				fprintf(stderr, "ERR  (prev)\n");

				err = 1;
			}
		}

		if (node->next != NULL)
		{
			if (node->next->prev != node)
			{
				fprintf(stderr, "ERR  (next)\n");

				err = 1;
			}
		}

		if (node->free != ALLOCATED)
		{
			struct node *	k	= list;

			while (k != NULL && k != node)
			{
				k = k->free;
			}

			if (k != node)
			{
				fprintf(stderr, "ERR  (free list)\n");

				err = 1;
			}
		}

		if (	node->free == node		||
			node->link == &node->free	||
			node->prev == node		||
			node->next == node		)
		{
			fprintf(stderr, "ERR  (loop)\n");

			abort();
		}

		node = node->next;
	}

	//fprintf(stderr, "free list\n");

	node = list;

	while (node != NULL)
	{
		size = node_get_size(node);

		if (node->free == ALLOCATED)
		{
			//fprintf(stderr, "    A 0x%08lx @ %p\n", size, node);
		}
		else
		{
			//fprintf(stderr, "    F 0x%08lx @ %p\n", size, node);
		}

		//fprintf(stderr, "        free  %p\n", node->free);
		//fprintf(stderr, "        link  %p\n", node->link);
		//fprintf(stderr, "        prev  %p\n", node->prev);
		//fprintf(stderr, "        next  %p\n", node->next);

		if (node->free != ALLOCATED && node->free != NULL)
		{
			if (node->free->link != &node->free)
			{
				fprintf(stderr, "ERR  (free)\n");

				err = 1;
			}
		}

		if (node->free != ALLOCATED)
		{
			if (*node->link != node)
			{
				fprintf(stderr, "ERR  (link)\n");

				err = 1;
			}
		}

		if (node->prev != NULL)
		{
			if (node->prev->next != node)
			{
				fprintf(stderr, "ERR  (prev)\n");

				err = 1;
			}
		}

		if (node->next != NULL)
		{
			if (node->next->prev != node)
			{
				fprintf(stderr, "ERR  (next)\n");

				err = 1;
			}
		}

		{
			struct node *	k	= &heap.node;

			while (k != NULL && k != node)
			{
				k = k->next;
			}

			if (k != node)
			{
				fprintf(stderr, "ERR  (node list)\n");

				err = 1;
			}
		}

		if (	node->free == node		||
			node->link == &node->free	||
			node->prev == node		||
			node->next == node		)
		{
			fprintf(stderr, "ERR  (loop)\n");

			abort();
		}

		node = node->free;
	}

	if (err)
	{
		abort();
	}
}

int main()
{
#define NPTR	100
#define NACT	1000
#define MAX	(1024 * 1024)
	struct node *	ptr[NPTR]	= { };

	srand(time(NULL));

	print_heap();

	for (int i = 0; i < NACT; i++)
	{
		int	n	= rand() % NPTR;
		size_t	size;

		if (ptr[n] == NULL)
		{
			size = rand() % MAX;

			ptr[n] = my_malloc(size);

			if (ptr[n] != NULL)
			{
				fprintf(stderr, "alloc 0x%08lx @ %p\n",
					size, ptr[n] - 1);

				for (int j = 0; j < size; j++)
				{
					((char *) ptr[n])[j] = j;
				}
			}
			else
			{
				fprintf(stderr, "alloc 0x%08lx @ %p\n",
					size, ptr[n]);
			}
		}
		else
		{
			size = node_get_size(ptr[n] - 1);

			fprintf(stderr, "free 0x%08lx @ %p\n",
				size, ptr[n] - 1);

			{
				struct node *	k	= &heap.node;

				while (k != NULL && k != ptr[n] - 1)
				{
					k = k->next;
				}

				if (k != ptr[n] - 1)
				{
					fprintf(stderr, "ERR  (list)\n");

					abort();
				}
			}

			my_free(ptr[n]);

			ptr[n] = NULL;
		}

		print_heap();
	}

	for (int i = 0; i < NPTR; i++)
	{
		int	n	= i;
		size_t	size;

		if (ptr[n] != NULL)
		{
			size = node_get_size(ptr[n] - 1);

			fprintf(stderr, "free 0x%08lx @ %p\n",
				size, ptr[n] - 1);

			{
				struct node *	k	= &heap.node;

				while (k != NULL && k != ptr[n] - 1)
				{
					k = k->next;
				}

				if (k != ptr[n] - 1)
				{
					fprintf(stderr, "ERR  (list)\n");

					abort();
				}
			}

			my_free(ptr[n]);

			ptr[n] = NULL;

			print_heap();
		}
	}
}
