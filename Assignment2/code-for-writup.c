// This is the original function from noop-iosched.c:
static void noop_add_request(struct request_queue *q, struct request *rq)
{
    struct noop_data *nd = q->elevator->elevator_data;

    list_add_tail(&rq->queuelist, &nd->queue);
}

//This is the new function in sstf-iosched.c:
static void sstf_add_request(struct request_queue *q, struct request *rq)
{
    struct sstf_data *nd = q->elevator->elevator_data;
    /* While sector of new request is larger than current sector check next request in queue */
    while (blk_rq_pos(rq) > blk_rq_pos(&nd->queue)) {
   	 nd = nd->queue.next;
   	 /* Stop when the next sector is larger than the new request */
   	 if (blk_rq_pos(&nd->queue) < blk_rq_pos(nd->queue.prev))
   		 break;
    }
    /* Add new request after the last smaller sector */
    list_add_tail(&rq->queuelist, &nd->queue);
}
