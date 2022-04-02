//
//  kdtree.c
//  
//
//  Created by Bigdata LAB on 2019/11/05.
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#define MAX_DIM 2
#define COMPARE(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

// kdtree�� node �ڷᱸ�� ����.
struct kd_node_t
{
	double x[MAX_DIM];
	struct kd_node_t *left, *right;
};
struct Rect {
	double min_x, min_y;
	double max_x, max_y;
};
struct candidate_node {
	struct kd_node_t cur_node;
	struct Rect rec;
	int dim;
};
struct kd_node_t *kd_tree;

//priority queue �ڷᱸ��
struct queue_node {
	struct kd_node_t node;
	struct queue_node* next;
	double dist;
	int dim;
};
struct priority_queue {
	struct queue_node *head;
	int size;
};
struct priority_queue resultQ;
struct priority_queue nodeQ;

// �Ÿ��Լ� ����.
static inline double dist(struct kd_node_t *a, struct kd_node_t *b, int dim)
{
	double t, d = 0;
	while (dim--)
	{
		t = a->x[dim] - b->x[dim];
		d = d + t * t;
	}

	return d;
}

// swap �Լ� ����.
static inline void swap(struct kd_node_t *x, struct kd_node_t *y)
{
	double tmp[MAX_DIM];
	memcpy(tmp, x->x, sizeof(tmp));
	memcpy(x->x, y->x, sizeof(tmp));
	memcpy(y->x, tmp, sizeof(tmp));
}

// �߾Ӱ� ã�� �Լ� ����. �߾Ӱ��� kdtree�� node split point�� ã�� �� ���ȴ�.
struct kd_node_t* find_median(struct kd_node_t *start, struct kd_node_t *end, int idx)
{
	if (end <= start) return NULL;
	if (end == start + 1)
		return start;

	struct kd_node_t *p, *store, *med;
	med = start + (end - start) / 2;
	double pivot;

	while (1)
	{
		pivot = med->x[idx];

		swap(med, end - 1);
		for (store = p = start; p < end; p++)
			if (p->x[idx] < pivot)
			{
				if (p != store)
					swap(p, store);
				store++;
			}
		swap(store, end - 1);

		if (store->x[idx] == med->x[idx])
			return med;
		if (store > med) end = store;
		else start = store;
	}
}

// recursion���� kdtree�� build�ϴ� �Լ�.
struct kd_node_t* make_kdtree(struct kd_node_t *t, int len, int i, int dim)
{
	struct kd_node_t *n;

	if (!len) return NULL;

	if ((n = find_median(t, t + len, i)))
	{
		i = (i + 1) % dim;
		n->left = make_kdtree(t, n - t, i, dim);
		n->right = make_kdtree(n + 1, t + len - (n + 1), i, dim);
	}

	return n;
}

void rangeQuery(struct kd_node_t *p, double point_x, double point_y, double r)
{
	struct candidate_node* stack;
	stack = (struct candidate_node*)malloc(sizeof(struct candidate_node) * 1000000);
	int top = 0;
	double dZero = 0.0;
	double distance;
	struct kd_node_t q;
	struct candidate_node *start;
	start = (struct candidate_node*)malloc(sizeof(struct candidate_node));
	start->cur_node = kd_tree[0];
	start->dim = 0;
	start->rec.min_x = 0;
	start->rec.min_y = 0;

	start->rec.max_x = 1 / dZero;
	start->rec.max_y = 1 / dZero;

	stack[top] = *start;

	int count = 0;
	q.x[0] = point_x;
	q.x[1] = point_y;

	while (top >= 0) {
		struct candidate_node top_node = stack[top];
		distance = sqrt(pow(top_node.cur_node.x[0] - point_x, 2) + pow(top_node.cur_node.x[1] - point_y, 2));
		if (distance <= r) {
			//printf("(%lf, %lf)\n", top_node.cur_node.x[0], top_node.cur_node.x[1]);
			count += 1;
		}
		top--;

		if (top_node.dim % 2 == 0) {//x
			if (top_node.cur_node.x[0] > point_x - r && top_node.cur_node.left) {//left node
				struct candidate_node left;
				left.cur_node = *top_node.cur_node.left;

				left.dim = 1;
				left.rec.min_x = top_node.rec.min_x;
				left.rec.min_y = top_node.rec.min_y;
				left.rec.max_x = top_node.cur_node.x[0];
				left.rec.max_y = top_node.rec.max_y;
				top += 1;
				stack[top] = left;
			}
			if (top_node.cur_node.x[0] < point_x + r && top_node.cur_node.right) {//right_node
				struct candidate_node right;
				right.cur_node = *top_node.cur_node.right;
				right.dim = 1;
				right.rec.min_x = top_node.cur_node.x[0];
				right.rec.min_y = top_node.rec.min_y;
				right.rec.max_x = top_node.rec.max_x;
				right.rec.max_y = top_node.rec.max_y;
				top += 1;
				stack[top] = right;
			}

		}
		else { //y
			if (top_node.cur_node.x[1] > point_y - r && top_node.cur_node.left) {//left node
				//stack push
				struct candidate_node left;
				left.cur_node = *top_node.cur_node.left;
				left.dim = 0;
				left.rec.min_x = top_node.rec.min_x;
				left.rec.min_y = top_node.rec.min_y;
				left.rec.max_x = top_node.rec.max_x;
				left.rec.max_y = top_node.cur_node.x[1];
				top += 1;
				stack[top] = left;
			}
			if (top_node.cur_node.x[1] < point_y + r && top_node.cur_node.right) {//right_node
				//stack_push
				struct candidate_node right;
				right.cur_node = *top_node.cur_node.right;
				right.dim = 0;
				right.rec.min_x = top_node.rec.min_x;
				right.rec.min_y = top_node.cur_node.x[1];
				right.rec.max_x = top_node.rec.max_x;
				right.rec.max_y = top_node.rec.max_y;
				top += 1;
				stack[top] = right;
			}

		}

	}
	printf("count: %d\n", count);

	//range query�� ���� ������ ���� ����Ʈ�� ���� �ݰ�
}
void print_queue(struct priority_queue queue);
void init_queue(struct priority_queue *queue, struct kd_node_t *p, struct kd_node_t *q) {
	struct queue_node * new;

	new = (struct queue_node*)malloc(sizeof(struct queue_node));
	new->node = *p;
	new->next = NULL;
	new->dim = 0;
	new->dist = dist(p, q, 2);
	queue->head = new;
	queue->size += 1;

}
void enqueue_node(struct kd_node_t *n, struct kd_node_t *q, int dim) {
	struct queue_node *new;
	new = (struct queue_node *)malloc(sizeof(struct queue_node));
	double distance;
	distance = sqrt(pow(n->x[0] - q->x[0], 2) + pow(n->x[1] - q->x[1], 2));
	new->dist = distance;
	new->next = NULL;
	new->node = *n;
	new->dim = dim;


	struct queue_node *ptr;

	if (nodeQ.size == 0) {
		nodeQ.head = new;
		nodeQ.size += 1;
		//init_queue(&nodeQ,n,q);
		return;
	}

	ptr = nodeQ.head;
	if (ptr->dist >= new->dist) {
		new->next = ptr;
		nodeQ.head = new;
		nodeQ.size += 1;
		return;
	}


	while (ptr->dist < new->dist) {
		if (!ptr->next) break;
		if (ptr->next->dist > new->dist) break;
		ptr = ptr->next;
	}
	new->next = ptr->next;
	ptr->next = new;
	nodeQ.size += 1;

}
void enqueue_res(struct priority_queue *queue, struct kd_node_t n, struct kd_node_t *q) {
	struct queue_node *new;
	new = (struct queue_node *)malloc(sizeof(struct queue_node));
	double distance;
	distance = sqrt(pow(n.x[0] - q->x[0], 2) + pow(n.x[1] - q->x[1], 2));
	new->dist = distance;
	new->next = NULL;
	new->node = n;
	if (queue->size == 0) {
		//queue->head=new;
		//queue->size+=1;
		init_queue(queue, &n, q);
		return;
	}


	struct queue_node *ptr;
	ptr = queue->head;

	if (ptr->dist < new->dist) {
		new->next = ptr;
		queue->head = new;
		queue->size += 1;
		return;
	}
	else if (ptr->dist == new->dist) {
		return;
	}
	while (ptr->dist > new->dist) {
		if (!ptr->next) break;
		if (ptr->next->dist < new->dist) break;
		ptr = ptr->next;
	}
	new->next = ptr->next;
	ptr->next = new;
	queue->size += 1;
}

void print_queue(struct priority_queue queue) {
	struct queue_node *ptr;
	ptr = queue.head;
	for (int i = 0; i < queue.size; i++) {
		printf("(%.2lf %.2lf)	 dist: %lf \n", ptr->node.x[0], ptr->node.x[1], ptr->dist);
		ptr = ptr->next;
		if (!ptr)
			break;
	}
}
int check_intersect(struct queue_node top_node, struct kd_node_t *q, double r) {

	if (top_node.dim % 2 == 0) {//x������ �������
		if (top_node.node.x[0] >= q->x[0] - r && top_node.node.left) {//left node
			enqueue_node(top_node.node.left, q, 1);
		}
		if (top_node.node.x[0] <= q->x[0] + r && top_node.node.right) {//right_node
			enqueue_node(top_node.node.right, q, 1);

		}

	}
	else { //y������ ���� ��
		if (top_node.node.x[1] >= q->x[1] - r && top_node.node.left) {//left node
			enqueue_node(top_node.node.left, q, 0);
		}
		if (top_node.node.x[1] <= q->x[1] + r && top_node.node.right) {//right_node
			enqueue_node(top_node.node.right, q, 0);
		}

	}



}

void kNNquery(double point_x, double point_y, int K)
{
	struct kd_node_t cn;
	double R = DBL_MAX;
	int count = 0;
	struct kd_node_t q;
	q.x[0] = point_x;
	q.x[1] = point_y;

	nodeQ.size = 0;
	resultQ.size = 0;
	init_queue(&nodeQ, &kd_tree[0], &q);
	while (nodeQ.size > 0) {
		struct queue_node *tmp;
		tmp = nodeQ.head;
		nodeQ.head = nodeQ.head->next;
		nodeQ.size -= 1;

		if (dist(&tmp->node, &q, 2) < R) {
			if (count < K) {
				enqueue_res(&resultQ, tmp->node, &q);
				count += 1;
			}
			else if (K == 1) {
				resultQ.size -= 1;
				init_queue(&resultQ, &tmp->node, &q);
			}
			else {
				resultQ.head = resultQ.head->next;
				enqueue_res(&resultQ, tmp->node, &q);
			}

		}
		if (count == K && R > resultQ.head->dist) {
			R = resultQ.head->dist;

		}
		check_intersect(*tmp, &q, R);

	}

	printf("KD-TREE KNN QUERY RESULT K:");
	printf("%d \n", K);
	print_queue(resultQ);
	printf("R:%lf\n", R);
	//kNN query�� ���������� ���� ����Ʈ�� �ֱ����̿� ����

}
int load_data(struct kd_node_t* kd_tree, int filetype) {
	FILE *fp = NULL;
	int i = 0;
	char v;
	double x, y;

	switch (filetype) {
	case 1:
		fp = fopen("../geospatial_data/clustered_dataset.txt", "r");
		break;
	case 2:
		fp = fopen("../geospatial_data/gaussian_dataset.txt", "r");
		break;
	case 3:
		fp = fopen("../geospatial_data/uniformed_dataset.txt", "r");
		break;
	}

	while (fscanf(fp, "%lf%c%c%lf", &x, &v, &v, &y) != EOF) {
		kd_tree[i].x[0] = x;
		kd_tree[i].x[1] = y;
		kd_tree[i].left = NULL;
		kd_tree[i].right = NULL;
		i += 1;
	}
	printf("%lf %lf", kd_tree[0].x[0], kd_tree[0].x[1]);
	return i;
}


int main(void)
{

	kd_tree = (struct kd_node_t*)malloc(sizeof(struct kd_node_t) * 1000001);
	int query_type, file_type, k, len;
	double point_x, point_y, range;
	printf("1. Range query\n");
	printf("2. KNN query\n");
	scanf("%d", &query_type);
	printf("file\n");
	printf("1. clustered_dataset\n");
	printf("2. gaussian_dataset\n");
	printf("3. uniformed_dataset\n");
	scanf("%d", &file_type);

	if (query_type != 1 && query_type != 2) {
		printf("Error: Invalid query type");
		return -1;
	}
	printf("Please enter the query point:  \n");
	printf("Point x: ");
	scanf("%lf", &point_x);
	printf("Point y: ");
	scanf("%lf", &point_y);
	len = load_data(kd_tree, file_type);

	kd_tree = make_kdtree(kd_tree, len, 0, 2);
	clock_t start, end;
	if (query_type == 1) {
		printf("Please enter the d ");
		scanf("%lf", &range);
		start = clock();
		rangeQuery(kd_tree, point_x, point_y, range*0.01);
		end = clock();
		printf("range*0.01 Execution time: %lf \n", (double)(end - start));
		printf("d*0.01�� ����Դϴ�.\n");
		start = clock();
		rangeQuery(kd_tree, point_x, point_y, range*0.02);
		end = clock();
		printf("range*0.02 Execution time: %lf \n", (double)(end - start));
		printf("d*0.02�� ����Դϴ�. \n");
		start = clock();
		rangeQuery(kd_tree, point_x, point_y, range*0.04);
		end = clock();
		printf("range*0.04 Execution time: %lf \n", (double)(end - start));
		printf("d*0.04�� ����Դϴ�. \n");
		start = clock();
		rangeQuery(kd_tree, point_x, point_y, range*0.06);
		end = clock();
		printf("range*0.06 Execution time: %lf \n", (double)(end - start));
		printf("d*0.06�� ����Դϴ�. \n");
		start = clock();
		rangeQuery(kd_tree, point_x, point_y, range*0.08);
		end = clock();
		printf("range*0.08 Execution time: %lf \n", (double)(end - start));
		printf("d*0.08�� ����Դϴ�. \n");
		start = clock();
		rangeQuery(kd_tree, point_x, point_y, range*0.1);
		end = clock();
		printf("range*0.1 Execution time: %lf \n", (double)(end - start));
		printf("d*0.1�� ����Դϴ�. \n");

	}
	else if (query_type == 2) {

		printf("Please enter k: �ڵ����� 1-101�� ���");
		//scanf("%d", &k);
		
		k = 1;
		while (k < 102) {
			start = clock();
			kNNquery(point_x, point_y, k);
			end = clock();
			printf("Execution time: %lf\n", (double)(end - start));
			k = k + 10;
		};
	}

	return 0;
}




