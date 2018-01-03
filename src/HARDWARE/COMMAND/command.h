#ifndef __COMMAND_H
#define	__COMMAND_H

#include "stm32f10x.h"


u8 LX200( u8 *buffer,s32 *current_pos,s32 *target_pos );
int RA_STEP_CALCULATE( s32 current_ra, s32 target_ra ,double stp_angle);
int DEC_STEP_CALCULATE( s32 current_dec, s32 target_dec ,double stp_angle);
s32 CURRENT_POS_RA( s32 target_ra ,int ra_step ,double stp_angle);
s32 CURRENT_POS_DEC( s32 target_dec ,int dec_step , double stp_angle);
u32 RA_DEG_INTO_ARCSEC(u8 ra[6]);
u32 DEC_DEG_INTO_ARCSEC(u8 dec[7]);
void RA_ARCSEC_INTO_DEG(s32 current_ra,u8 *raout);
void DEC_ARCSEC_INTO_DEG ( s32 current_dec, u8 *dec_out );
u8 GOTO_CHECK( u8 decode_state);
void REMOTE_KEY_CONTROL ( s8 *remote_key_state ,  u8 decode_state );
void SHUTTER_CONTROL( u8 *buffer, u16 *shutter );

#endif /* __COMMAND_H */
