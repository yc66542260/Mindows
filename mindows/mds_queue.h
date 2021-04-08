
#ifndef MDS_QUEUE_H
#define MDS_QUEUE_H


typedef struct m_que    /* 队列结构 */
{
    M_CHAIN strChain;   /* 队列链表 */
}M_QUE;


/*********************************** 函数声明 *************************************/
extern U32 MDS_QueCreate(M_QUE* pstrQue);
extern U32 MDS_QuePut(M_QUE* pstrQue, M_CHAIN* pstrQueNode);
extern U32 MDS_QueGet(M_QUE* pstrQue, M_CHAIN** ppstrQueNode);


#endif

