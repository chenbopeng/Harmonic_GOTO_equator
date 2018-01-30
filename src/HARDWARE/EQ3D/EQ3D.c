#include "EQ3D.h"

void EQ3D_GPIO_OUT_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
		PCout(13)=1;
		PBout(12)=1;
}

void EQ3D_GPIO_KEY_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);//使能PORTA,PORTC时钟

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
}
void DEC_STEP_Config(u8 a)
{
    switch (a)
    {
    case 1:
        DEC_MS0 = 0; //1MS0 //DEC
        DEC_MS1 = 0; //1MS1
        DEC_MS2 = 0; //1MS2
        break;
    case 2:
        DEC_MS0 = 1; //1MS0
        DEC_MS1 = 0; //1MS1
        DEC_MS2 = 0; //1MS2
        break;
    case 4:
        DEC_MS0 = 0; //1MS0
        DEC_MS1 = 1; //1MS1
        DEC_MS2 = 0; //1MS2
        break;
    case 8:
        DEC_MS0 = 1; //1MS0
        DEC_MS1 = 1; //1MS1
        DEC_MS2 = 0; //1MS2
        break;
    case 16:
        DEC_MS0 = 0; //1MS0
        DEC_MS1 = 0; //1MS1
        DEC_MS2 = 1; //1MS2
        break;
    default:
        DEC_MS0 = 1; //1MS0
        DEC_MS1 = 1; //1MS1
        DEC_MS2 = 1; //1MS2
    }
}

void RA_STEP_Config(u8 a)
{
    switch (a)
    {
    case 1:
        RA_MS0 = 0; //1MS0  //RA
        RA_MS1 = 0; //1MS1
        RA_MS2 = 0; //1MS2
        break;
    case 2:
        RA_MS0 = 1; //1MS0
        RA_MS1 = 0; //1MS1
        RA_MS2 = 0; //1MS2
        break;
    case 4:
        RA_MS0 = 0; //1MS0
        RA_MS1 = 1; //1MS1
        RA_MS2 = 0; //1MS2
        break;
    case 8:
        RA_MS0 = 1; //1MS0
        RA_MS1 = 1; //1MS1
        RA_MS2 = 0; //1MS2
        break;
    case 16:
        RA_MS0 = 0; //1MS0
        RA_MS1 = 0; //1MS1
        RA_MS2 = 1; //1MS2
        break;
    default:
        RA_MS0 = 1; //1MS0
        RA_MS1 = 1; //1MS1
        RA_MS2 = 1; //1MS2
    }
}


void EQ3D_STEP(u8 stp_ra, u8 stp_dec)  //向步进电机驱动器按需发送脉冲
{
    u8 a;
    RA_STP = stp_ra;
    DEC_STP = stp_dec;
    for(a = 22; a > 0; a--) {};
    RA_STP = 0;
    DEC_STP = 0;
}

void HANDLE_CONTROL(u16 speed, u8 *dir_state, s8 *key_statue) //按指定的速度、基准方向、方向执行手控器按键的动作，以及正常速度跟踪
{
		u16 temp;
		static u16 ra_stp_count = 1, dec_stp_count = 1;
		u32 ra_overflows, dec_overflows;
    u8 ra_stp_flag = 0, dec_stp_flag = 0;
	
	  ra_stp_count++;
    dec_stp_count++;
	
		if(key_statue[0]==0&&key_statue[1]==0){SW_LED=0;}  //按键指示灯
		else {SW_LED=1;}
	
    switch (key_statue[0])//RA运动计算
    {
			case 0:  //RA正常跟踪
				RA_DIR = !dir_state[0];
				RA_EN = STEPPER_MOTOR_STAR;
				ra_overflows = NORM_STP_COUNT;
				break;
			case -1:  //RA+  S+1倍速
				RA_DIR = !dir_state[0];
				RA_EN = STEPPER_MOTOR_STAR;
				ra_overflows = NORM_STP_COUNT/(speed+1);
				break;
			case 1:  //RA-  S-1倍速
				RA_DIR = dir_state[0];
				RA_EN = STEPPER_MOTOR_STAR;
				temp=speed-1;
				if(temp==0){ra_overflows=0xfffff;} //当2倍速时，使得计数无法达到溢出值，以此使RA轴停转
				else {ra_overflows = NORM_STP_COUNT/(speed-1);}
				break;
			default:
				break;
    }
		switch (key_statue[1])//DEC运动计算
    {
			case 0:  //DEC停转
				DEC_EN = !STEPPER_MOTOR_STAR;
        dec_overflows = NORM_STP_COUNT;
        dec_stp_count=0;	//不加这句则无法正常起转，原因待查
				break;
			case 1:  //DEC+
				DEC_DIR = dir_state[1];
				DEC_EN = STEPPER_MOTOR_STAR;
				dec_overflows = NORM_STP_COUNT/speed;
				break;
			case -1:  //DEC-
				DEC_DIR = !dir_state[1];
				DEC_EN = STEPPER_MOTOR_STAR;
				dec_overflows = NORM_STP_COUNT/speed;
				break;
			default:
				break;
		}
    if(ra_stp_count > ra_overflows)
    {
        ra_stp_count = 1;
        ra_stp_flag = 1;
    }
    else
    {
        ra_stp_flag = 0;
    }
    if(dec_stp_count > dec_overflows)
    {
        dec_stp_count = 1;
        dec_stp_flag = 1;
    }
    else
    {
        dec_stp_flag = 0;
    }

    if(ra_stp_flag == 1 || dec_stp_flag == 1)
    {
        EQ3D_STEP(ra_stp_flag, dec_stp_flag);
    }
}


void GOTO ( int *ra_step, int *dec_step , u8 *dir_state)  //按计算结果移动步进电机
{
    static u16 ra_stp_count = 1, dec_stp_count = 1;
    u16 ra_overflows, dec_overflows;
    u8 ra_stp_flag = 0, dec_stp_flag = 0;

    ra_stp_count++;
    dec_stp_count++;

    RA_EN = 0;
    if((*ra_step) == 0 )	//到位了就切换到RA正常速度跟踪
    {
        RA_DIR = !dir_state[0];
        ra_overflows = NORM_STP_COUNT;
    }
    else if((*ra_step) > 0 )	//如果步进没到位就步进
    {
        RA_DIR = dir_state[0];
        ra_overflows = GOTO_STP_COUNT;
    }
    else	//如果步进没到位就步进
    {
        RA_DIR = !dir_state[0];
        ra_overflows = GOTO_STP_COUNT;
    }

    if((*dec_step) == 0)		//到位了DEC轴停转
    {
        DEC_EN = 1;
    }
    else if( (*dec_step) > 0 )		//如果步进没到位就步进
    {
        DEC_DIR = dir_state[1];
        DEC_EN = 0;
        dec_overflows = GOTO_STP_COUNT;
    }
    else		//如果步进没到位就步进
    {
        DEC_DIR = !dir_state[1];
        DEC_EN = 0;
        dec_overflows = GOTO_STP_COUNT;
    }

    if(ra_stp_count > ra_overflows) //RA轴运动标志置位及按运动情况修改记步数据
    {
        ra_stp_count = 1;
        ra_stp_flag = 1;
        if ( (*ra_step) > 0 )
        {
            (*ra_step)--;
        }
        if ( (*ra_step) < 0 )
        {
            (*ra_step)++;
        }
    }
    else
    {
        ra_stp_flag = 0;
    }
    if(dec_stp_count > dec_overflows) //DEC轴运动标志置位及按运动情况修改记步数据
    {
        dec_stp_count = 1;
        dec_stp_flag = 1;
        if ( (*dec_step) > 0 )
        {
            (*dec_step)--;
        }
        if ( (*dec_step) < 0 )
        {
            (*dec_step)++;
        }
    }
    else
    {
        dec_stp_flag = 0;
    }

    if(ra_stp_flag == 1 || dec_stp_flag == 1) //按运动标志位执行动作
    {
        EQ3D_STEP(ra_stp_flag, dec_stp_flag);
    }
}

void LOCAL_KEY_CONTROL ( s8 *key_statue ) //手控器上方向按键信号采集
{
	if(RA_PLUS==0)
		{key_statue[0]=-1;}
	if(RA_SUB==0)
		{key_statue[0]=1;}
	if(RA_PLUS == 1 && RA_SUB == 1)
		{key_statue[0]=0;}
	
	if(DEC_PLUS==0)
		{key_statue[1]=1;}
	if(DEC_SUB==0)
		{key_statue[1]=-1;}
	if(DEC_PLUS == 1 && DEC_SUB == 1)
		{key_statue[1]=0;}
}

void KEY_CONTROL_MIX(s8 *remote_key_state,s8 *local_key_state,s8 *key_state)  //手控器的按键及无线按键控制信号融合
{
	key_state[0]=remote_key_state[0]+local_key_state[0];
	key_state[1]=remote_key_state[1]+local_key_state[1];
}

u16 SPEED_CONTROL()  //速度切换按钮对速度的控制
{
	static int counter;
	static u8 temp=1;
    if(SPEED_KEY==0)  //速度切换
    {
        counter++;
        if(counter==2000)
        {
					temp++;
					if(temp>=5)
					{
						temp=1;
					}
        }
    }
    else counter=0;
		switch (temp)
		{
			case 1:
				SPEED_LED=0; 
				return GUIDE_SPEED;
			case 2:
				SPEED_LED=0; 
				return LOW_SPEED;
			case 3:
				SPEED_LED=1; 
				return MEDIUM_SPEED;
			case 4:
				SPEED_LED=1; 
				return HIGH_SPEED;
			default:
				return GUIDE_SPEED;
		}
}

void DIR_CONTROL(u8 *dir_state)  //方向切换按钮对基准方向的控制
{
	static int counter;
	static u8 temp=1;
    if(DIR_KEY==0)  //速度切换
    {
        counter++;
        if(counter==2000)
        {
					temp++;
					if(temp>=5)
					{
						temp=1;
					}
        }
    }
		else counter=0;
		switch (temp)
		{
			case 1:
				DIR_LED=0;
				dir_state[0]=RA_DIR_SET;
				dir_state[1]=DEC_DIR_SET;
				break;
			case 2:
				DIR_LED=0; 
				dir_state[0]=RA_DIR_SET;
				dir_state[1]=!DEC_DIR_SET;
				break;
			case 3:
				DIR_LED=1; 
				dir_state[0]=!RA_DIR_SET;
				dir_state[1]=DEC_DIR_SET;
				break;
			case 4:
				DIR_LED=1; 
				dir_state[0]=!RA_DIR_SET;
				dir_state[1]=!DEC_DIR_SET;
				break;
			default:
				dir_state[0]=RA_DIR_SET;
				dir_state[1]=DEC_DIR_SET;
		}
}


