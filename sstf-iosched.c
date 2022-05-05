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
	struct request; //usar aqui blk_rq_pos
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static long long int abslong(long long int in){
	if (in < 0){
		return in *-1;	
	}
	return in;
}

/* Esta função despacha o próximo bloco a ser lido. */
static int sstf_dispatch(struct request_queue *q, int force){
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	struct request *rq;

	/* Aqui deve-se retirar uma requisição da fila e enviá-la para processamento.
	 * Use como exemplo o driver noop-iosched.c. Veja como a requisição é tratada.
	 *
	 * Antes de retornar da função, imprima o sector que foi atendido.
	 */

	
	

	// * SEARCH FOR THE CLOSES REQUEST OF THE LAST SEEK POSITION
	// for now we have to search for the closes request, maybe its better to order it in add_request
	// se tiver ordenado pela posicao absoluta de seek, da ir iterando enquanto encontra uma distancia entre
	// o current seek e o old seek, no momento que deixar de diminuir a distancia, retorna com o valor anterior.
	struct request *closest_rq = NULL; //= kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	struct list_head *closest_head = NULL;
	long long unsigned int closest_dist = 23372036854775807;
	struct list_head *ptr;
	list_for_each(ptr, &nd->queue){
		rq = list_entry(ptr, struct request, queuelist); 
		long long unsigned int new_pos = blk_rq_pos(rq);
		//printk(KERN_EMERG "pos %llu, last pos %llu\n", new_pos, last_pos);
		if ( abslong(new_pos - last_pos) < closest_dist ) {
			closest_rq = rq;
			closest_head = ptr;
			closest_dist = abslong(new_pos - last_pos); 
		}
	}
	
	if(closest_rq){
		last_pos = blk_rq_pos(closest_rq);
		list_del(closest_head);
		elv_dispatch_sort(q, closest_rq);
		
		printk(KERN_EMERG "[SSTF] dsp %c %llu\n", direction, blk_rq_pos(closest_rq));
		return 1;
	}

	// if (rq) {
	// 	last_pos = blk_rq_pos(rq);
	// 	list_del_init(&rq->queuelist);
	// 	elv_dispatch_sort(q, rq);
	// 	printk(KERN_EMERG "[SSTF] dsp %c %llu\n", direction, blk_rq_pos(rq));

	// 	return 1;
	// }
	return 0;
}

//estratégia: adicionar a requisição de maneira ordenada pelo setor
static void sstf_add_request(struct request_queue *q, struct request *rq){
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';

	/* Aqui deve-se adicionar uma requisição na fila do driver.
	 * Use como exemplo o driver noop-iosched.c
	 *
	 * Antes de retornar da função, imprima o sector que foi adicionado na lista.
	 */
	
	// * IDEIA: ordernar lista pelo blk_rq_pos absoluto


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

MODULE_AUTHOR("Miguel Xavier");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
