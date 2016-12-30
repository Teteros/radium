
#ifndef _RADIUM_COMMON_SEQTRACK_AUTOMATION_PROC_H
#define _RADIUM_COMMON_SEQTRACK_AUTOMATION_PROC_H

extern LANGSPEC struct SeqtrackAutomation *SEQTRACK_AUTOMATION_create(struct SeqTrack *seqtrack, const hash_t *automation_state);
extern LANGSPEC void SEQTRACK_AUTOMATION_free(struct SeqtrackAutomation *seqtrackautomation);
extern LANGSPEC int SEQTRACK_AUTOMATION_add_automation(struct SeqtrackAutomation *seqtrackautomation, struct Patch *patch, int effect_num, double seqtime1, double value1, int logtype, double seqtime2, double value2);
extern LANGSPEC int SEQTRACK_AUTOMATION_get_num_automations(struct SeqtrackAutomation *seqtrackautomation);
extern LANGSPEC struct Patch *SEQTRACK_AUTOMATION_get_patch(struct SeqtrackAutomation *seqtrackautomation, int automationnum);
extern LANGSPEC int SEQTRACK_AUTOMATION_get_effect_num(struct SeqtrackAutomation *seqtrackautomation, int automationnum);
extern LANGSPEC double SEQTRACK_AUTOMATION_get_value(struct SeqtrackAutomation *seqtrackautomation, int automationnum, int nodenum);
extern LANGSPEC double SEQTRACK_AUTOMATION_get_seqtime(struct SeqtrackAutomation *seqtrackautomation, int automationnum, int nodenum);
extern LANGSPEC int SEQTRACK_AUTOMATION_get_logtype(struct SeqtrackAutomation *seqtrackautomation, int automationnum, int nodenum);
extern LANGSPEC int SEQTRACK_AUTOMATION_get_num_nodes(struct SeqtrackAutomation *seqtrackautomation, int automationnum);
extern LANGSPEC int SEQTRACK_AUTOMATION_add_node(struct SeqtrackAutomation *seqtrackautomation, int automationnum, double seqtime, double value, int logtype);
extern LANGSPEC void SEQTRACK_AUTOMATION_delete_node(struct SeqtrackAutomation *seqtrackautomation, int automationnum, int nodenum);
extern LANGSPEC void SEQTRACK_AUTOMATION_set_curr_node(struct SeqtrackAutomation *seqtrackautomation, int automationnum, int nodenum);
extern LANGSPEC void SEQTRACK_AUTOMATION_cancel_curr_node(struct SeqtrackAutomation *seqtrackautomation, int automationnum);
extern LANGSPEC void SEQTRACK_AUTOMATION_set_curr_automation(struct SeqtrackAutomation *seqtrackautomation, int automationnum);
extern LANGSPEC void SEQTRACK_AUTOMATION_cancel_curr_automation(void);
extern LANGSPEC int SEQTRACK_AUTOMATION_get_curr_automation(struct SeqtrackAutomation *seqtrackautomation);
extern LANGSPEC void SEQTRACK_AUTOMATION_set(struct SeqtrackAutomation *seqtrackautomation, int automationnum, int nodenum, double seqtime, double value, int logtype);
extern LANGSPEC void RT_SEQTRACK_AUTOMATION_called_per_block(struct SeqTrack *seqtrack);
extern LANGSPEC void RT_SEQTRACK_AUTOMATION_called_when_player_stopped(void);
extern LANGSPEC hash_t *SEQTRACK_AUTOMATION_get_state(struct SeqtrackAutomation *seqtrackautomation);
extern LANGSPEC float SEQTRACK_AUTOMATION_get_node_x(struct SeqtrackAutomation *seqtrackautomation, struct SeqTrack *seqtrack, int automationnum, int nodenum);
extern LANGSPEC float SEQTRACK_AUTOMATION_get_node_y(struct SeqtrackAutomation *seqtrackautomation, int seqtracknum, int automationnum, int nodenum);

#ifdef USE_QT4
extern void SEQTRACK_AUTOMATION_paint(QPainter *p, struct SeqtrackAutomation *seqtrackautomation, float x1, float y1, float x2, float y2, double start_time, double end_time);
#endif

#endif
