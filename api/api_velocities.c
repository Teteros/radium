#include "Python.h"

#include "../common/nsmtracker.h"
#include "../common/list_proc.h"
#include "../common/placement_proc.h"
#include "../common/velocities_proc.h"
#include "../common/nodelines_proc.h"
#include "../common/undo.h"
#include "../common/undo_notes_proc.h"
#include "../common/notes_proc.h"

#include "api_mouse_proc.h"

#include "api_common_proc.h"
#include "api_support_proc.h"
#include "radium_proc.h"



// velocities
//////////////////////////////////////////////////

static struct Node *get_velocitynode(int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return NULL;

  const vector_t *nodes = GetVelocityNodes(window, wblock, wtrack, note);
  if (velocitynum < 0 || velocitynum>=nodes->num_elements) {
    handleError("There is no velocity #%d in note %d in track #%d in block #%d",velocitynum, (int)note->id, tracknum, blocknum);
    return NULL;
  }
  
  return nodes->elements[velocitynum];
}


float getVelocityX(int num, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Node *node = get_velocitynode(num, dynnote, tracknum, blocknum, windownum);
  return node==NULL ? 0 : node->x;
}

float getVelocityY(int num, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Node *node = get_velocitynode(num, dynnote, tracknum, blocknum, windownum);
  return node==NULL ? 0 : node->y-get_scroll_pos();
}


float getVelocityValue(int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  struct Velocities *velocity = getVelocityFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, velocitynum);

  if (velocity==NULL){

    if (note==NULL)
      return 0;

    else if (velocitynum==0)
      return (double)note->velocity / (double)MAX_VELOCITY;

    else
      return (double)note->velocity_end / (double)MAX_VELOCITY;

  }

  return (double)velocity->velocity / (double)MAX_VELOCITY;
}

Place getVelocityPlace(int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  struct Velocities *velocity = getVelocityFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, velocitynum);

  if (velocity==NULL){

    if (note==NULL)
      return p_Create(0,0,1);

    else if (velocitynum==0)
      return note->l.p;

    else
      return note->end;

  }

  return velocity->l.p;
}

int getVelocityLogtype(int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  struct Velocities *velocity = getVelocityFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, velocitynum);

  if (velocity==NULL){

    if (note==NULL)
      return 0;

    else if (velocitynum==0)
      return note->velocity_first_logtype;

    else
      return 0; // Last logtype. Always 0. Irrelevant.
  }

  return velocity->logtype;
}

int getNumVelocities(dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return 0;

  return 2+(note->velocities==NULL ? 0 : ListFindNumElements3(&note->velocities->l));
}

static int addVelocity2(float value, Place place, dyn_t dynnote, int tracknum, int blocknum, int windownum, bool show_errors){

  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return -1;

  if (PlaceLessThan(&place, &note->l.p)) {
    handleError("addVelocity: placement before note start for note. velocity: %s. note: %s", p_ToString(place), p_ToString(note->l.p));
    return -1;
  }

  if (PlaceEqual(&place, &note->l.p)) {
    if (show_errors)
      handleError("addVelocity: placement at note start for note. velocity: %s. note: %s", p_ToString(place), p_ToString(note->l.p));
    return -1;
  }

  if (PlaceEqual(&place, &note->end)) {
    if (show_errors)
      handleError("addVelocity: placement at note end for note. velocity: %s. note end: %s", p_ToString(place), p_ToString(note->end));
    return -1;
  }

  if (PlaceGreaterThan(&place, &note->end)) {
    handleError("addVelocity: placement after note end for note. velocity: %s. note end: %s", p_ToString(place), p_ToString(note->end));
    return -1;
  }

  int ret = AddVelocity(value*MAX_VELOCITY, &place, note);

  if (ret==-1){
    if (show_errors)
      handleError("addVelocity: Can not create new velocity with the same position as another velocity");
    return -1;
  }

  
  window->must_redraw_editor = true;

  return ret+1;
}

int addVelocity(float value, Place place, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  return addVelocity2(value, place, dynnote, tracknum, blocknum, windownum, true);
}
int addVelocityDontDisplayErrors(float value, Place place, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  return addVelocity2(value, place, dynnote, tracknum, blocknum, windownum, false);
}
  
int addVelocityF(float value, float floatplace, int notenum, int tracknum, int blocknum, int windownum){
  if (floatplace < 0){
    handleError("Place can not be negative: %f", floatplace);
    return -1;
  }

  Place place;
  Float2Placement(floatplace, &place);
  return addVelocity(value, place, DYN_create_int(notenum), tracknum, blocknum, windownum);
}

dyn_t setVelocity(float value, Place place, int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return dynnote;

  struct Blocks *block = wblock->block;
  struct Tracks *track = wtrack->track;
  
  const vector_t *nodes = GetVelocityNodes(window, wblock, wtrack, note);
  if (velocitynum < 0 || velocitynum>=nodes->num_elements) {
    handleError("There is no velocity #%d in note %d in track #%d in block# %d",velocitynum, (int)note->id, tracknum, blocknum);
    return dynnote;
  }

  window->must_redraw_editor = true;

  //printf("velocitynum==%d. floatplace: %f\n",velocitynum,floatplace);

  if (velocitynum==0) {
    
    note->velocity = R_BOUNDARIES(0,value*MAX_VELOCITY,MAX_VELOCITY);
    if (!p_is_same_place(place)){
      if(place.line < 0){handleError("Negative place");return dynnote;}
      return MoveNote(block, track, note, &place, true);
    }
    
  } else if (velocitynum==nodes->num_elements-1) {
    
    note->velocity_end = R_BOUNDARIES(0,value*MAX_VELOCITY,MAX_VELOCITY);
    if (!p_is_same_place(place)){
      if(place.line < 0){handleError("Negative place");return dynnote;}
      MoveEndNote(block, track, note, &place, true);
    }
    
  } else {

    struct Velocities *velocity;

    if (p_is_same_place(place)){
      
      velocity = ListFindElement3_num(&note->velocities->l, velocitynum-1);
      
    } else {
      
      if(place.line < 0){handleError("Negative place");return dynnote;}
      
      Place firstLegalPlace,lastLegalPlace;
      PlaceFromLimit(&firstLegalPlace, &note->l.p);
      PlaceTilLimit(&lastLegalPlace, &note->end);
      
      {
        SCOPED_PLAYER_LOCK_IF_PLAYING();
        
        velocity = (struct Velocities*)ListMoveElement3_FromNum_ns(&note->velocities, velocitynum-1, &place, &firstLegalPlace, &lastLegalPlace);
        NOTE_validate(block, track, note);
      }
      
    }
    
    velocity->velocity=R_BOUNDARIES(0,value*MAX_VELOCITY,MAX_VELOCITY);
  }

  return dynnote;
}

dyn_t setVelocityF(float value, float floatplace, int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  Place place;
  
  if (floatplace < 0) {
    R_ASSERT_NON_RELEASE(false); // Don't know if this is a legal situation.
    place = g_same_place;
  }else
    Float2Placement(floatplace, &place);
  return setVelocity(value, place, velocitynum, dynnote, tracknum, blocknum, windownum);
}

void deleteVelocity(int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
 struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return;

  struct Blocks *block=wblock->block;
  struct Tracks *track=wtrack->track;

  const vector_t *nodes = GetVelocityNodes(window, wblock, wtrack, note);
  if (velocitynum < 0 || velocitynum>=nodes->num_elements) {
    handleError("There is no velocity #%d in note %d in track #%d in block #%d",velocitynum, (int)note->id, tracknum, blocknum);
    return;
  }

  bool is_first                      = velocitynum==0;
  bool is_last                       = velocitynum==nodes->num_elements-1;
  bool is_last_and_no_velocities     = is_last && nodes->num_elements==2;
  bool is_last_and_there_are_pitches = is_last && note->pitches!=NULL;

  if (is_first || is_last_and_no_velocities || is_last_and_there_are_pitches){
    {
      SCOPED_PLAYER_LOCK_IF_PLAYING();
      RemoveNote(block, track, note);
    }

  } else if (velocitynum==nodes->num_elements-1) {
    struct Velocities *last = (struct Velocities*)ListLast3((struct ListHeader3*)note->velocities);
    {
      SCOPED_PLAYER_LOCK_IF_PLAYING();
      note->end = last->l.p;
      note->velocity_end = last->velocity;
      ListRemoveElement3(&note->velocities, &last->l);
      NOTE_validate(block, track, note);
    }

  } else {
    {
      SCOPED_PLAYER_LOCK_IF_PLAYING();
      ListRemoveElement3_fromNum(&note->velocities, velocitynum-1);
      NOTE_validate(block, track, note);
    }
  }

  window->must_redraw_editor = true;
}

void setVelocityLogtype(int logtype, int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return;

  //struct Blocks *block=wblock->block;
  //struct Tracks *track=wtrack->track;

  const vector_t *nodes = GetVelocityNodes(window, wblock, wtrack, note);
  if (velocitynum < 0 || velocitynum>=nodes->num_elements) {
    handleError("There is no velocity #%d in note %d in track #%d in block #%d",velocitynum, (int)note->id, tracknum, blocknum);
    return;
  }
    
  bool is_first                      = velocitynum==0;
  bool is_last                       = velocitynum==nodes->num_elements-1;

  if (is_last) {
    handleError("Can not set logtype for last velocity (doesn't make any sense)");
    return;
  }

  if (is_first)
    note->velocity_first_logtype = logtype;
  else {
    struct Node *node = nodes->elements[velocitynum];
    struct Velocities *velocity = (struct Velocities*)node->element;

    velocity->logtype = logtype;
  }

  window->must_redraw_editor = true;
}
  

void setCurrentVelocityNode(int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
 struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return;

  const vector_t *nodes = GetVelocityNodes(window, wblock, wtrack, note);
  if (velocitynum < 0 || velocitynum>=nodes->num_elements) {
    handleError("There is no velocity #%d in note %d in track #%d in block #%d",velocitynum, (int)note->id, tracknum, blocknum);
    return;
  }

  struct Node *node = nodes->elements[velocitynum];
  struct Velocities *current = (struct Velocities*)node->element;

  API_setCurrentNode(&current->l);
}

void setIndicatorVelocityNode(int velocitynum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return;

  const vector_t *nodes = GetVelocityNodes(window, wblock, wtrack, note);
  if (velocitynum < 0 || velocitynum>=nodes->num_elements) {
    handleError("There is no velocity #%d in note %d in track #%d in block #%d",velocitynum, (int)note->id, tracknum, blocknum);
    return;
  }

  API_setIndicatorNode(&note->l);
  g_indicator_velocity_num = velocitynum;
}



