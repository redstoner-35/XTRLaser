#ifndef _FASTOP_
#define _FASTOP_

//�����
#define abs(x) x>0?x:x*-1  //��ĳ���ľ���ֵ

//�ж��Ƿ����ĳ���ĵĿ�ݷ�ʽ
#define IsLargerThanThreeU16(x) (x&0xFFFC) //λ�����ж�16bit�޷��������Ƿ����3
#define IsLargerThanThreeU8(x) (x&0xFC) //λ�����ж�8bit�޷��������Ƿ����3
#define IsLargerThanOneU16(x) (x&0xFFFE) //λ�����ж�16bit�޷��������Ƿ����1
#define IsLargerThanOneU8(x) (x&0xFE) //λ�����ж�8bit�޷��������Ƿ����1

//�ж��Ƿ�С��0�Ŀ�ݷ�ʽ
#define IsNegative16(x) (x&0x8000) //ʹ��ȡ����λ�����ж�16bit�з��������Ƿ�С��0����ֱ�ӱȽ�ʡ�ռ䣩
#define IsNegative8(x) (x&0x80)		//ʹ��ȡ����λ�����ж�8bit�з��������Ƿ�С��0����ֱ�ӱȽ�ʡ�ռ䣩

#endif
