#include "TypeMeter.h"
#include "TypeE2p.h"
#include "TypeRAM.h"
//#include "msp430x54x.h"
#include "ht6xxx_lib.h"
#include "DLT698.h"					
#include "Measure.h"
//#include "In430.h"
#include "Port.h"
#include "TypeFlash.h"
#include "Data.h"
#include "General.h"
#include "Mem.h"

#include "RsComm.h"
#include "interrupt.h"
#include "Mem.h"
#include "Time.h"
#include "Power.h"
#if ( MEASCHIP == ADE7878 )	
#include "AD7878.h"
#endif
#include "Initial.h"
//#include "Produce.h"
#include "cpucard.h"
#if ( MEASCHIP == IDT90E32 )							//12.08.29	
#include "IDT90E32.h"
#endif

/*
#define NEW_EVENT_NUM0		30		//ͨѶ��0�����¼��б����������⣩
#define NEW_EVENT_NUM1		30		//ͨѶ��1�����¼��б�������4851��
#define NEW_EVENT_NUM2		30		//ͨѶ��2�����¼��б�������4852���ز���

#define NewEventL0FirstNo			EPhCBreakAmpHour+16				//ͨ��0�����¼��б��������(1)
#define NewEventL0Recs				NewEventL0FirstNo+1*1			//ͨ��0�����¼��б���Ч��¼��(1)
#define NewEventL1FirstNo			NewEventL0FirstNo+1*2			//ͨ��1�����¼��б��������(1)
#define NewEventL1Recs				NewEventL0FirstNo+1*3			//ͨ��1�����¼��б���Ч��¼��(1)
#define NewEventL2FirstNo			NewEventL0FirstNo+1*4			//ͨ��2�����¼��б��������(1)
#define NewEventL2Recs				NewEventL0FirstNo+1*5			//ͨ��2�����¼��б���Ч��¼��(1)

#define NewEvent0List				NewEventL2Recs+1				//ͨ��0�����¼��б�(NEW_EVENT_NUM0*5)
#define NewEvent1List				NewEvent0List+NEW_EVENT_NUM0*5	//ͨ��1�����¼��б�(NEW_EVENT_NUM1*5)
#define NewEvent2List				NewEvent1List+NEW_EVENT_NUM1*5	//ͨ��2�����¼��б�(NEW_EVENT_NUM2*5)

typedef struct
{
	unsigned short ListAddr;		//�����׵�ַ
	unsigned short FirstNoAddr;		//������ŵ�ַ
	unsigned short RecsAddr;		//��Ч��¼����ַ
	unsigned short CapNumMax;		//�����������(�����ռ��������յ�����)
}NEW_EVENT_LIST_TAB;	

const NEW_EVENT_LIST_TAB New_Event_ListTab[] = 
{					
	NewEvent0List, 	NewEventL0FirstNo,	NewEventL0Recs, 	NEW_EVENT_NUM0,		//COM0
	NewEvent1List, 	NewEventL1FirstNo,	NewEventL1Recs, 	NEW_EVENT_NUM1,		//COM1
	NewEvent2List, 	NewEventL2FirstNo,	NewEventL2Recs, 	NEW_EVENT_NUM2,		//COM2																																																																																																																																										
}
*/

////BIT0��ʾ���һ���¼���¼
//typedef struct
//{
//	unsigned short StartState;		//�¼�����״̬		0��δ���� 1���ѷ���
//	unsigned short StartUpState;	//�¼������ϱ�״̬  0��δ�ϱ� 1�����ϱ�
//	unsigned short EndState;		//�¼�����״̬		0��δ���� 1���ѷ���
//	unsigned short EndUpState;		//�¼������ϱ�״̬	0��δ�ϱ� 1�����ϱ�
//}EVENT_UPSTATE;	

/*
void UpStateStringToShortType( EVENT_UPSTATE* EventUpState, unsigned char* Source )
{
	unsigned short Temp;
	
	RAM_Write( (unsigned char*)&Temp, Source, 2 );
	Temp &= 0x03FF;
	EventUpState->StartState = Temp;

	RAM_Write( (unsigned char*)&Temp, Source+1, 2 );
	Temp &= 0x0FFC;
	Temp >>= 2;
	EventUpState->StartUpState = Temp;
	
	RAM_Write( (unsigned char*)&Temp, Source+2, 2 );
	Temp &= 0x3FF0;
	Temp >>= 4;
	EventUpState->EndState = Temp;
	
	RAM_Write( (unsigned char*)&Temp, Source+3, 2 );
	Temp &= 0xFFC0;
	Temp >>= 6;
	EventUpState->EndUpState = Temp;
}	

void UpStateShortTypeToString( unsigned char* Dest, EVENT_UPSTATE* EventUpState )
{
	unsigned short Temp;
	
	RAM_Fill( Dest, 5 );
	
	RAM_Write( Dest, (unsigned char*)&EventUpState->StartState, 2 );
	*(Dest+1) &= 0x03;
	
	Temp = EventUpState->StartUpState;
	Temp <<= 2;
	*(Dest+1) |= Temp & 0xFC;
	*(Dest+2) |= ( Temp & 0x0F00 ) >> 8;
	
	Temp = EventUpState->EndState;
	Temp <<= 4;
	*(Dest+2) |= Temp & 0x00F0;
	*(Dest+3) |= (Temp & 0x3F00) >> 8;
	
	Temp = EventUpState->EndUpState;
	Temp <<= 6;
	*(Dest+3) |= Temp & 0x00C0;
	*(Dest+4) |= (Temp & 0xFF00) >> 8;
}	

unsigned short GetSignleNewEventList( unsigned char* Dest, EVENT_UPSTATE* EventUpState, unsigned short ItemNo )
{
	unsigned char Buff[6];
	unsigned char* Point;
	unsigned short Items;
	unsigned short Temp1;
	unsigned short Temp2;
	unsigned short StartState;		//�¼�����״̬		0��δ���� 1���ѷ���
	unsigned short StartUpState;	//�¼������ϱ�״̬  0��δ�ϱ� 1�����ϱ�
	unsigned short EndState;		//�¼�����״̬		0��δ���� 1���ѷ���
	unsigned short EndUpState;		//�¼������ϱ�״̬	0��δ�ϱ� 1�����ϱ�
	short i;
	unsigned char* Ptr;
		
	Ptr = Dest;	
	Point = Buff;
	StartState = EventUpState->StartState;	
	StartUpState = EventUpState->StartUpState;	
	EndState = EventUpState->EndState;	
	EndUpState = EventUpState->EndUpState;	
		
	Temp1 = StartState | EndState;
	if( Temp1 == 0 ) return 0;				//���¼�����
	Temp1 = StartState & (~StartUpState); 	//�Ƿ����ѷ�����δ�ϱ��ļ�¼
	Temp2 = EndState & (~EndUpState); 		//�Ƿ����ѻָ���δ�ϱ��ļ�¼
	Temp1 |= Temp2;							//�з������������һ��δ�ϱ�
	if( Temp1 == 0 ) return 0;				//������δ�ϱ��¼�
	
	Items = 0;
	RAM_Write( Point, (unsigned char*)DL698_Event_ProfileTab[ItemNo].OAD, 3 );			
	for( i=0;i<10;i++ )
	{
		if(( Temp1 & ByteBit[i] ) != 0 )
		{
			*Ptr = Data_structure;
			*(Ptr+1) = 2;
			*(Ptr+2) = D_OAD;
			RAM_Write( Ptr+3, Point, 3 );
//			*(Ptr+6) = i+1;
			*(Ptr+6) = 0x00;				//ֻ����1��	
			*(Ptr+7) = D_BOOLEAN;
			*(Ptr+8) = 0x01;						
			Ptr += 9;
			Items += 1;
			break;							//ֻ����1��
		}		
	}	
	
	return Items;	
}	

unsigned short GetNewEventList( short COM_No, unsigned char* Dest )
{
	unsigned short ShortBuff[6];
	EVENT_UPSTATE* EventUpState;
	unsigned char Buff[6];
	unsigned char* Point;
	unsigned short Items;
	unsigned short Len;
	unsigned char* Ptr;
	short i;
	unsigned char ReportFlag;	//�ϱ���ʶ
	
    if( COM_No >= 3 )           //�Ǳ�ͨ������
    {
        *Dest = 0x00;
        Len = 1;
        return Len;
    }  
    
	EventUpState = (EVENT_UPSTATE*)&ShortBuff;
	Point = Buff;
	*Dest = Data_array;
	Ptr = Dest+4;				//ǰ3�ֽ�Ϊ�����ֽ�Ԥ��	
	Items = 0;
	for( i=0;i<DL698_Event_ProfileSum;i++ )
	{
		E2P_RData( (unsigned char*)&ReportFlag, DL698_Event_ProfileTab[i].EReportFlagAddr, 1 ); 
		if(( ReportFlag == 0 )||( ReportFlag > 3 )) continue;		//���ϱ�
		E2P_RData( Point, DL698_Event_ProfileTab[i].UpState+COM_No*5, 5 ); 
		UpStateStringToShortType( EventUpState, Point );
		Len = GetSignleNewEventList( Ptr, EventUpState, i );
		Ptr += Len * 9;
		Items += Len;
		if( Items > 200 ) break;	//���ܳ���APDU���ȣ��˳�
	}	
	
	Len = Items * 9;
	if( Items < 128 )
	{
		*(Dest+1) = Items;
		RAM_Write( Dest+2, Dest+4, Len );	
		Len += 1;
	}
	else if( Items < 256 )
	{
		*(Dest+1) = 0x81;
		*(Dest+2) = Items;
		RAM_Write( Dest+3, Dest+4, Len );	
		Len += 2;		
	}		
	else 
	{
		*(Dest+1) = 0x82;
		*(Dest+2) = ( Items & 0xff00 ) >> 8;
		*(Dest+3) = ( Items & 0x00ff );
		Len += 3;		
	}		
	
	Len += 1;	//Data_array����
	
	return Len;
}

void AddSignleNewEvent( EVENT_UPSTATE* EventUpState, unsigned char ReportFlag, short End )
{	

	EventUpState->StartState <<= 1;	
	EventUpState->StartUpState <<= 1;	
	EventUpState->EndState <<= 1;	
	EventUpState->EndUpState <<= 1;	
	
	if( End == 0 )
	{
		if(( ReportFlag == 1 )||( ReportFlag == 3 )) EventUpState->StartState |= 0x01;
	}	 
	else 
	{
		if(( ReportFlag == 2 )||( ReportFlag == 3 )) EventUpState->EndState |= 0x01;		
	}	

	EventUpState->StartState &= 0x03FF ;	
	EventUpState->StartUpState &= 0x03FF ;	
	EventUpState->EndState &= 0x03FF ;	
	EventUpState->EndUpState &= 0x03FF ;	

	return;	
}	

unsigned short GetEventClassTabItemNo_Event_No( unsigned short Event_No )
{
	unsigned short i;
		
	for( i=0;i<DL698_Event_ProfileSum;i++ )
	{
		if( Event_No == DL698_Event_ProfileTab[i].EventNo ) break;
	}	
	return i;	
}



//End: 0���¼���ʼ 1���¼�����
void AddToNewEventList( unsigned short Event_No, short End )
{
	unsigned short ShortBuff[6];
	EVENT_UPSTATE* EventUpState;
	unsigned char Buff1[6];
	unsigned char Buff2[6];
	unsigned char* Point;
	unsigned char* Ptr;
	short i;
	unsigned char ReportFlag;	//�ϱ���ʶ
	unsigned short ItemNo;
	
	EventUpState = (EVENT_UPSTATE*)&ShortBuff;
	Point = Buff1;
	Ptr = Buff2;				

	ItemNo = GetEventClassTabItemNo_Event_No( Event_No );
	if( ItemNo == DL698_Event_ProfileSum ) return;
	E2P_RData( (unsigned char*)&ReportFlag, DL698_Event_ProfileTab[ItemNo].EReportFlagAddr, 1 ); 
	if(( ReportFlag == 0 )||( ReportFlag > 3 )) return;		//���ϱ�
	if(( ReportFlag == 1 )&&( End != 0 )) return;		//����Ϊ�����ϱ��������¼��������仯
	if(( ReportFlag == 2 )&&( End == 0 )) return;		//����Ϊ�����ϱ��������¼��������仯
	for( i=0;i<3;i++ )
	{
		E2P_RData( Point, DL698_Event_ProfileTab[ItemNo].UpState+i*5, 5 ); 
		UpStateStringToShortType( EventUpState, Point );
		AddSignleNewEvent( EventUpState, ReportFlag, End );
		UpStateShortTypeToString( Ptr, EventUpState );
		E2P_WData( DL698_Event_ProfileTab[ItemNo].UpState+i*5, Ptr, 5 ); 
	}
	
}

//��ע��Ӧ���ϱ�λ
//void DelSignleNewEvent( EVENT_UPSTATE* EventUpState, unsigned char ReportNo, unsigned char ReportFlag )
void DelSignleNewEvent( EVENT_UPSTATE* EventUpState, unsigned char ReportFlag )
{	
	int i;
	
	for( i=0;i<10;i++ )
	{
		if(( EventUpState->StartState & ByteBit[i] ) != 0 )
		{
			if(( ReportFlag == 1 )||( ReportFlag == 3 )) EventUpState->StartUpState |= ByteBit[i]; 	
		}	
		
		if(( EventUpState->EndState & ByteBit[i] ) != 0 )
		{
			if(( ReportFlag == 2 )||( ReportFlag == 3 )) EventUpState->EndUpState |= ByteBit[i]; 	
		}	
	}	
	return;	
}	

//��ע��Ӧ���ϱ�λ���Դ������¼��б���ɾ��
void DelFromNewEventList( short COM_No, unsigned char* OAD )
{
	unsigned short ShortBuff[6];
	EVENT_UPSTATE* EventUpState;
	unsigned char Buff1[6];
	unsigned char Buff2[6];
	unsigned char* Point;
	unsigned char* Ptr;
	unsigned char ReportFlag;	//�ϱ���ʶ
	unsigned short ItemNo;
	unsigned short UpStateAddr;
	
	EventUpState = (EVENT_UPSTATE*)&ShortBuff;
	Point = Buff1;
	Ptr = Buff2;					

//	if(( *(OAD+3) == 0 )||( *(OAD+3) > 10 )) return;		//������¼��Χ
	if( *(OAD+3) != 0 ) return;		//������¼��Χ
	ItemNo = GetEventClassTabItemNo_OAD3( OAD );
	if( ItemNo == DL698_Event_ProfileSum ) return;
	E2P_RData( (unsigned char*)&ReportFlag, DL698_Event_ProfileTab[ItemNo].EReportFlagAddr, 1 ); 
	if(( ReportFlag == 0 )||( ReportFlag > 3 )) return;		//���ϱ�������

	UpStateAddr = DL698_Event_ProfileTab[ItemNo].UpState+COM_No*5;
	E2P_RData( Point, UpStateAddr, 5 ); 
	UpStateStringToShortType( EventUpState, Point );
//	DelSignleNewEvent( EventUpState, *(OAD+3), ReportFlag );
	DelSignleNewEvent( EventUpState, ReportFlag );
	UpStateShortTypeToString( Ptr, EventUpState );
	if( Data_Comp( Point, Ptr, 5 ) != 0 )					//�в�����Ҫ����
	{
		E2P_WData( UpStateAddr, Ptr, 5 ); 
	}	
}	

//Type: 0:ȫ�������1�����˵�������¼��2�����˵�������¼���¼���¼
void InitialNewEventList( short Type )
{
	if( Type == 0 ) Clr_E2PData( PhFail_UpState, 15, DL698_Event_ProfileSum );	
	else if( Type == 1 ) 
	{
		Clr_E2PData( PhFail_UpState, 15, 49 );
		Clr_E2PData( MDClr_UpState, 15, DL698_Event_ProfileSum-50 );
	}	
	else if( Type == 2 ) 
	{
		Clr_E2PData( PhFail_UpState, 15, 49 );
		Clr_E2PData( MDClr_UpState, 15, 1 );
		Clr_E2PData( TimeSet_UpState, 15, DL698_Event_ProfileSum-52 );
	}	
}	
*/
//================================================================================================
unsigned short GetEventClassTabItemNo_Event_No( unsigned short Event_No )
{
	unsigned short i;
		
	for( i=0;i<DL698_Event_ProfileSum;i++ )
	{
		if( Event_No == DL698_Event_ProfileTab[i].EventNo ) break;
	}	
	return i;	
}

unsigned short GetEventClassTabItemNo_OAD3( unsigned char* OAD )		//17.05.19
{
	unsigned short i;
		
	for( i=0;i<DL698_Event_ProfileSum;i++ )
	{
		if( Data_Comp( OAD, (unsigned char*)DL698_Event_ProfileTab[i].OAD, 3 ) == 0 ) break;				//���Ҷ�Ӧ��OBIS��
	}	
	return i;	
}

//Type: 0:ȫ�������1�����˵�������¼��2�����˵�������¼���¼���¼
void InitialNewReportEventList( short Type )							//17.05.19
{
	unsigned char Buff[NEW_REPORT_EVENT_NUM];
	unsigned char* Ptr;
	unsigned char EventRecs;
	unsigned char Recs;
	unsigned short i,j;
	
	Ptr = Buff;
	
	for( i=0;i<3;i++ )
	{
		Recs = 0;
		if( Type != 0 )
		{	
			E2P_RData( (unsigned char*)&EventRecs, NewReportEventL0Recs+i, 1 );	
			if( EventRecs > NEW_REPORT_EVENT_NUM ) EventRecs = 0;
			E2P_RData( Ptr, NewReportEvent0List+i*NEW_REPORT_EVENT_NUM, EventRecs );
			for( j=0;j<EventRecs;j++ )
			{
				if( Type == 1 )
				{
					if( *(Ptr+j) == ECClr_No ) 
					{
						RAM_Write( Ptr, Ptr+j, EventRecs - j );	
						Recs = 1;
						break;
					}	
				}		
				else if( Type == 2 ) 
				{
					if(( *(Ptr+j) == ECClr_No )||( *(Ptr+j) == EventClr_No )) 
					{
						RAM_Write( Ptr, Ptr+j, EventRecs - j );	
						EventRecs -= j;
						j = 1;
						Recs += 1;
						if( Recs >= 2 ) break;
					}	
				}	
			}		
		}	
		E2P_WData( NewReportEventL0Recs+i, (unsigned char*)&Recs, 1 );	
		E2P_WData( NewReportEvent0List+i*NEW_REPORT_EVENT_NUM, Ptr, Recs );
#if ( CarryComm == YesCheck )			//�ز�ͨ��
		if( i == 2 ) Para.RNewReportEventCarryRecs = Recs;						//17.05.20
#endif
	}	
	InitialClearFollowOADDelay();							//17.05.20
}	

//�����¼��б���ɾ�������¼�
void DelCurFromNewReportEventList( short COM_No )		//17.05.20
{
	unsigned char Buff[NEW_REPORT_EVENT_NUM];
	unsigned char* Ptr;
	unsigned short RecsAddr;
	unsigned short ListAddr;
	unsigned char EventRecs;
	
	Ptr = Buff;
	
	if( COM_No > 3 ) return;
	RecsAddr = NewReportEventL0Recs+COM_No;
	ListAddr = NewReportEvent0List+COM_No*NEW_REPORT_EVENT_NUM;
	E2P_RData( (unsigned char*)&EventRecs, RecsAddr, 1 );	
	if(( EventRecs > NEW_REPORT_EVENT_NUM )||( EventRecs == 0 )) 
	{
		EventRecs = 0;
	}
	else
	{	
		E2P_RData( Ptr, ListAddr, EventRecs );
		RAM_Write( Ptr, Ptr+1, EventRecs - 1 );
		
		EventRecs--;
		E2P_WData( ListAddr, Ptr, EventRecs );	
		E2P_WData( RecsAddr, (unsigned char*)&EventRecs, 1 );	
	}	
#if ( CarryComm == YesCheck )			//�ز�ͨ��
	if( COM_No == 2 ) Para.RNewReportEventCarryRecs = EventRecs;		//17.05.20
#endif

	return;
}	


//��ע��Ӧ���ϱ�λ���Դ������¼��б���ɾ��
void DelFromNewReportEventList( short COM_No, unsigned char* OAD )		//17.05.19
{
	unsigned char Buff[NEW_REPORT_EVENT_NUM];
	unsigned char* Ptr;
	unsigned short ItemNo;
	unsigned char EventNo;
	unsigned short RecsAddr;
	unsigned short ListAddr;
	unsigned char EventRecs;
	unsigned short i;
	
	Ptr = Buff;
	
//	if(( *(OAD+3) == 0 )||( *(OAD+3) > 10 )) return;		//������¼��Χ
	if( *(OAD+3) != 0 ) return;		//������¼��Χ
	if( COM_No > 3 ) return;
	ItemNo = GetEventClassTabItemNo_OAD3( OAD );
	if( ItemNo == DL698_Event_ProfileSum ) return;
//	E2P_RData( (unsigned char*)&ReportFlag, DL698_Event_ProfileTab[ItemNo].EReportFlagAddr, 1 ); 
	EventNo = DL698_Event_ProfileTab[ItemNo].EventNo;
//	if(( ReportFlag == 0 )||( ReportFlag > 3 )) return;		//���ϱ�������

	RecsAddr = NewReportEventL0Recs+COM_No;
	ListAddr = NewReportEvent0List+COM_No*NEW_REPORT_EVENT_NUM;
	E2P_RData( (unsigned char*)&EventRecs, RecsAddr, 1 );	
	if(( EventRecs > NEW_REPORT_EVENT_NUM )||( EventRecs == 0))
	{
		EventRecs = 0;
	}
	else
	{	
		E2P_RData( Ptr, ListAddr, EventRecs );
		for( i=0;i<EventRecs;i++ )
		{
			if( *(Ptr+i) == EventNo )
			{
				RAM_Write( Ptr+i, Ptr+i+1, EventRecs - i - 1 );
				break; 
			}				
		}	
		
		if( i != EventRecs )
		{
			EventRecs--;
			E2P_WData( ListAddr, Ptr, EventRecs );	
			E2P_WData( RecsAddr, (unsigned char*)&EventRecs, 1 );	
		}
	}
#if ( CarryComm == YesCheck )			//�ز�ͨ��                                
	if( COM_No == 2 ) Para.RNewReportEventCarryRecs = EventRecs;		//17.05.20
#endif                                                                            
	return;			
}	

//End: 0���¼���ʼ 1���¼�����
void AddToNewReportEventList( unsigned short Event_No, short End )			//17.05.19
{
	unsigned char Buff[NEW_REPORT_EVENT_NUM];
	unsigned char* Ptr;
	unsigned char ReportFlag;	//�ϱ���ʶ
	unsigned short ItemNo;
	unsigned char EventNo;
	unsigned short RecsAddr;
	unsigned short ListAddr;
	unsigned char EventRecs;
	unsigned short i,j;
	
	Ptr = Buff;
	
	ItemNo = GetEventClassTabItemNo_Event_No( Event_No );
	if( ItemNo == DL698_Event_ProfileSum ) return;
	E2P_RData( (unsigned char*)&ReportFlag, DL698_Event_ProfileTab[ItemNo].EReportFlagAddr, 1 ); 
	if(( ReportFlag == 0 )||( ReportFlag > 3 )) return;		//���ϱ�
	if(( ReportFlag == 1 )&&( End != 0 )) return;		//����Ϊ�����ϱ��������¼��������仯
	if(( ReportFlag == 2 )&&( End == 0 )) return;		//����Ϊ�����ϱ��������¼��������仯

	EventNo = DL698_Event_ProfileTab[ItemNo].EventNo;
	for( i=0;i<3;i++ )
	{	
		RecsAddr = NewReportEventL0Recs+i;
		ListAddr = NewReportEvent0List+i*NEW_REPORT_EVENT_NUM;
		E2P_RData( (unsigned char*)&EventRecs, RecsAddr, 1 );	
		if( EventRecs > NEW_REPORT_EVENT_NUM ) EventRecs = 0;
		E2P_RData( Ptr, ListAddr, EventRecs );
		for( j=0;j<EventRecs;j++ )
		{
			if( *(Ptr+j) == EventNo )
			{
				RAM_Write( Ptr+j, Ptr+j+1, EventRecs - j - 1 );
				EventRecs--;
			}				
		}	
		RAM_Write_Rev( Ptr+1, Ptr, EventRecs );
		EventRecs++;
		*Ptr = EventNo;
		E2P_WData( ListAddr, Ptr, EventRecs );	
		E2P_WData( RecsAddr, (unsigned char*)&EventRecs, 1 );	
#if ( CarryComm == YesCheck )			//�ز�ͨ��                                
		if( i == 2 ) Para.RNewReportEventCarryRecs = EventRecs;		//17.05.20
#endif                                                                            
	}		
	InitialClearFollowOADDelay();							//17.05.20
}

unsigned short GetSignleNewReportEventList( unsigned char* Dest, unsigned short Event_No )			//17.05.19
{
	unsigned char ReportFlag;	//�ϱ���ʶ
	unsigned short ItemNo;
	unsigned short Len;
	
	Len = 1;
	*Dest = 0;
	ItemNo = GetEventClassTabItemNo_Event_No( Event_No );
	if( ItemNo == DL698_Event_ProfileSum ) return Len;
	E2P_RData( (unsigned char*)&ReportFlag, DL698_Event_ProfileTab[ItemNo].EReportFlagAddr, 1 ); 
//	if(( ReportFlag == 0 )||( ReportFlag > 3 )) return;		//���ϱ�

	*Dest = D_OAD;
	RAM_Write( Dest+1, (unsigned char*)DL698_Event_ProfileTab[ItemNo].OAD, 4 );
	*(Dest+4) = 0x01;
	Len = 5;
	
	return Len;
	
}	

unsigned short GetNewReportEventList( short COM_No, unsigned char* Dest, unsigned char* OAD )	//17.05.19
{
	unsigned char Buff[NEW_REPORT_EVENT_NUM];
	unsigned char* Point;
	unsigned short Len;
	unsigned short Length;
	unsigned char* Ptr;
	unsigned short RecsAddr;
	unsigned short ListAddr;
	unsigned char EventRecs;
	unsigned short i;
	
    Ptr = Buff;
    
    *Dest = 0x00;
    Len = 1;
    if( COM_No >= 3 ) return Len;          //�Ǳ�ͨ������
    
	RecsAddr = NewReportEventL0Recs+COM_No;
	ListAddr = NewReportEvent0List+COM_No*NEW_REPORT_EVENT_NUM;
	E2P_RData( (unsigned char*)&EventRecs, RecsAddr, 1 );	
	if( EventRecs > NEW_REPORT_EVENT_NUM ) 
	{
		EventRecs = 0;
		return Len;
	}	
	if( *(OAD+3) > EventRecs ) return Len;	//������Χ
	E2P_RData( Ptr, ListAddr, EventRecs );
	if( *(OAD+3) == 0 )
	{
		*Dest = Data_array;
		*(Dest+1) = EventRecs;
		Len = 2;
		Point = Dest+2;
		for( i=0;i<EventRecs;i++ )
		{
			Length = GetSignleNewReportEventList( Point, (unsigned short)*(Ptr+i) );
			Point += Length;
			Len += Length;
		}	
	}
	else 
	{
		Len = GetSignleNewReportEventList( Dest, (unsigned short)*(Ptr+(*(OAD+3)-1)) );
	}		

	return Len;
}

unsigned short GetNewReportEventListAttribute3( unsigned char* Dest )		//17.05.19
{
	unsigned short Items;
	unsigned short Len;
	unsigned char* Ptr;
	unsigned short Class_Id;		//���
	unsigned short i;
	unsigned char ReportFlag;	//�ϱ���ʶ
	 
	*Dest = Data_array;
	Ptr = Dest+2;				//ǰ3�ֽ�Ϊ�����ֽ�Ԥ��	
	Items = 0;
	Len = 2;
	for( i=0;i<DL698_Event_ProfileSum;i++ )
	{
		E2P_RData( (unsigned char*)&ReportFlag, DL698_Event_ProfileTab[i].EReportFlagAddr, 1 ); 
		Class_Id = DL698_Event_ProfileTab[i].Class_Id;
		if(( ReportFlag == 0 )||( ReportFlag > 3 )) 
		{
			if( Class_Id == Class_id_PhaseEvent ) i += 3;		//�������
			continue;											//���ϱ�
		}	
		*Ptr = D_OI;
		RAM_Write( Ptr+1, (unsigned char*)DL698_Event_ProfileTab[i].OAD, 2 );
		Items += 1;
		Len += 3;
		Ptr += 3;
		if( Class_Id == Class_id_PhaseEvent ) i += 3;			//�������
	}	
	*(Dest+1) = Items;
	
	return Len;
}

void CheckClearFollowOADDelay( void )							//17.05.20
{
	unsigned char* Ptr;
	unsigned char* Check;
	unsigned short i;
	
	Ptr = (unsigned char*)&HComm.ClearFollowOADDelay0;
	Check = (unsigned char*)&HComm.ClearFollowOADDelayCheck0;
	for( i=0;i<3;i++ )
	{
		if( *Check != ChkNum( Ptr, 2 ) )
		{
			*Ptr = 0;
			*(Ptr+1) = 0;
			*Check = 0xA5;			
		}
		Ptr += 2;
		Check += 1;				
	}	
}	

void InitialClearFollowOADDelay( void )							//17.05.20
{
	unsigned short* ShortPtr;
	unsigned char* Check;
	unsigned short i;
	
	ShortPtr = &HComm.ClearFollowOADDelay0;
	Check = (unsigned char*)&HComm.ClearFollowOADDelayCheck0;
	for( i=0;i<3;i++ )
	{
		*ShortPtr = 0;
		*Check = 0xA5;			
		ShortPtr += 1;
		Check += 1;				
	}	
}	

void DecClearFollowOADDelay( void )								//17.05.20
{
	unsigned char* Ptr;
	unsigned short* ShortPtr;
	unsigned char* Check;
	unsigned short i;
	
	Ptr = (unsigned char*)&HComm.ClearFollowOADDelay0;
	Check = (unsigned char*)&HComm.ClearFollowOADDelayCheck0;
	ShortPtr = &HComm.ClearFollowOADDelay0;
	for( i=0;i<3;i++ )
	{
		if( *Check != ChkNum( Ptr, 2 ) )
		{
			*Ptr = 0;
			*(Ptr+1) = 0;
			*Check = 0xA5;			
		}
		else
		{
			if( *ShortPtr != 0 )
			{
				*ShortPtr -= 1;
				if( *ShortPtr == 0 )
				{
					DelCurFromNewReportEventList( i );					
				}		
				*Check = ChkNum( Ptr, 2 );
			}		
		}	
		Ptr += 2;
		Check += 1;	
		ShortPtr += 1;			
	}	
}

//����30������ʱ��ʱ
void StartClearFollowOADDelay( short COM_No )							//17.05.20
{
	unsigned char* Ptr;
	unsigned short* ShortPtr;
	unsigned char* Check;
	
	Ptr = (unsigned char*)&HComm.ClearFollowOADDelay0;
    Ptr += COM_No*2;
	ShortPtr = &HComm.ClearFollowOADDelay0;
    ShortPtr += COM_No;
	Check = (unsigned char*)&HComm.ClearFollowOADDelayCheck0;
    Check += COM_No;
	*ShortPtr = 1800;			//30����
	*Check = ChkNum( Ptr, 2 );
}	







