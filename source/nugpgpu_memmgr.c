#include "nugpgpu_memmgr.h"

/* Linux scatter list API's - https://lwn.net/Articles/256368/ */

int allocate_object(struct nugpgpu_private *gpu_priv, 
                    struct nugpgpu_obj *obj, u32 num)
{
  int i;
  struct sg_table *st;
  struct scatterlist *sg;
  struct page *page;
  unsigned long last_pfn = 0;	/* suppress gcc warning */
  gfp_t gfp;

  printk(LOG_INFO "allocate_pages - %u\n" LOG_END, num);
  TRACE_IN

  gfp = GFP_KERNEL | __GFP_RECLAIMABLE | __GFP_ZERO ;
  gfp |= __GFP_NORETRY | __GFP_NOWARN | __GFP_NO_KSWAPD;
  gfp &= ~(__GFP_IO | __GFP_WAIT);

  st = kmalloc(sizeof(*st), GFP_KERNEL);
  if ( !st ){
      printk(LOG_ERR "could not allocate sg_table, out of memory\n" LOG_END);
      return -ENOMEM;
  }

  if (sg_alloc_table(st, num, GFP_KERNEL)){
      printk(LOG_ERR "could not allocate sg_table, out of memory\n" LOG_END);
      return -ENOMEM;
  }

  sg = st->sgl;
  st->nents = 0;

  printk(LOG_INFO "creating the scatterlist of pages\n" LOG_END);

  for (i=0; i<num; i++){
    page = alloc_pages(gfp, 0);
    if (page == NULL){
      printk(LOG_ERR "failed to allocate page with gfp\n" LOG_END);
      return -ENOMEM;
    }
    /* 
    * If 1st page or the pages are not continuos,
    * we need to add new list entry and set page.
    * If pages are continous then we can increase the
    * length of previous sg entry. 
    */
    if ( !i || (page_to_pfn(page) != last_pfn + 1) ){
      if (i)
        sg = sg_next(sg);
      st->nents++;
      sg_set_page(sg, page, PAGE_SIZE, 0);
    }else{
      sg->length += PAGE_SIZE;
    }
    last_pfn = page_to_pfn(page);
  }

  sg_mark_end(sg);
  
  printk(LOG_INFO "Marking the pages PCI_DMA_BIDIRECTIONAL\n" LOG_END);
  
  if ( !dma_map_sg(&gpu_priv->pdev->dev, 
                   st->sgl,
                   st->nents,
                   PCI_DMA_BIDIRECTIONAL) ){
    printk(LOG_ERR " Setting the pages to bidirectional pci dma failed" LOG_END);
    return -ENOSPC;
  }

  obj->pg_list = st;

  TRACE_OUT
  return 0;
}
