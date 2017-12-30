#ifndef __EQ3D_H
#define __EQ3D_H
#include "sys.h"

////////////////////第三版硬件接口配置////////////////////////

#define RA_DIR			PAout(15)			//RA_DIR
#define RA_STP			PBout(3)			//RA_STP
#define RA_SLEEP		PBout(4)			//RA_SLEEP
#define RA_REST			PBout(5)			//RA_REST
#define RA_MS2			PBout(6)			//RA_MS2
#define RA_MS1			PBout(7)			//RA_MS1
#define RA_MS0			PBout(8)			//RA_MS0
#define RA_EN				PBout(9)			//RA_EN

#define DEC_DIR			PBout(13)			//DEC_DIR
#define DEC_STP			PBout(14)			//DEC_STP
#define DEC_SLEEP		PBout(15)			//DEC_SLEEP
#define DEC_REST		PAout(8)			//DEC_REST
#define DEC_MS2			PAout(9)			//DEC_MS2
#define DEC_MS1			PAout(10)			//DEC_MS1
#define DEC_MS0			PAout(11)			//DEC_MS0
#define DEC_EN			PAout(12)			//DEC_EN

#define DEC_PLUS		PAin(3)				//DEC+
#define DEC_SUB			PAin(1)				//DEC-
#define RA_PLUS			PAin(7)				//RA+
#define RA_SUB			PAin(0)				//RA-
#define DIR_KEY			PAin(4)				//N/S MODE
#define SPEED_KEY		PBin(1)				//SPEED

#define SW_LED			PAout(5)			//SW pressed LED
#define PWR_LED			PAout(6)			//power display LED
#define SPEED_LED		PBout(0)			//SPEED select LED
#define DIR_LED			PAout(2)			//N/S mode LED

#define RA_STP_ANGLE		4.05		//RA步进电机一个步进时赤道仪的转角（角秒）1.730769231
#define DEC_STP_ANGLE		4.05		//DEC步进电机一个步进时赤道仪的转角（角秒）3.461538462

#define STP_INTERVAL		(65-1)	//分解质因数之后的脉冲间隔
#define NORM_STP_COUNT	8285		//正常跟踪时一个步进所需脉冲间隔数目  8285
#define MIN_STP_COUNT		10			//手柄最大速度时一个步进所需脉冲间隔数目
#define GOTO_STP_COUNT	10			//GOTO速度时一个步进所需脉冲间隔数目

#define STEPPER_MOTOR_STAR  0   //步进电机启动时EN的电平
#define RA_DIR_SET			0				//设置默认RA轴基准方向
#define DEC_DIR_SET			1				//设置默认RA轴基准方向

#define GUIDE_SPEED			1				//四种手柄速度的第一种，一般设为1，也就是X2
#define LOW_SPEED				10			//四种手柄速度的第二种
#define MEDIUM_SPEED		100			//四种手柄速度的第三种
#define HIGH_SPEED			800			//四种手柄速度的第四种

void EQ3D_GPIO_OUT_Config(void);
void EQ3D_GPIO_KEY_Config(void);
void RA_STEP_Config(u8 a);
void DEC_STEP_Config(u8 a);
void EQ3D_STEP(u8 stp_ra , u8 stp_dec);
void HANDLE_CONTROL(u16 speed, u8 *dir_state, s8 *key_statue);
void GOTO ( int *ra_step, int *dec_step , u8 *dir_state);
void LOCAL_KEY_CONTROL ( s8 *key_statue );
void KEY_CONTROL_MIX(s8 *remote_key_state,s8 *local_key_state,s8 *key_state);
u16 SPEED_CONTROL(void);
void DIR_CONTROL(u8 *dir_state);

#endif

