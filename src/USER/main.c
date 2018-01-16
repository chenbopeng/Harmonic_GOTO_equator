#include "delay.h"
#include "sys.h"
#include "dma.h"
#include "timer.h"
#include "EQ3D.h"
#include "usart.h"
#include "command.h"

u8 data_receive_buffer[USART_REC_LEN];  //来自于串口DMA输入
u8 step_flag=0;  //定时器中断标志

int main(void)
{
    s32 current_pos[2]= {0,0}, target_pos[2]= {0,0};
    s32 target_ra=0, target_dec=0;
    int ra_step=0, dec_step=0;
    u8 decode_state=0, dir_state[2]= {0,0} ;
    u16 move_speed=1;
    s8 remote_key_state[2]= {0,0}, local_key_state[2]= {0,0}, key_state[2]= {0,0} ;
		u16 shutter[2]={0,0}, timer_counter=0 , sec_counter=0 ;

    delay_init();	       //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
    uart3_init(9600);
    DMA_TX_init((u32)data_receive_buffer, USART_REC_LEN);
    DMA_RX_init((u32)data_receive_buffer, USART_REC_LEN);
    EQ3D_GPIO_KEY_Config();
    EQ3D_GPIO_OUT_Config();
    TIM3_Int_Init(STP_INTERVAL, 35); //42步进电机3595+1微秒/拍，
    PWR_LED = 1; //电源指示灯

    while(1)
    {
        if(data_receive_buffer[USART_REC_LEN - 1] == 1)  //DMA收到消息标志
        {
            data_receive_buffer[USART_REC_LEN - 1] = 0; //清理标志
					
						SHUTTER_CONTROL( data_receive_buffer, shutter); //相机快门线控制
					
            decode_state = LX200( data_receive_buffer, current_pos, target_pos ); //LX200协议解析
            REMOTE_KEY_CONTROL(remote_key_state,decode_state); //无线按键控制解码
            if(GOTO_CHECK(decode_state) == 0xff) //目标位置齐备
            {
                if(current_pos[0] == 0 && current_pos[1] == 0) //如果是系统刚开始运行，则将第一次的GOTO信息设置为当前位置
                {
                    current_pos[0] = target_pos[0];
                    current_pos[1] = target_pos[1];
                }
                else  //正常GOTO时，根据目标位置计算步数及方向，根据步数和方向乘以系数得到实际需要的步数和方向
                {
                    target_ra = target_pos[0];
                    target_dec = target_pos[1];
                    ra_step	=	RA_STEP_CALCULATE(current_pos[0], target_pos[0], RA_STP_ANGLE );   //计算各轴所需步数和方向，正负号代表方向
                    dec_step	=	DEC_STEP_CALCULATE(current_pos[1], target_pos[1], DEC_STP_ANGLE );

                    if(ra_step>0)
                    {
                        ra_step=ra_step*((float) NORM_STP_COUNT/(float)(NORM_STP_COUNT+GOTO_STP_COUNT));
                    }
                    else
                    {
                        ra_step=ra_step*((float) NORM_STP_COUNT/(float)(NORM_STP_COUNT-GOTO_STP_COUNT));
                    }
                }
            }
        }
        if(step_flag == 1)  //步进标志
        {
            step_flag = 0;  //清除标志
            DIR_CONTROL(dir_state);  //按键切换赤道仪移动方向基准值（北半球东向，北半球西向，南半球东向，南半球西向）是为了解决中天翻转和南北半球设置的问题
            if(ra_step == 0 && dec_step == 0)  //无需GOTO
            {
                LOCAL_KEY_CONTROL(local_key_state); //手柄控制赤道仪移动
                KEY_CONTROL_MIX(remote_key_state,local_key_state,key_state);  //手柄与遥控按键数据融合
                move_speed=SPEED_CONTROL();  //按键时赤道仪移动速度设置
                HANDLE_CONTROL( move_speed,dir_state,key_state);  //响应按键或正常速度跟踪
            }
            else  //需要GOTO
            {
                GOTO( &ra_step, &dec_step,dir_state);    //执行GOTO任务
                current_pos[0] = CURRENT_POS_RA ( target_ra, ra_step, RA_STP_ANGLE );   //更新当前指向
                current_pos[1] = CURRENT_POS_DEC ( target_dec, dec_step, DEC_STP_ANGLE );
            }
						timer_counter++;
        }
				
				if(shutter[1]!=0&&timer_counter>=30769)//快门控制
				{
					timer_counter=0;
					sec_counter++;
					if(sec_counter==5&&shutter[0]!=0) //延时5秒后，B门拍照
					{
						PCout(13)=0;
						PBout(12)=0;
					}
					if(sec_counter>=(shutter[0]+5))  //到时停止B门
					{
						PCout(13)=1;
						PBout(12)=1;
						shutter[1]--;
						sec_counter=0;
					}
				}
    }
}

