
#ifndef MDS_USERBOOT_INNER_H
#define MDS_USERBOOT_INNER_H


/* ���жϷ����, 0x00000000~0x1FFFFFFFΪ����ϵͳ���� */
#define SWI_TASKSCHED       0x10001001  /* ������� */
#define SWI_INTENABLE       0x10002001  /* �ж�ʹ�� */
#define SWI_INTDISABLE      0x10002002  /* �жϽ�ֹ */


/*********************************** �������� *************************************/
extern void MDS_RootTask(void);
extern void MDS_IdleTask(void);


#endif

