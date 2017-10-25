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

//#include "RsComm.h"
#include "interrupt.h"
#include "Mem.h"
#include "Time.h"
#include "Power.h"
#if ( MEASCHIP == ADE7878 )	
#include "AD7878.h"
#endif
#include "Initial.h"
//#include "Produce.h"
#if ( MEASCHIP == IDT90E32 )							//12.08.29	
#include "IDT90E32.h"
#endif

//����ָ������
short Erase_Sector(unsigned short nDest)
{
	unsigned long Value;
	
	Value = nDest;
	Value *= FLASH_SECTOR_SIZE;

	return BlockErase( Value );
}

//��ʽ��FAT��
void File_Format( void )
{
	unsigned short i;
	
	for( i=FAT_SECTOR;i<DATA_START_SECTOR;i++)
	{
		Erase_Sector(i);		
	}	
}	

//FBuff.Buff	������Buff����̬������ͨ��4096�ֽ�

//��ʣ��������������
//RemainStartSectorʣ����еĵ�һ��������ַ
unsigned short GetRemainSectors( unsigned short* RemainStartSector )
{
	unsigned short i,j;
	unsigned long Fls_Src;
	unsigned short CurSector;
	unsigned short IdleSector;
	unsigned char* Ptr;
	
	*RemainStartSector = 0xFFFF;
	
	CurSector = 0;
	IdleSector = 0;
	for( i=FAT_INDEX_START_SECTOR;i<FAT_INDEX_END_SECTOR;i++ )
	{
		Fls_Src = i;
		Fls_Src *= FLASH_SECTOR_SIZE;
		Read_Flash( FBuff.Buff, Fls_Src, 4096 );

		Ptr = FBuff.Buff;
		for( j=0;j<FAT_SECTOR_INDEXS;j++ )
		{
			if(( *Ptr == 0xFF )&&( *(Ptr+1) == 0xFF )&&( *(Ptr+2) == 0xFF ))
			{
				if( *RemainStartSector == 0xFFFF ) *RemainStartSector = CurSector;
				IdleSector++;				
			}
			Ptr += 3;		
			CurSector++;
			if( CurSector >= ( FLASH_SECTOR_COUNT - DATA_START_SECTOR ) ) return IdleSector; 		
		}	
	}
    
    return IdleSector;
}	

/*
//�ӵ�ǰ������ʼ������һ�����е�������ַ
unsigned short GetNextRemainSector( unsigned short StartSector )
{
	unsigned char Buff[6];
	unsigned short i,j;
	unsigned long Fls_Src;
	unsigned short IdleSector;
	unsigned char* Ptr;
	unsigned short StartIndexSector;

	Ptr = Buff;
	
	IdleSector = StartSector;
	StartIndexSector = StartSector / FAT_SECTOR_INDEXS; 			//���ܲ��������ĵ�һҳ
	StartSector = StartSector % FAT_SECTOR_INDEXS;
	
	Fls_Src = (unsigned long)(StartIndexSector+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;
	Fls_Src += StartSector*3;
	for( i=StartIndexSector;i<(FAT_INDEX_END_SECTOR-FAT_INDEX_START_SECTOR);i++ )
	{
//		Fls_Src = (unsigned long)(i+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;	
		for( j=StartSector;j<FAT_SECTOR_INDEXS;j++ )
		{		
			Read_Flash( Ptr, Fls_Src, 3 );
			if(( *Ptr == 0xFF )&&( *(Ptr+1) == 0xFF )&&( *(Ptr+2) == 0xFF ))
			{
				return IdleSector;
			}		
			IdleSector++;
			if( IdleSector >= ( FLASH_SECTOR_COUNT - DATA_START_SECTOR ) ) return 0xFFFF; 		//������Ч��Χ
			Fls_Src += 3;			
		}	
		StartSector = 0;
		Fls_Src = (unsigned long)(i+1+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;
	}	
	return 0xFFFF; 		//������Ч��Χ
}	
*/

//�ӵ�ǰ������ʼ������һ�����е�������ַ
unsigned short GetNextRemainSector( unsigned short StartSector )
{
	unsigned char Buff[6];
	unsigned short i,j;
	unsigned long Fls_Src;
	unsigned short IdleSector;
	unsigned char* Ptr;
	unsigned short StartIndexSector;

	Ptr = Buff;
	
	IdleSector = StartSector;
	StartIndexSector = StartSector / FAT_SECTOR_INDEXS; 			//���ܲ��������ĵ�һҳ
	StartSector = StartSector % FAT_SECTOR_INDEXS;
	
	if( StartSector == 0 )											//��ͷ����ʱ�ȶ���ҳ���ң�����߲����ٶ�
	{
		Fls_Src = (unsigned long)(StartIndexSector+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;
		Read_Flash( FBuff.Buff, Fls_Src, 4096 );
		Ptr = FBuff.Buff;
		for( i=StartIndexSector;i<(FAT_INDEX_END_SECTOR-FAT_INDEX_START_SECTOR);i++ )
		{
			for( j=StartSector;j<FAT_SECTOR_INDEXS;j++ )
			{		
				if(( *Ptr == 0xFF )&&( *(Ptr+1) == 0xFF )&&( *(Ptr+2) == 0xFF ))
				{
					return IdleSector;
				}		
				IdleSector++;
				if( IdleSector >= ( FLASH_SECTOR_COUNT - DATA_START_SECTOR ) ) return 0xFFFF; 		//������Ч��Χ
				Ptr += 3;			
			}	
			StartSector = 0;
			Fls_Src += 4096;
			Read_Flash( FBuff.Buff, Fls_Src, 4096 );
			Ptr = FBuff.Buff;
		}		
	}	
	else
	{
		Fls_Src = (unsigned long)(StartIndexSector+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;
		Fls_Src += StartSector*3;
		for( i=StartIndexSector;i<(FAT_INDEX_END_SECTOR-FAT_INDEX_START_SECTOR);i++ )
		{
//			Fls_Src = (unsigned long)(i+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;	
			for( j=StartSector;j<FAT_SECTOR_INDEXS;j++ )
			{		
				Read_Flash( Ptr, Fls_Src, 3 );
				if(( *Ptr == 0xFF )&&( *(Ptr+1) == 0xFF )&&( *(Ptr+2) == 0xFF ))
				{
					return IdleSector;
				}		
				IdleSector++;
				if( IdleSector >= ( FLASH_SECTOR_COUNT - DATA_START_SECTOR ) ) return 0xFFFF; 		//������Ч��Χ
				Fls_Src += 3;			
			}	
			StartSector = 0;
			Fls_Src = (unsigned long)(i+1+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;
		}
	}		
	return 0xFFFF; 		//������Ч��Χ
}	



//����һ�����е���������д�뵱ǰ��������
void WriteNextSectorNo( unsigned short CurSector, unsigned short NextSector )
{
	unsigned char Buff[6];
//	unsigned short i,j;
	unsigned long Fls_Src;
	unsigned short IdleSector;
	unsigned char* Ptr;
	unsigned short IndexPages;

	Ptr = Buff;
	
	IndexPages = CurSector / FAT_SECTOR_INDEXS; 			//���ܲ��������ĵ�һҳ
	IdleSector = CurSector % FAT_SECTOR_INDEXS;
	
	Fls_Src = (unsigned long)((IndexPages+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE);			//�����ӵڶ�ҳ��ʼ
	Fls_Src += IdleSector*3;
	RAM_Write( Ptr, (unsigned char*)&NextSector, 2 );
	*(Ptr+2) = ChkNum( Ptr, 2 );
	Write_Flash_Direct( Fls_Src, Ptr, 3 );
	Write_Flash_Direct( Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, Ptr, 3 );//����	
	
}	


//��ͷ��ʼ������һ��ͬ�ļ�����Ŀ¼��ַ�������ص�һ����Ŀ¼��ַ
unsigned short GetDirNo( unsigned char* FileName, unsigned short* RemainStartDir )
{
	unsigned char Buff[22];
	unsigned short i;
	unsigned long Fls_Src;
	unsigned short CurDir;
	unsigned char* Ptr;

	Ptr = Buff;
	
	*RemainStartDir = 0xFFFF;
	
	CurDir = 0;
	Fls_Src = FLASH_SECTOR_SIZE;				//Ŀ¼�ӵ�һҳ�����ǵ�0ҳ��ʼ
	for( i=0;i<FAT_DIRS;i++ )
	{
		Read_Flash( Ptr, Fls_Src, 10 );
		if( IsAllData( Ptr, FAT_DIR_LEN, 0xff ) == 0 )	//�п���Ŀ¼λ��
		{
			if( *RemainStartDir == 0xFFFF ) *RemainStartDir = CurDir;
		}
		else
		{
			if( *(Ptr+9) != ChkNum( Ptr, 9 ) )
			{
				Read_Flash( Ptr+10, Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, 10 );
				if( *(Ptr+19) != ChkNum( Ptr+10, 9 ) ) return 0xFFFF;
				else
				{
					RAM_Write( Ptr, Ptr+10, 10 );				
				}	
			}							
		}			
		if( Data_Comp( Ptr, FileName, 5 ) == 0 )
		{
			return CurDir;			
		}	
		CurDir++;
		Fls_Src += FAT_DIR_LEN;			
	}	
	return 0xFFFF; 		//��ָ���ļ���
}	

//��ͷ��ʼ������һ��ͬ�ļ�����Ŀ¼��ַ�������ص�һ����Ŀ¼��ַ
unsigned short GetNewDirNo( unsigned char* FileName, unsigned short* RemainStartDir )
{
	unsigned char Buff[22];
	unsigned short i;
	unsigned long Fls_Src;
	unsigned short CurDir;
	unsigned char* Ptr;
	unsigned char* Point;

	Point = Buff;
	
	*RemainStartDir = 0xFFFF;
	
	CurDir = 0;
	Fls_Src = FLASH_SECTOR_SIZE;				//Ŀ¼�ӵ�һҳ�����ǵ�0ҳ��ʼ
	Read_Flash( FBuff.Buff, Fls_Src, 4096 );
	Ptr = FBuff.Buff;
	for( i=0;i<FAT_DIRS;i++ )
	{
		if( IsAllData( Ptr, FAT_DIR_LEN, 0xff ) == 0 )	//�п���Ŀ¼λ��
		{
			if( *RemainStartDir == 0xFFFF ) *RemainStartDir = CurDir;
		}
		else
		{
			if( *(Ptr+9) != ChkNum( Ptr, 9 ) )
			{
				Read_Flash( Point, Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, 10 );
				if( *(Point+9) != ChkNum( Point, 9 ) ) return 0xFFFF;
				else
				{
					RAM_Write( Ptr, Point, 10 );				
				}	
			}							
		}			
		if( Data_Comp( Ptr, FileName, 5 ) == 0 )
		{
			return CurDir;			
		}	
		CurDir++;
		Fls_Src += FAT_DIR_LEN;			
		Ptr += FAT_DIR_LEN;
	}	
	return 0xFFFF; 		//��ָ���ļ���
}	


//��ʣ��Ŀ¼��������
//RemainStartDirʣ����еĵ�һ��Ŀ¼��ַ
unsigned short GetRemainDirs( unsigned short* RemainStartDir )
{
	unsigned short i;
	unsigned long Fls_Src;
	unsigned short IdleDir;
	unsigned char* Ptr;
	
	*RemainStartDir = 0xFFFF;
	
	Fls_Src = FLASH_SECTOR_SIZE;
	Read_Flash( FBuff.Buff, Fls_Src, 4096 );

	IdleDir = 0;
	Ptr = FBuff.Buff;
	for( i=0;i<FAT_DIRS;i++ )
	{
		if( IsAllData( Ptr, FAT_DIR_LEN, 0xff ) == 0 )
		{
			if( *RemainStartDir == 0xFFFF ) *RemainStartDir = i;
			IdleDir++;				
		}		
		Ptr += FAT_DIR_LEN;
	}	
	return IdleDir;
}	

//typedef struct
//{
//	unsigned short DirAddr;
//	unsigned short StartSector;
//	unsigned short Sectors;
//}FILEPARA

//�������ļ�ǰ����֪�������ܿ�����FLASH��������ô�����ݿ�
//�����ļ�ָ��Ŀ¼λ�á���һ������λ��
unsigned short CreatFile( FILEPARA* FilePara, unsigned char* FileName )
{
	unsigned char Buff[12];
//	unsigned char Buff2[4];
	unsigned char* Ptr;	
//	unsigned char* Point;	
	unsigned short DirAddr;
	unsigned short RemainStartDir;
	unsigned long Fls_Src;
	unsigned short i;
	unsigned short StartSector;
	unsigned short CurSector;
	unsigned short NextSector;
	
	Ptr = Buff;
//	Point = Buff2;
//	DirAddr = GetDirNo( FileName, &RemainStartDir );						//ȡ���ļ�Ŀ¼��ַ
	DirAddr = GetNewDirNo( FileName, &RemainStartDir );						//ȡ���ļ�Ŀ¼��ַ
	if( DirAddr != 0xFFFF ) return 0xFFFF;									//���ļ����Ѵ��ڣ�����ʧ��
	if( RemainStartDir == 0xFFFF ) return 0xFFFF;							//�޿��ļ�Ŀ¼��ַ
	FilePara->DirAddr = RemainStartDir;										//ȡ���ļ�Ŀ¼��ַ
	
	StartSector = 0;														//�ӵ�һ��Ԫ������ʼ����
	for( i=0;i<FilePara->Sectors;i++ )
	{
		NextSector = GetNextRemainSector( StartSector );					//����������λ��
		if( NextSector == 0xFFFF ) return 0xFFFF;							//���޿�����λ��
		if( i == 0 )
		{
			FilePara->StartSector = NextSector;								//�ļ���ʼ����λ��
			RAM_Write( Ptr, FileName, 5 );
			RAM_Write( Ptr+5, (unsigned char*)&FilePara->StartSector, 2 );
			RAM_Write( Ptr+7, (unsigned char*)&FilePara->Sectors, 2 );
			*(Ptr+9) = ChkNum( Ptr, 9 );
			Fls_Src = FLASH_SECTOR_SIZE;									//FAT�ļ�Ŀ¼��
			Fls_Src += RemainStartDir * 10;
			Write_Flash_Direct( Fls_Src, Ptr, 10 );								//�����ļ�Ŀ¼��FLASH��FF����ֱ��д����ֵ	
			Write_Flash_Direct( Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, Ptr, 10 );//�����ļ�Ŀ¼��FLASH��FF����ֱ��д����ֵ	
		}
		else
		{
			WriteNextSectorNo( CurSector, NextSector );						//�ڵ�ǰ����λ�ã�д����һ������ַ
		}		
		CurSector = NextSector;												//����һ������ַ��Ϊ��ǰ������ַ
		StartSector = NextSector + 1;
	}	
	if( i == FilePara->Sectors )												//�����������
	{
		WriteNextSectorNo( CurSector, 0x0000 );								//���һ����������0x0000,��ʾ�ļ�������
	}	
	return 0;
}	

//���ļ�����ʼ�˿�ʼɾ��ָ������������
//StartSector:��ʼ����λ��
//SectorSum:��������
//����0������ɾ�������㣺ɾ���쳣
unsigned short DeleteFileSector( unsigned short StartSector, unsigned short SectorSum )					
{
	unsigned char Buff[6];
	unsigned short i;
	unsigned short Addr;
	unsigned long Fls_Src;
	unsigned short CurSector;
	unsigned char* Ptr;
	unsigned char* Point;
	unsigned short Sector;
	unsigned long PageNo;
	unsigned long NewPageNo;

	Point = Buff;
	
	CurSector = StartSector;
	for( i=0;i<SectorSum; )
	{
		PageNo = CurSector / FAT_SECTOR_INDEXS; 					//���ܲ��������ĵ�һҳ
		if( PageNo > FAT_INDEX_SECTOR_LEN ) return 0xFFFF;			//����������,������Ч��Χ	
		Sector = CurSector % FAT_SECTOR_INDEXS;
    	
		Fls_Src = (PageNo+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;
		Read_Flash( FBuff.Buff, Fls_Src, 4096 );
		
		Ptr = FBuff.Buff;
		do
		{
			Addr = Sector * 3;
			RAM_Write( Point, Ptr+Addr, 3 );						//��ȡ��һ������ַ
			if( *(Point+2) != ChkNum( Point, 2 ) )
			{
				Read_Flash( Point+3, Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, 3 );
				if( *(Point+5) != ChkNum( Point+3, 2 ) ) return 0xFFFF;
				else
				{
					RAM_Write( Point, Point+3, 3 );				
				}	
			}	
			RAM_Write( (unsigned char*)&CurSector, Point, 2 );				
			RAM_DataFill( Ptr+Addr, 3, 0xFF);					//��������ʱ������ȫ����0xFF,���㴴���ļ�ʱֱ��д�룬����Ҫ�ٲ���
			NewPageNo = CurSector / FAT_SECTOR_INDEXS;
			Sector = CurSector % FAT_SECTOR_INDEXS;	
			i++;
			if(( i>= SectorSum )||( CurSector == 0 )) break;	//�ļ�����
		}while( PageNo == NewPageNo );							//ͬһ����ҳ����RAM�ڲ���
		
		BlockErase(Fls_Src);
		Write_Flash_Direct( Fls_Src, FBuff.Buff, 4096 );
		Fls_Src += FAT_SECTOR_BAK*FLASH_SECTOR_SIZE;			//���µ�ǰ����
		BlockErase(Fls_Src);
		Write_Flash_Direct( Fls_Src, FBuff.Buff, 4096 );				//���±�������

		if(( i>= SectorSum )||( CurSector == 0 )) break;		//�ļ�����
	}	
	
	return 0;
	
}

//��ɾ���ļ�ǰ����֪�ļ���
//����0��ɾ���ɹ�����0��ɾ��ʧ��
unsigned short DeletFile( FILECURVEPARA* FileCurvePara, unsigned char* FileName )
{
	unsigned char Buff[24];
	unsigned char* Ptr;	
	unsigned short DirAddr;
//	unsigned short RemainStartDir;
	unsigned long Fls_Src;
	unsigned short StartSector;
	unsigned short SectorSum;
	
	Ptr = Buff;
//	DirAddr = GetDirNo( FileName, &RemainStartDir );						//ȡ���ļ�Ŀ¼��ַ
//	if( DirAddr == 0xFFFF ) return 0xFFFF;									//���ļ��������ڣ�ɾ��ʧ��
	
	DirAddr = FileCurvePara->DirNo;											//��ɾ���ļ�ǰ�ȴ��ļ����		
	Fls_Src = FAT_SECTOR * FLASH_SECTOR_SIZE;
	Fls_Src += DirAddr * 10;
	Read_Flash( Ptr, Fls_Src, 10 );
	if( *(Ptr+9) != ChkNum( Ptr, 9 ) )
	{
		Read_Flash( Ptr+10, Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, 10 );
		if( *(Ptr+19) != ChkNum( Ptr+10, 9 ) ) return 0xFFFF;
		else
		{
			RAM_Write( Ptr, Ptr+10, 10 );				
		}	
	}	
	
	RAM_Write( (unsigned char*)&StartSector, Ptr+5, 2 );
	RAM_Write( (unsigned char*)&SectorSum, Ptr+7, 2 );

	if( DeleteFileSector( StartSector, SectorSum ) != 0 ) return 0xFFFF;

	RAM_DataFill( Ptr, 10, 0xFF);
	Fls_Src = FAT_SECTOR*FLASH_SECTOR_SIZE;
	Read_Flash( FBuff.Buff, Fls_Src, 4096 );
	RAM_Write( FBuff.Buff+DirAddr * 10, Ptr, 10 );
	
	BlockErase(Fls_Src);
	Write_Flash_Direct( Fls_Src, FBuff.Buff, 4096 );
	Fls_Src += FAT_SECTOR_BAK*FLASH_SECTOR_SIZE;			//���µ�ǰĿ¼
	BlockErase(Fls_Src);
	Write_Flash_Direct( Fls_Src, FBuff.Buff, 4096 );				//���±���Ŀ¼

	return 0;
}	

//���ļ��������з��Ӧ���ļ�����ռ�ÿռ��Ƿ�һ�£�����ָ���һ��������
unsigned short Open_File( FILECURVEPARA* FileCurvePara, unsigned char* FileName )
{
	unsigned char Buff[24];
	unsigned char* Ptr;	
	unsigned short DirAddr;
	unsigned short RemainStartDir;
	unsigned long Fls_Src;
	unsigned short StartSector;
	unsigned short SectorSum;
	
	Ptr = Buff;
	DirAddr = GetDirNo( FileName, &RemainStartDir );						//ȡ��ǰ�ļ�Ŀ¼��ַ
	if( DirAddr == 0xFFFF ) return 0xFFFF;									//���ļ��������ڣ�ɾ��ʧ��
	
	Fls_Src = FAT_SECTOR * FLASH_SECTOR_SIZE;
	Fls_Src += DirAddr * 10;
	Read_Flash( Ptr, Fls_Src, 10 );
	if( *(Ptr+9) != ChkNum( Ptr, 9 ) )
	{
		Read_Flash( Ptr+10, Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, 10 );
		if( *(Ptr+19) != ChkNum( Ptr+10, 9 ) ) return 0xFFFF;
		else
		{
			RAM_Write( Ptr, Ptr+10, 10 );				
		}	
	}	
	
	RAM_Write( (unsigned char*)&StartSector, Ptr+5, 2 );
	RAM_Write( (unsigned char*)&SectorSum, Ptr+7, 2 );
	if( SectorSum != FileCurvePara->Pages ) return 0xFFFF;					//�洢�ռ��Ƿ�һ��
	else FileCurvePara->StartSectorNo = StartSector;

	FileCurvePara->DirNo = DirAddr;
	return 0;
}	

//������ʼ�����ţ���ȡָ��������Ӧ�����������׵�ַ,SectorSum=0ʱָ���һ��
long GetAppointSector( unsigned short StartSector, unsigned short SectorSum )					
{
	unsigned char Buff[6];
	unsigned short i;
//	unsigned short Addr;
	long Fls_Src;
	unsigned short CurSector;
	unsigned char* Point;
	unsigned short Sector;
	unsigned long PageNo;

	Point = Buff;
	
	CurSector = StartSector;
	for( i=0;i<SectorSum;i++ )
	{
		PageNo = CurSector / FAT_SECTOR_INDEXS; 					//���ܲ��������ĵ�һҳ
		if( PageNo > FAT_INDEX_SECTOR_LEN ) return -1;				//����������,������Ч��Χ	
		Sector = CurSector % FAT_SECTOR_INDEXS;
    	
		Fls_Src = (PageNo+FAT_INDEX_START_SECTOR) * FLASH_SECTOR_SIZE;
		
		Fls_Src += Sector * 3;
		Read_Flash( Point, Fls_Src, 3 );
		if( *(Point+2) != ChkNum( Point, 2 ) )
		{
			Read_Flash( Point+3, Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, 3 );		//ȡ��������
			if( *(Point+5) != ChkNum( Point+3, 2 ) ) return -1;
			else
			{
				RAM_Write( Point, Point+3, 3 );				
			}	
		}	
		RAM_Write( (unsigned char*)&CurSector, Point, 2 );			//��һ������ַ	
	}
	
	CurSector += DATA_START_SECTOR;									//ǰһ������ֵ����Ϊ��Ӧ��������
	Fls_Src = CurSector;
	Fls_Src *= FLASH_SECTOR_SIZE;									//��ʵ������ַ
	
	return Fls_Src;
	
}

//////////////////////////////////////////////////////////////////
//��ͷ��ʼ������һ��ͬ�ļ�����Ŀ¼��ַ�������ص�һ����Ŀ¼��ַ
void GetDirTo_FBuff( void )								//17.03.08
{
	unsigned long Fls_Src;

	Fls_Src = FLASH_SECTOR_SIZE;				//Ŀ¼�ӵ�һҳ�����ǵ�0ҳ��ʼ
	Read_Flash( FBuff.Buff, Fls_Src, 4096 );
}

//��RAM�д�ͷ��ʼ������һ��ͬ�ļ�����Ŀ¼��ַ�������ص�һ����Ŀ¼��ַ
unsigned short GetDirNo_Form_FBuff( unsigned char* FileName, unsigned short* RemainStartDir )		//17.03.08
{
	unsigned char Buff[22];
	unsigned short i;
	unsigned long Fls_Src;
	unsigned short CurDir;
	unsigned char* Ptr;
	unsigned char* Point;

	Point = Buff;
	Ptr = FBuff.Buff;
	
	*RemainStartDir = 0xFFFF;
	
	CurDir = 0;
	Fls_Src = FLASH_SECTOR_SIZE;				//Ŀ¼�ӵ�һҳ�����ǵ�0ҳ��ʼ
	for( i=0;i<FAT_DIRS;i++ )
	{
//		Read_Flash( Ptr, Fls_Src, 10 );
		if( IsAllData( Ptr, FAT_DIR_LEN, 0xff ) == 0 )	//�п���Ŀ¼λ��
		{
			if( *RemainStartDir == 0xFFFF ) *RemainStartDir = CurDir;
		}
		else
		{
			if( *(Ptr+FAT_DIR_LEN-1) != ChkNum( Ptr, FAT_DIR_LEN-1 ) )
			{
				Read_Flash( Point, Fls_Src+FAT_SECTOR_BAK*FLASH_SECTOR_SIZE, 10 );
				if( *(Point+FAT_DIR_LEN-1) != ChkNum( Point, FAT_DIR_LEN-1 ) ) return 0xFFFF;
				else
				{
					RAM_Write( Ptr, Point, FAT_DIR_LEN );				
				}	
			}							
		}			
		if( Data_Comp( Ptr, FileName, 5 ) == 0 )
		{
			return CurDir;			
		}	
		CurDir++;
		Fls_Src += FAT_DIR_LEN;		
		Ptr += FAT_DIR_LEN;		
	}	
	return 0xFFFF; 		//��ָ���ļ���
}	

//����ǰĿ¼����FBuff��ʱ�����ļ��������з��Ӧ���ļ�����ռ�ÿռ��Ƿ�һ�£�����ָ���һ��������
unsigned short Open_File_In_FBuff( FILECURVEPARA* FileCurvePara, unsigned char* FileName )				//17.03.08
{
	unsigned char* Ptr;	
	unsigned short DirAddr;
	unsigned short RemainStartDir;
	unsigned short StartSector;
	unsigned short SectorSum;

	Ptr = FBuff.Buff;

	DirAddr = GetDirNo_Form_FBuff( FileName, &RemainStartDir );						//ȡ��ǰ�ļ�Ŀ¼��ַ
	if( DirAddr == 0xFFFF ) return 0xFFFF;									//���ļ��������ڣ�ɾ��ʧ��
	Ptr += DirAddr * FAT_DIR_LEN;
	
	RAM_Write( (unsigned char*)&StartSector, Ptr+5, 2 );
	RAM_Write( (unsigned char*)&SectorSum, Ptr+7, 2 );
	if( SectorSum != FileCurvePara->Pages ) return 0xFFFF;					//�洢�ռ��Ƿ�һ��
	else FileCurvePara->StartSectorNo = StartSector;

	FileCurvePara->DirNo = DirAddr;
	return 0;
}	



