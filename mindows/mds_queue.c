
#include "mindows_inner.h"
#include "mds_queue_inner.h"


/***********************************************************************************
��������: ��������.
��ڲ���: pstrQue: ����ָ��.
�� �� ֵ: RTN_SUCD: �������гɹ�.
          RTN_FAIL: ��������ʧ��.
***********************************************************************************/
U32 MDS_QueCreate(M_QUE* pstrQue)
{
    /* ��ڲ������ */
    if(NULL == pstrQue)
    {
        return RTN_FAIL;
    }

    MDS_ChainInit(&pstrQue->strChain);

    return RTN_SUCD;
}

/***********************************************************************************
��������: ��һ���ڵ�������.
��ڲ���: pstrQue: ����ָ��.
          pstrQueNode: Ҫ����Ķ��нڵ�ָ��.
�� �� ֵ: RTN_SUCD: ���ڵ������гɹ�.
          RTN_FAIL: ���ڵ�������ʧ��.
***********************************************************************************/
U32 MDS_QuePut(M_QUE* pstrQue, M_CHAIN* pstrQueNode)
{
    /* ��ڲ������ */
    if((NULL == pstrQue) || (NULL == pstrQueNode))
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* ���ڵ������� */
    MDS_ChainNodeAdd(&pstrQue->strChain, pstrQueNode);

    (void)MDS_IntUnlock();

    return RTN_SUCD;
}

/***********************************************************************************
��������: �Ӷ�����ȡ��һ�����нڵ�.
��ڲ���: pstrQue: ���и��ڵ�ָ��.
          ppstrQueNode: ��Ŷ��нڵ�ָ���ָ��.
�� �� ֵ: RTN_SUCD: �Ӷ���ȡ���ڵ�ɹ�.
          RTN_FAIL: �Ӷ���ȡ���ڵ�ʧ��.
***********************************************************************************/
U32 MDS_QueGet(M_QUE* pstrQue, M_CHAIN** ppstrQueNode)
{
    M_CHAIN* pstrQueNode;

    /* ��ڲ������ */
    if((NULL == pstrQue) || (NULL == ppstrQueNode))
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* �Ӷ���ȡ���ڵ� */
    pstrQueNode = MDS_ChainNodeDelete(&pstrQue->strChain);

    (void)MDS_IntUnlock();

    /* ���в�Ϊ��, ����ȡ���ڵ� */
    if(NULL != pstrQueNode)
    {
        *ppstrQueNode = pstrQueNode;

        return RTN_SUCD;
    }
    else /* ����Ϊ��, �޷�ȡ���ڵ� */
    {
        return RTN_FAIL;
    }
}

