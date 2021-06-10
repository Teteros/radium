#include "Python.h"

#include "../common/nsmtracker.h"
#include "../common/list_proc.h"
#include "../common/placement_proc.h"
#include "../common/pitches_proc.h"
#include "../common/nodelines_proc.h"
#include "../common/undo.h"
#include "../common/undo_notes_proc.h"
#include "../common/notes_proc.h"

#include "api_mouse_proc.h"

#include "api_common_proc.h"
#include "api_support_proc.h"
#include "radium_proc.h"



// pitches
//////////////////////////////////////////////////

float getPitchValue(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  struct Pitches *pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (pitch==NULL){
    if (note==NULL)
      return 0;

    else if (pitchnum==0)
      return note->note;

    // No, we do return 0 for pitch_end if pitch_end is actually 0.
    //
    //else if (note->pitch_end==0 && note->pitches==NULL)
    //  return note->note;

    else
      return note->pitch_end;
  }

  return pitch->note;
}

Place getPitchPlace(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  struct Pitches *pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (pitch==NULL){

    if (note==NULL)
      return p_Create(0,0,1);

    else if (pitchnum==0)
      return note->l.p;

    else
      return ratio2place(note->end);

  }

  return pitch->l.p;
}

int getPitchLogtype(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  struct Pitches *pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (pitch==NULL){

    if (note==NULL)
      return 0;

    else if (pitchnum==0) {
      /*
      if (note->pitch_end==0.0)
        return LOGTYPE_HOLD;
      else
      */
        return note->pitch_first_logtype;

    } else
      return 0; // Last logtype. Always 0. Irrelevant.
  }

  return pitch->logtype;
}

float getPitchChance(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  struct Pitches *pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (pitch==NULL){

    if (note==NULL)
      return 0;

    else if (pitchnum==0)
      return (double)note->chance / 256.0;

    else
      return 1; // Last chance. Always 1.
  }

  return (double)pitch->chance / 256.0;
}

int getNumPitches(dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return 0;

  return 2+ListFindNumElements3((struct ListHeader3*)note->pitches);
}

int addPitch(float value, Place place, dyn_t dynnote, int tracknum, int blocknum, int windownum){

  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return -1;

  if (PlaceLessOrEqual(&place, &note->l.p)) {
    //if (dynnote>0)
    handleError("addPitch: placement before note start for note. pitch: %s. note: %s", p_ToString(place), p_ToString(note->l.p));
    return -1;
  }

  if (place2ratio(place) > note->end) {
    handleError("addPitch: placement after note end for note. pitch: %s. note end: %s", p_ToString(place), p_ToString(ratio2place(note->end)));
    return -1;
  }

  struct Pitches *pitch = AddPitch(window, wblock, wtrack, note, &place, value);
  if (pitch==NULL){
    handleError("addPitch: Can not create new pitch with the same position as another pitch");
    return -1;
  }

  window->must_redraw_editor = true;

  return ListPosition3(&note->pitches->l, &pitch->l) + 1;
}

int addPitchF(float value, float floatplace, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  if (floatplace < 0){
    handleError("Place can not be negative: %f", floatplace);
    return -1;
  }

  Place place;
  Float2Placement(floatplace, &place);
  return addPitch(value, place, dynnote, tracknum, blocknum, windownum);
}


dyn_t setPitch(float value, Place place, int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return dynnote;

  struct Blocks *block = wblock->block;
  struct Tracks *track = wtrack->track;

  if (value < 0.001){
    handleError("setPitch: Pitch less than 0.001: %f\n", value);
    return dynnote;
  }

  if (value > 127)
    value = 127;
  
  const vector_t *nodes = GetPitchNodes(window, wblock, wtrack, note);
  if (pitchnum < 0 || pitchnum>=nodes->num_elements) {
    handleError("There is no pitch #%d in note %d in track #%d in block #%d",pitchnum, (int)note->id, tracknum, blocknum);
    return dynnote;
  }

  window->must_redraw_editor = true;

  //printf("pitchnum==%d. floatplace: %f\n",pitchnum,floatplace);

  if (pitchnum==0) {
    
    note->note = value;
    if (!p_is_same_place(place)){
      if(place.line < 0){handleError("Negative place");return dynnote;}
      return MoveNote(block, track, note, &place, true);
    }
    
  } else if (pitchnum==nodes->num_elements-1) {
    
    note->pitch_end = value;
    if (!p_is_same_place(place)){
      if(place.line < 0){handleError("Negative place");return dynnote;}
      MoveEndNote(block, track, note, &place, true);
    }
    
  } else {

    struct Pitches *pitch;

    if (p_is_same_place(place)){
      pitch = (struct Pitches*)ListFindElement3_num(&note->pitches->l, pitchnum-1);
    } else {
      if(place.line < 0){handleError("Negative place");return dynnote;}
      Place firstLegalPlace,lastLegalPlace;
      PlaceFromLimit(&firstLegalPlace, &note->l.p);

      Place noteend = ratio2place(note->end);
      PlaceTilLimit(&lastLegalPlace, &noteend);
      
      {
        SCOPED_PLAYER_LOCK_IF_PLAYING();
        pitch = (struct Pitches*)ListMoveElement3_FromNum_ns(&note->pitches, pitchnum-1, &place, &firstLegalPlace, &lastLegalPlace);
        NOTE_validate(block, track, note);
      }
    }
    
    pitch->note=value;
  }

  return dynnote;
}

dyn_t setPitchF(float value, float floatplace, int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  Place place;
  
  if (floatplace < 0) {

    R_ASSERT_NON_RELEASE(false); // Don't know if this is a legal situation.
    place = g_same_place;
    
  }else {
    
    Float2Placement(floatplace, &place);
    
  }
  
  return setPitch(value, place, pitchnum, dynnote, tracknum, blocknum, windownum);
}


void deletePitch(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  deletePianonote(pitchnum, dynnote, tracknum, blocknum, windownum); // Think this is correct.
}

void setPitchLogtype(int logtype, int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  setPianonoteLogtype(logtype, pitchnum, dynnote, tracknum, blocknum, windownum); // Think this is correct
}




