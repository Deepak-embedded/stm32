#ifndef __PCBPLC_CONFIG_H__
#define __PCBPLC_CONFIG_H__

void pcbplc_config_log(pcbplcCnfg_t *ptr);
bool pcbplc_default_configuration(pcbplcCnfg_t* ptr);
int pcbplc_config_json_parsing(pcbplcCnfg_t* ptr);
int pcbplc_json_parse(const char *pBuffer, pcbplcCnfg_t* ptr);

#endif
