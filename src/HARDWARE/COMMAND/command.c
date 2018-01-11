#include "command.h"
#include "dma.h"
#include "stdlib.h"
#include "string.h"

//函数名：LX200
//功能：解析LX200协议，该协议波特率9600，本函数支持Stellarium和skysafari。
//输入参数：buffer 报文字段输入
//          current_pos[2] 当前坐标（角秒）,用于向上位机汇报
//          target_pos[2] 目标坐标（角秒），作为输出使用，供上级程序调用
//返回值：返回解析状态，0解析失败，1解析成功且已进行反馈，3成功解析并返回目标RA值，4成功解析并返回目标DEC值，
//        'e'向东按键，'E'向东按键停止，'w'向西按键，'W'向西按键停止，'s'向南按键，'S'向南按键停止，'n'向北按键，'N'向北按键停止
u8 LX200( u8 *buffer, s32 *current_pos, s32 *target_pos )
{
    u8 command[20], target_data[10];
    static u8 send_data[10];
    u8 i=0, a=0;

    while ( buffer[i] != ':' ) //找头，如果找头失败就直接返回
    {
        i++;
        if ( i > USART_REC_LEN - 5 )
        {
            return 0;
        }
    }
    for ( a = 0; a < 19; a++ ) //转存有效命令
    {
        command[a] = buffer[i];
        i++;
    }
    memset(buffer, 0, USART_REC_LEN); //清空数据

    /*ANALYZE AND REPLY*/
    if ( command[1] == 'G') //向上位机回复当前朝向
    {
        if ( command[2] == 'R' ) //GR 回复赤经 24h60m60s
        {
            RA_ARCSEC_INTO_DEG ( current_pos[0], send_data );  //把当前位置（角秒）转换为度分秒再发送出去
            DMA_SEND_DATA ( ( u32 ) send_data, 9 );
        }
        if ( command[2] == 'D' ) //GD 回复赤纬 +-90:60:60
        {
            DEC_ARCSEC_INTO_DEG( current_pos[1], send_data );  //把当前位置（角秒）转换为度分秒再发送出去
            DMA_SEND_DATA ( ( u32 ) send_data, 10 );
        }
        return 1;
    }

    if ( command[1] == 'Q' && command[5] == 'r' ) //#:Q#:Sr 回复1
    {
        target_data[0] = command[7];
        target_data[1] = command[8];
        target_data[2] = command[10];
        target_data[3] = command[11];
        target_data[4] = command[13];
        target_data[5] = command[14];
        command[0] = '1';
        DMA_SEND_DATA ( ( u32 ) command, 1 );
        target_pos[0] = RA_DEG_INTO_ARCSEC ( target_data );
        return 3;
    }

    if ( command[1] == 'Q' )
    {
        if ( command[2] == 'e' ) //Qe 遥控按键东停止
        {
            return 'E';
        }
        if ( command[2] == 'w' ) //Qw 遥控按键西停止
        {
            return 'W';
        }
        if ( command[2] == 's' ) //Qs 遥控按键南停止
        {
            return 'S';
        }
        if ( command[2] == 'n' ) //Qn 遥控按键北停止
        {
            return 'N';
        }
    }

    if ( command[1] == 'S' && command[2] == 'r' ) //:Sr 回复1
    {
        target_data[0] = command[3];
        target_data[1] = command[4];
        target_data[2] = command[6];
        target_data[3] = command[7];
        target_data[4] = command[9];
        target_data[5] = command[10];
        command[0] = '1';
        DMA_SEND_DATA ( ( u32 ) command, 1 );
        target_pos[0] = RA_DEG_INTO_ARCSEC ( target_data );
        return 3;
    }

    if ( command[1] == 'S' && command[2] == 'd' ) //Sd 回复1
    {
        if ( command[3] == ' ' )
        {
            target_data[0] = command[4];
            target_data[1] = command[5];
            target_data[2] = command[6];
            target_data[3] = command[8];
            target_data[4] = command[9];
            target_data[5] = command[11];
            target_data[6] = command[12];
        }
        else
        {
            target_data[0] = command[3];
            target_data[1] = command[4];
            target_data[2] = command[5];
            target_data[3] = command[7];
            target_data[4] = command[8];
            target_data[5] = command[10];
            target_data[6] = command[11];
        }
        command[0] = '1';
        DMA_SEND_DATA ( ( u32 ) command, 1 );
        target_pos[1] = DEC_DEG_INTO_ARCSEC ( target_data );
        return 4;
    }

    if ( command[1] == 'M' )
    {
        if ( command[2] == 'S' ) //Ms 回复0
        {
            command[0] = '0';
            DMA_SEND_DATA ( ( u32 ) command, 1 );
            return 1;
        }
        if ( command[2] == 'e' ) //Me 遥控按键东
        {
            return 'e';
        }
        if ( command[2] == 'w' ) //Mw 遥控按键西
        {
            return 'w';
        }
        if ( command[2] == 's' ) //Ms 遥控按键南
        {
            return 's';
        }
        if ( command[2] == 'n' ) //Mn 遥控按键北
        {
            return 'n';
        }
    }
    return 0;

}

//函数名：RA_STEP_CALCULATE
//功能：计算得到RA轴由当前位置到目标位置所需的步数
//输入参数：current_ra 当前的RA值（角秒）
//          target_ra 目标RA值（角秒）
//					stp_angle RA步进单步角度（角秒）
//返回值：ra_step 有符号整数，为到目标位置所需的步进数，符号代表步进方向
int RA_STEP_CALCULATE ( s32 current_ra, s32 target_ra,double stp_angle )
{
    int ra_step ;
    if ( abs ( current_ra - target_ra ) > 648000 )  //差值大于半周
    {
        if ( current_ra > target_ra )	//当前23  目标1  当前数值++
        {
            ra_step = ( target_ra + 1296000 - current_ra ) / stp_angle;
            return  ra_step;//RA数值+
        }
        else 													//当前1  目标23  当前数值--
        {
            ra_step = ( target_ra - 1296000 - current_ra ) / stp_angle;
            return  ra_step; //RA数值-
        }
    }
    else   																					 //差值小于半周
    {
        ra_step = ( target_ra - current_ra ) / stp_angle;
        return  ra_step; //不必再分情况，这样相减自带符号
    }
}

//函数名：DEC_STEP_CALCULATE
//功能：计算得到DEC轴由当前位置到目标位置所需的步数
//输入参数：current_dec 当前的DEC值（角秒）
//          target_dec 目标DEC值（角秒）
//					stp_angle DEC步进单步角度（角秒）
//返回值：dec_step 有符号整数，为到目标位置所需的步进数，符号代表步进方向
int DEC_STEP_CALCULATE ( s32 current_dec, s32 target_dec,double stp_angle )
{
    int dec_step;
    dec_step = ( target_dec - current_dec ) / stp_angle;
    return dec_step ;
}

//函数名：CURRENT_POS_RA
//功能：由目标位置及剩余的步数和步进方向算出当前位置
//输入参数：target_ra 目标RA值（角秒）
//          ra_step RA剩余的步进数
//					stp_angle RA步进单步角度（角秒）
//返回值：current_ra 当前RA的值（角秒）
s32 CURRENT_POS_RA ( s32 target_ra,int ra_step,double stp_angle )
{
    s32 current_ra;
    if ( target_ra - ra_step * stp_angle < 0 ) //目标1 移动+2
    {
        current_ra = target_ra + 1296000 - ra_step * stp_angle; //1+24-2
    }
    if ( target_ra - ra_step * stp_angle > 1296000 )  //目标23 移动-2
    {
        current_ra = target_ra - 1296000 - ra_step * stp_angle; //23-24-（-2）
    }
    else																			//目标15 移动-2
    {
        current_ra = target_ra - ra_step * stp_angle;  //15-（-2）
    }
    return current_ra;
}

//函数名：CURRENT_POS_DEC
//功能：由目标位置及剩余的步数和步进方向算出当前位置
//输入参数：target_dec 目标DEC值（角秒）
//          dec_step DEC剩余的步进数
//					stp_angle DEC步进单步角度（角秒）
//返回值：current_dec 当前DEC的值（角秒）
s32 CURRENT_POS_DEC ( s32 target_dec,int dec_step, double stp_angle )
{
    s32 current_dec;
    current_dec = target_dec - dec_step * stp_angle;
    return current_dec;
}

u32 RA_DEG_INTO_ARCSEC ( u8 ra[6] ) //ra的度分秒转化为角秒，算目标位置时使用
{
    u32 result;
    result = ( ra[0] - '0' ) * 540000 + ( ra[1] - '0' ) * 54000 + ( ra[2] - '0' ) * 9000 + ( ra[3] - '0' ) * 900 + ( ra[4] - '0' ) * 150 + ( ra[5] - '0' ) * 15;
    return result;
}

u32 DEC_DEG_INTO_ARCSEC ( u8 dec[7] ) //dec的度分秒转化为角秒，算目标位置时使用
{
    u32 result;
    result = ( dec[1] - '0' ) * 36000 + ( dec[2] - '0' ) * 3600 + ( dec[3] - '0' ) * 600 + ( dec[4] - '0' ) * 60 + ( dec[5] - '0' ) * 10 + dec[6] - '0';
    if ( dec[0] == '+' )
        result = 324000 + result;
    if ( dec[0] == '-' )
        result = 324000 - result;
    return result;
}

void RA_ARCSEC_INTO_DEG( s32 current_ra, u8 *ra_out ) //根据current_ra更新当前朝向值，回复QD QR时使用
{
    s32 temp;
    temp = current_ra / 54000;
    ra_out[0] = ( temp / 10 ) + '0';
    ra_out[1] = ( temp % 10 ) + '0';
    ra_out[2] = ':';

    temp = current_ra % 54000;
    temp = temp / 900;
    ra_out[3] = ( temp / 10 ) + '0';
    ra_out[4] = ( temp % 10 ) + '0';
    ra_out[5] = ':';

    temp = current_ra % 900;
    temp = temp / 15;
    ra_out[6] = ( temp / 10 ) + '0';
    ra_out[7] = ( temp % 10 ) + '0';
    ra_out[8] = '#';
}

void DEC_ARCSEC_INTO_DEG ( s32 current_dec, u8 *dec_out ) //根据current_dec更新当前朝向值，回复QD QR时使用
{
    s32 temp;
    s32 temp2;
    if ( current_dec >= 324000 )
    {
        temp2 = current_dec;
        temp2 = temp2 - 324000;
        dec_out[0] = '+';
    }
    if ( current_dec < 324000 )
    {
        temp2 = current_dec;
        temp2 = 324000 - temp2;
        dec_out[0] = '-';
    }

    temp = temp2 / 3600;
    dec_out[1] = ( temp / 10 ) + '0';
    dec_out[2] = ( temp % 10 ) + '0';
    dec_out[3] = ':';

    temp = temp2 % 3600;
    temp = temp / 60;
    dec_out[4] = ( temp / 10 ) + '0';
    dec_out[5] = ( temp % 10 ) + '0';
    dec_out[6] = ':';

    temp = temp2 % 60;
    dec_out[7] = ( temp / 10 ) + '0';
    dec_out[8] = ( temp % 10 ) + '0';
    dec_out[9] = '#';
}

u8 GOTO_CHECK( u8 decode_state)
{
    static u8 goto_flag=0;
    if ( goto_flag==0xff) goto_flag=0;
    if(decode_state==3)
    {
        goto_flag|=0xf0;
    }
    if(decode_state==4)
    {
        goto_flag|=0x0f;
    }
    return goto_flag;
}

void REMOTE_KEY_CONTROL ( s8 *remote_key_state,  u8 decode_state )
{
    switch (decode_state)
    {
    case 'E':
        remote_key_state[0]=0;
        break;
    case 'W':
        remote_key_state[0]=0;
        break;
    case 'S':
        remote_key_state[1]=0;
        break;
    case 'N':
        remote_key_state[1]=0;
        break;
    case 'e':  //RA-
        remote_key_state[0]=1;
        break;
    case 'w':  //RA+
        remote_key_state[0]=-1;
        break;
    case 's':  //DEC-
        remote_key_state[1]=-1;
        break;
    case 'n':  //DEC+
        remote_key_state[1]=1;
        break;
    default:
        break;
    }
}

void SHUTTER_CONTROL( u8 *buffer, u16 *shutter )
{
	char temp[5]={0};
	u8 i=0 , j=0; 
	if( buffer[0]=='T' && buffer[1]=='p') //协议找头
	{
		while(buffer[i+2]!=' ')  //第一组数据转存
		{
			temp[i]=buffer[i+2];
			i++;
		}
		shutter[0]  = atof(temp); //转为u16
		
		memset(temp, 0, 5); //清空数据
		i++;
		while(buffer[i+2]!=0)  //第二组数据转存
		{
			temp[j]=buffer[i+2];
			i++;
			j++;
		}
		shutter[1] = atof(temp);
		memset(buffer, 0, 20); //清空数据
	}
}

