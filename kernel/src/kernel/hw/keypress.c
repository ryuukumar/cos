#include <kernel/hw/keypress_map.h>

int kp_isaction (unsigned char kp) { return kb_action_escape <= kp && kp <= kb_action_delete; }
int kp_isext (unsigned char kp) { return kp == kb_action_ext; }
int kp_ischar (unsigned char kp) { return kp != 0 && !kp_isaction (kp) && !kp_isext (kp); }
