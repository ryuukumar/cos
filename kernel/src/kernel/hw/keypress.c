#include <kclib/ctype.h>
#include <kernel/hw/keypress_map.h>

int kp_isaction (unsigned char kp) {
	return kb_action_escape <= keypress_to_char_or_action[kp] &&
		   keypress_to_char_or_action[kp] <= kb_action_delete;
}
int kp_isext (unsigned char kp) { return kp == kb_action_ext; }
int kp_ischar (unsigned char kp) { return kp != 0 && !kp_isaction (kp) && !kp_isext (kp); }
int kp_islk (unsigned char kp) {
	return keypress_to_char_or_action[kp] == kb_action_caps ||
		   keypress_to_char_or_action[kp] == kb_action_numlk ||
		   keypress_to_char_or_action[kp] == kb_action_scrllk;
}

unsigned char map_keypress (statemachine_t* sm, unsigned char kp) {
	if (kp == 0) return 0;
	if (sm->ext) {
		sm->ext = false;
		if (kp >= kb_ps2_sc1_released_offset) {
			unsigned char released = kp - kb_ps2_sc1_released_offset;
			if (ext_keypress_to_action[released] == kb_action_ralt)
				sm->ralt = false;
			else if (ext_keypress_to_action[released] == kb_action_rctl)
				sm->rctl = false;
			return 0;
		}
		unsigned char mapped = ext_keypress_to_action[kp];
		if (ext_keypress_to_action[mapped] == kb_action_ralt)
			sm->ralt = true;
		else if (ext_keypress_to_action[mapped] == kb_action_rctl)
			sm->rctl = true;
		return mapped;
	}

	if (kp_isext (kp)) {
		sm->ext = true;
		return 0;
	}

	if (kp >= kb_ps2_sc1_released_offset) {
		unsigned char released = keypress_to_char_or_action[kp - kb_ps2_sc1_released_offset];
		if (released == kb_action_lshift)
			sm->lshift = false;
		else if (released == kb_action_rshift)
			sm->rshift = false;
		else if (released == kb_action_lalt)
			sm->lalt = false;
		else if (released == kb_action_lctl)
			sm->lctl = false;
		return 0;
	}

	if (kp_isaction (kp)) {
		unsigned char mapped = keypress_to_char_or_action[kp];
		if (mapped == kb_action_lshift)
			sm->lshift = true;
		else if (mapped == kb_action_rshift)
			sm->rshift = true;
		else if (mapped == kb_action_lalt)
			sm->lalt = true;
		else if (mapped == kb_action_lctl)
			sm->lctl = true;
		else if (mapped == kb_action_caps)
			sm->caps = !sm->caps;
		else if (mapped == kb_action_numlk)
			sm->numlk = !sm->numlk;
		else if (mapped == kb_action_scrllk)
			sm->scrllk = !sm->scrllk;
		return mapped;
	}

	if (kp_ischar (kp)) {
		int offset = 128 * (sm->lshift || sm->rshift);
		if (isalpha (keypress_to_char_or_action[kp]))
			offset = 128 * (sm->caps ^ (sm->lshift || sm->rshift));
		return keypress_to_char_or_action[offset + kp];
	}

	return 0;
}
