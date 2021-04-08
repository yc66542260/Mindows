
#ifndef MDS_QUEUE_H
#define MDS_QUEUE_H


typedef struct m_que    /* ���нṹ */
{
    M_CHAIN strChain;   /* �������� */
}M_QUE;


/*********************************** �������� *************************************/
extern U32 MDS_QueCreate(M_QUE* pstrQue);
extern U32 MDS_QuePut(M_QUE* pstrQue, M_CHAIN* pstrQueNode);
extern U32 MDS_QueGet(M_QUE* pstrQue, M_CHAIN** ppstrQueNode);


#endif

