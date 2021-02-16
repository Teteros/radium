#ifndef RADIUM_COMMON_SEQBLOCK_AUTOMATION_PROC_H
#define RADIUM_COMMON_SEQBLOCK_AUTOMATION_PROC_H


extern LANGSPEC void SEQBLOCK_calculate_time_conversion_table(struct SeqBlock *seqblock, bool seqblock_is_live);

static inline double get_stretch_from_automation(double value) {
  if (value < 0.0)
    return 1.0 / (1.0 - value);
  else
    return value + 1.0;
}

static inline double get_speed_from_automation(double value) {
  if (value < 0.0)
    return 1.0 - value;
  else
    return 1.0 / (1.0 + value);
}


static inline int64_t get_stretch_automation_sample_pos(const struct SeqBlock *seqblock, int64_t sample_pos){  
  if (seqblock->conversion_table.array==NULL)
    return sample_pos;

  int pos = sample_pos / RADIUM_BLOCKSIZE;
  int size = seqblock->conversion_table.num_elements;

  if (pos <= 0){

    R_ASSERT_NON_RELEASE(pos==0);
    return 0;

  }

  if (pos > size){

    R_ASSERT_NON_RELEASE(false);
    pos = size;

  }

  return seqblock->conversion_table.array[pos] / seqblock->conversion_table.stretchspeed_automation_compensation;
}

extern LANGSPEC void RT_SEQBLOCK_AUTOMATION_called_before_scheduler(void);
extern LANGSPEC void RT_SEQBLOCK_AUTOMATION_called_before_editor(struct SeqTrack *seqtrack);
extern LANGSPEC void RT_SEQBLOCK_AUTOMATION_called_after_scheduler_and_before_audio(void);
extern LANGSPEC void RT_SEQBLOCK_AUTOMATION_called_when_player_stopped(void);

#ifdef __cplusplus

struct SeqblockAutomation *SEQBLOCK_AUTOMATION_create(const struct SeqTrack *seqtrack, struct SeqBlock *seqblock, enum Seqblock_Automation_Type sat, const dyn_t automation_state, double state_samplerate);
void SEQBLOCK_AUTOMATION_free(struct SeqblockAutomation *seqblockenvelope);
double SEQBLOCK_AUTOMATION_get_min_value(const struct SeqblockAutomation *seqblockenvelope);
double SEQBLOCK_AUTOMATION_get_default_value(const struct SeqblockAutomation *seqblockenvelope);
double SEQBLOCK_AUTOMATION_get_max_value(const struct SeqblockAutomation *seqblockenvelope);
const char *SEQBLOCK_AUTOMATION_get_display_string(struct SeqblockAutomation *seqblockenvelope, double value);
double SEQBLOCK_AUTOMATION_get_value(struct SeqblockAutomation *seqblockenvelope, int nodenum);
double SEQBLOCK_AUTOMATION_get_value_for_time(struct SeqblockAutomation *seqblockenvelope, int64_t time);
double SEQBLOCK_AUTOMATION_get_seqtime(struct SeqblockAutomation *seqblockenvelope, int nodenum);
int SEQBLOCK_AUTOMATION_get_logtype(struct SeqblockAutomation *seqblockenvelope, int nodenum);
int SEQBLOCK_AUTOMATION_get_num_nodes(struct SeqblockAutomation *seqblockenvelope);
int SEQBLOCK_AUTOMATION_add_node(struct SeqblockAutomation *seqblockenvelope, double seqtime, double db, int logtype);
void SEQBLOCK_AUTOMATION_delete_node(struct SeqblockAutomation *seqblockenvelope, int nodenum);
void SEQBLOCK_AUTOMATION_set_curr_node(struct SeqblockAutomation *seqblockenvelope, int nodenum);
void SEQBLOCK_AUTOMATION_cancel_curr_node(struct SeqblockAutomation *seqblockenvelope);
void SEQBLOCK_AUTOMATION_set_curr_automation(struct SeqTrack *seqtrack, struct SeqBlock *seqblock, struct SeqblockAutomation *seqblockenvelope);

// May be called if it there is no current automation.
void SEQBLOCK_AUTOMATION_cancel_curr_automation(void);
void SEQBLOCK_AUTOMATION_set(struct SeqblockAutomation *seqblockenvelope, int nodenum, double seqtime, double db, int lotype);
void SEQBLOCK_AUTOMATION_default_duration_changed(struct SeqblockAutomation *seqblockenvelope, int64_t new_default_duration, radium::PlayerLockOnlyIfNeeded *lock); // safe to call even if default duration hasn't changed.

////////////////////////

void RT_SEQBLOCK_AUTOMATION_called_when_player_stopped(void);
dyn_t SEQBLOCK_AUTOMATION_get_state(const struct SeqblockAutomation *seqblockenvelope);
void SEQBLOCK_AUTOMATION_apply_state(struct SeqblockAutomation *seqblockenvelope, const dyn_t envelope_state, double state_samplerate);
float SEQBLOCK_AUTOMATION_get_node_x(const struct SeqblockAutomation *seqblockenvelope, int nodenum);
float SEQBLOCK_AUTOMATION_get_node_y(const struct SeqblockAutomation *seqblockenvelope, int seqtracknum, int nodenum);
const radium::AutomationPainter *SEQBLOCK_AUTOMATION_get_painter(struct SeqblockAutomation *seqblockenvelope, float x1, float y1, float x2, float y, bool paint_nodes, float seqblock_x1, float seqblock_x2);

#include "SeqAutomation.hpp"

const radium::SeqAutomation<radium::AutomationNode> &SEQBLOCK_AUTOMATION_get_SeqAutomation(const struct SeqblockAutomation *automation);

bool RT_seqblock_automation_is_enabled(const struct SeqblockAutomation *automation);
void SEQBLOCK_AUTOMATION_set_enabled(struct SeqblockAutomation *automation, bool enabled);
bool RT_maybe_get_seqblock_automation_value(struct SeqblockAutomation *automation, double time, double &value);

#endif

#endif
