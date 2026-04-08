#ifndef __PCBPLC_INTERFACE_H__
#define __PCBPLC_INTERFACE_H__

//  Device driver
#define WR_VALUE    (128)
#define RD_VALUE    (64)

int initialize_hwdriver();
void set_ao(unsigned char iAoNum, float iPercentage);
void get_di_status();
void get_ai_status();
void set_do();
void set_dual_Do();
void set_pulse_do_polarity();
void set_do_on(unsigned int iDoNum);
void set_do_off(unsigned int iDoNum);
void get_rs485_parameters();

#endif
