
#include "mindows_inner.h"
#include "mds_queue_inner.h"


/***********************************************************************************
函数功能: 创建队列.
入口参数: pstrQue: 队列指针.
返 回 值: RTN_SUCD: 创建队列成功.
          RTN_FAIL: 创建队列失败.
***********************************************************************************/
U32 MDS_QueCreate(M_QUE* pstrQue)
{
    /* 入口参数检查 */
    if(NULL == pstrQue)
    {
        return RTN_FAIL;
    }

    MDS_ChainInit(&pstrQue->strChain);

    return RTN_SUCD;
}

/***********************************************************************************
函数功能: 将一个节点加入队列.
入口参数: pstrQue: 队列指针.
          pstrQueNode: 要加入的队列节点指针.
返 回 值: RTN_SUCD: 将节点加入队列成功.
          RTN_FAIL: 将节点加入队列失败.
***********************************************************************************/
U32 MDS_QuePut(M_QUE* pstrQue, M_CHAIN* pstrQueNode)
{
    /* 入口参数检查 */
    if((NULL == pstrQue) || (NULL == pstrQueNode))
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* 将节点加入队列 */
    MDS_ChainNodeAdd(&pstrQue->strChain, pstrQueNode);

    (void)MDS_IntUnlock();

    return RTN_SUCD;
}

/***********************************************************************************
函数功能: 从队列中取出一个队列节点.
入口参数: pstrQue: 队列根节点指针.
          ppstrQueNode: 存放队列节点指针的指针.
返 回 值: RTN_SUCD: 从队列取出节点成功.
          RTN_FAIL: 从队列取出节点失败.
***********************************************************************************/
U32 MDS_QueGet(M_QUE* pstrQue, M_CHAIN** ppstrQueNode)
{
    M_CHAIN* pstrQueNode;

    /* 入口参数检查 */
    if((NULL == pstrQue) || (NULL == ppstrQueNode))
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* 从队列取出节点 */
    pstrQueNode = MDS_ChainNodeDelete(&pstrQue->strChain);

    (void)MDS_IntUnlock();

    /* 队列不为空, 可以取出节点 */
    if(NULL != pstrQueNode)
    {
        *ppstrQueNode = pstrQueNode;

        return RTN_SUCD;
    }
    else /* 队列为空, 无法取出节点 */
    {
        return RTN_FAIL;
    }
}

