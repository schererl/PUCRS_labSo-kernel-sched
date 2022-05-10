/*
 * SSTF IO Scheduler
 *
 * For Kernel 4.13.9
 */


#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

long long unsigned int last_pos = 0;

/* SSTF data structure. */
struct sstf_data {
	struct list_head queue;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

/* valor absolutor para long long int */
static long long int abslong(long long int in){
	if (in < 0){
		return in *-1;	
	}
	return in;
}

/*Função para debug */
static void printList(struct sstf_data *nd ){
	printk(KERN_EMERG "INIT");
	struct list_head *ptr;
	struct request *rq;
	list_for_each(ptr, &nd->queue){
		rq = list_entry(ptr, struct request, queuelist); 
		long long unsigned int new_pos = blk_rq_pos(rq);
		printk(KERN_EMERG "%llu", new_pos);
	}
	printk(KERN_EMERG "END");
}

/* Na versão CAD o dispatch sempre é feito no header */
static int sstf_dispatch(struct request_queue *q, int force){
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	struct request *rq;
	//primeiro da lista
	rq = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	if (rq) {
		last_pos = blk_rq_pos(rq);
        list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		//printList(nd);
		printk(KERN_EMERG "[SSTF] dsp %c %llu\n", direction, last_pos);

		return 1;
	}
	return 0;
}

/*
	Adiciona de maneira ordenada, compara a distância entre pares de nodos conseguintes,
	se o novo request tem uma distância menor que o par corrente, coloca ele no meio.
*/
static void sstf_add_request(struct request_queue *q, struct request *rq){
	printk(KERN_EMERG "[SSTF] add %c %llu", direction, blk_rq_pos(rq));
	
	
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	
	struct list_head *ptr; 
	long long unsigned int req_pos = blk_rq_pos(rq);	
	struct request *head_req = list_first_entry_or_null(&nd->queue, struct request, queuelist);
	long long unsigned int head_pos = (long long unsigned int) blk_rq_pos(head_req);

	// Lista vazia: adiciona em tail
	
	if(head_req == NULL){
		list_add_tail(&rq->queuelist, &nd->queue);
        printk(KERN_EMERG "[SSTF] add %c %llu\n", direction, blk_rq_pos(rq));
	
		return;
	}
	
	// Adiciona no head: Caso sua distância entre rq e head seja menor que head e next head
	// |rq.pos - head| < |head.next - head| ==> rq <-> old_head <-> old_head.next ....
	
	if(abslong(req_pos - last_pos) < abslong(head_pos  - last_pos)){
		struct list_head *old_head = &nd->queue;
        struct list_head *new_head = &rq->queuelist;
        list_add(new_head, old_head);
        
		return;
	} 
	
	// Adiciona no meio: Se a distacia entre rq e o nodo atual for menor que nodo atual e seu sucessor, adiciona ele no meio
	
	ptr = (&(nd -> queue))->next; //pula head porque já foi avaliado anteriormente.
	while(ptr->next != &(nd -> queue)){
		struct list_head *next = ptr->next;
		//pega a pos 
		struct request *curE = list_entry(ptr, struct request, queuelist); 
		struct request *nextE = list_entry(next, struct request, queuelist); 
		long long unsigned int curPos = blk_rq_pos(curE);
		long long unsigned int nextPos = blk_rq_pos(nextE);

		// verifica se deve se rq deve ser inserido no meio
		if(abslong(curPos - nextPos) > abslong(curPos - req_pos)){
			list_add(&rq->queuelist, ptr); 
            return;
		}

		ptr = next;
	} 
	
	//tail
	list_add_tail(&rq->queuelist, &nd->queue);
    printk(KERN_EMERG "[SSTF] add %c %llu\n", direction, blk_rq_pos(rq));
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e){
	struct sstf_data *nd;
	struct elevator_queue *eq;

	/* Implementação da inicialização da fila (queue).
	 *
	 * Use como exemplo a inicialização da fila no driver noop-iosched.c
	 *
	 */

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue); //INICIALIZA A LISTA	

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	/* Implementação da finalização da fila (queue).
	 *
	 * Use como exemplo o driver noop-iosched.c
	 *
	 */
	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

/* Infrastrutura dos drivers de IO Scheduling. */
static struct elevator_type elevator_sstf = {
	.ops.sq = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

/* Inicialização do driver. */
static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

/* Finalização do driver. */
static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("Victor Putrich e Lucca");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");