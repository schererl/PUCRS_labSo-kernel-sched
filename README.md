# labSo-escalonador

# kkk eae man

## pseudocódigo
´´´
static void sstf_add_request(struct request_queue *q, struct request *rq){
	struct sstf_data *nd = q->elevator->elevator_data;
  
  lastPos = curPos; //posicao atual salva numa variavel global
	struct list_head *ptr;
  rPos = blk_rq_pos(rq)
  list_for_each(ptr, &nd->queue){
    //pega a pos
    entry = list_entry(ptr, struct request, queueList); 
    entryPos = blk_rq_pos(entry)
    
    //compara rPos com entryPos 
    //adiciona na lista
	}
}
´´´
