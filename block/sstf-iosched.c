/*
 * elevator look
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data {
	struct list_head queue;
        sector_t head_pos;
        int direction; //wisdom from TA
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
	struct sstf_data *nd = q->elevator->elevator_data;
        printk("In dispatch\n");
	if (!list_empty(&nd->queue)) { //if nothing in queue, why bother
		struct request *cur, *prev, *next;
		prev = list_entry(nd->queue.prev, struct request, queuelist);
		next = list_entry(nd->queue.next, struct request, queuelist);
		if(prev == next) {
                  printk("First or only 1 req\n");
                  cur = next; //only 1 thing to do, so do it
                }else{
                  printk("More than 1\n");
                  if(nd->direction == 1) {
                    printk("Thinking forward\n");
                    if(next->__sector > nd->head_pos)
                      cur = next; //if you can keep going, go
                    else{
                      nd->direction = -1;//turn around
                      cur = prev;
                    }
                  }else{
                    printk("Thinking backwards\n");
                    if(prev->__sector < nd->head_pos)
                      cur = prev; //go go go go
                    else{
                          nd->direction = 1;//turn around
                          cur = next;
                    }
                  }
                }
                list_del_init(&cur->queuelist);
                nd->head_pos = blk_rq_pos(cur) + blk_rq_sectors(cur); //update current head pos
                elv_dispatch_add_tail(q, cur);      
                printk("Going to sec %llu\n",(unsigned long long)cur->__sector);
                printk("Which was in the %i direction\n", nd->direction);
		return 1;
	}
	return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
        struct request *next, *prev;  
	struct sstf_data *nd = q->elevator->elevator_data;
        printk("In add\n");
        if(list_empty(&nd->queue)){
            printk("First item in list added\n");
            list_add(&rq->queuelist, &nd->queue);
        }else{
            next = list_entry(nd->queue.next, struct request, queuelist);
            prev = list_entry(nd->queue.prev, struct request, queuelist);
            while(blk_rq_pos(rq) > blk_rq_pos(next)){
                  printk("Looping\n");
                  next = list_entry(next->queuelist.next, struct request, queuelist);
                  prev = list_entry(prev->queuelist.prev, struct request, queuelist);
                  //cycle until the current request location is less than the next
                  //thus doing insertion sort as you never didn't do that
            }
            list_add(&rq->queuelist, &prev->queuelist);//add new guy to head of prev
          }
        printk("Added a sector, num %llu\n", (unsigned long long)rq->__sector);
        return;
}

static struct request *
sstf_former_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
sstf_latter_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *nd;
	struct elevator_queue *eq;
        eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
        nd->head_pos = 0; //the head starts at 0 for our purposes
        eq->elevator_data = nd;
	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_former_req_fn		= sstf_former_request,
		.elevator_latter_req_fn		= sstf_latter_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "SSTF",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("OS2 Group 11-3");
MODULE_LICENSE("Who cares");
MODULE_DESCRIPTION("SSTF I/O scheduler");
