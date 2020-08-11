/* Copyright 2001-2017 Kjetil S. Matheussen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#include "Python.h"

#include "../common/nsmtracker.h"
#include "../common/list_proc.h"
#include "../common/placement_proc.h"
#include "../common/tempos_proc.h"
#include "../common/LPB_proc.h"
#include "../common/Signature_proc.h"
#include "../common/temponodes_proc.h"
#include "../common/undo_signatures_proc.h"
#include "../common/undo_lpbs_proc.h"
#include "../common/undo_tempos_proc.h"
#include "../common/undo_temponodes_proc.h"
#include "../common/undo_maintempos_proc.h"
#include "../common/undo_swings_proc.h"
#include "../common/Beats_proc.h"
#include "../common/player_pause_proc.h"
#include "../common/time_proc.h"
#include "../common/nodelines_proc.h"
#include "../common/reltempo_proc.h"

#include "../embedded_scheme/s7extra_proc.h"

#include "api_common_proc.h"
#include "api_support_proc.h"
#include "radium_proc.h"

#include "api_timing_proc.h"


/********** Signatures  **********/

void setMainSignature(int numerator, int denominator){
  if (numerator <= 0 || denominator <= 0){
    handleError("%d / %d is not a legal signature", numerator, denominator);
    return;
  }
   
  if (numerator==root->signature.numerator && denominator==root->signature.denominator)
    return;
  
  struct Tracker_Windows *window = root->song->tracker_windows;
  struct WBlocks *wblock = window->wblock;

  ADD_UNDO(MainTempo(window,wblock));

  PC_Pause();{
    root->signature = make_static_ratio(numerator, denominator);
    TIME_global_signature_has_changed();
  }PC_StopPause(window);
  
  window->must_redraw = true;
}

Place getMainSignature(void){
  return p_Create(0,(int)root->signature.numerator,(int)root->signature.denominator);
}

int numSignatures(int blocknum, int windownum){
  struct WBlocks *wblock=getWBlockFromNum(windownum,blocknum);
  if(wblock==NULL)
    return 0;

  return ListFindNumElements3((struct ListHeader3*)wblock->block->signatures);
}

int addSignature(int numerator, int denominator,
                 Place place,
                 int blocknum)
{
  struct Tracker_Windows *window;
  struct WBlocks *wblock=getWBlockFromNumA(-1,&window,blocknum);
  if(wblock==NULL)
    return -1;

  if (!PlaceLegal(wblock->block, &place)) {
    handleError("Place %s is not legal", PlaceToString(&place));
    return -1;
  }

  if (numerator <= 0 || denominator <= 0){
    handleError("%d / %d is not a legal signature", numerator, denominator);
    return -1;    
  }
  
  ADD_UNDO(Signatures_CurrPos(window));
        
  struct Signatures *signature;

  PC_Pause();{
    signature = SetSignature(wblock->block,&place,make_static_ratio(numerator, denominator));
  }PC_StopPause(window);
  
  window->must_redraw=true;

  return ListFindElementPos3(&wblock->block->signatures->l,&signature->l);
}

int addSignature3(int numerator, int denominator,
                  int line,int counter,int dividor,
                  int blocknum)
{
  Place place = {line, (uint32_t)counter, (uint32_t)dividor};
  return addSignature(numerator, denominator, place, blocknum);
}

// Return a place, for convenience. The line part of the returned value is always 0 (even if numerator > denominator). Returns -1 on error.
// TODO: Return ratio
Place getSignature(int signaturenum, int blocknum, int windownum){
  struct Signatures *signature = getSignatureFromNum(windownum, blocknum, signaturenum);
  if (signature==NULL)
    return p_Create(0,-1,1);
  else
    return p_Create(0, (int)signature->signature.numerator, (int)signature->signature.denominator);
}

/******************* LPBs *************************/

void setMainLPB(int lpb_value){
  if (lpb_value <=1)
    return;
  if (lpb_value == root->lpb)
    return;
  
  struct Tracker_Windows *window = root->song->tracker_windows;
  struct WBlocks *wblock = window->wblock;

  printf("Undo MainTempo lpb: %d\n",lpb_value);
  ADD_UNDO(MainTempo(window,wblock));

  PC_Pause();{
    root->lpb=lpb_value;
    TIME_global_LPB_has_changed();
  }PC_StopPause(window);
  
  //UpdateAllWLPBs(window);
  window->must_redraw = true;
}

int getMainLPB(void){
  return root->lpb;
}

int numLPBs(int blocknum, int windownum){
  struct WBlocks *wblock=getWBlockFromNum(windownum,blocknum);
  if(wblock==NULL)
    return 0;

  return ListFindNumElements3((struct ListHeader3*)wblock->block->lpbs);
}

int addLPB(int lpb_value,
           Place place,
           int blocknum)
{
  struct Tracker_Windows *window;
  struct WBlocks *wblock=getWBlockFromNumA(-1,&window,blocknum);
  if(wblock==NULL)
    return -1;

  if (!PlaceLegal(wblock->block, &place)) {
    handleError("Place %s is not legal", PlaceToString(&place));
    return -1;
  }

  if (lpb_value < 1){
    handleError("addBPM: lpb must be 1 or higher. Found %d", lpb_value);
    return -1;
  }
  

  ADD_UNDO(LPBs_CurrPos(window));
  
  struct LPBs *lpb = SetLPB(wblock->block,&place,lpb_value);

  window->must_redraw=true;

  return ListFindElementPos3(&wblock->block->lpbs->l,&lpb->l);
}

int addLPB3(int lpb,
            int line,int counter,int dividor,
            int blocknum
            )
{
  Place place = {line, (uint32_t)counter, (uint32_t)dividor};

  return addLPB(lpb, place, blocknum);
}

int getLPB(int num, int blocknum, int windownum){
  struct LPBs *lpb = getLPBFromNum(windownum, blocknum, num);
  if (lpb==NULL)
    return -1;
  else
    return lpb->lpb;
}

dyn_t API_getAllLPB(const struct Blocks *block){
  dynvec_t ret = {};
  struct LPBs *lpb = block->lpbs;

  while(lpb != NULL){
    hash_t *bpm = HASH_create(3);

    HASH_put_dyn(bpm, ":place", DYN_create_place(lpb->l.p));
    HASH_put_int(bpm, ":lpb", lpb->lpb);
    HASH_put_int(bpm, ":logtype", LOGTYPE_HOLD);
    //HASH_put_int(bpm, ":logtype", tempo->logtype);

    DYNVEC_push_back(&ret, DYN_create_hash(bpm));

    lpb = NextLPB(lpb);
  }

  return DYN_create_array(ret);
}

dyn_t getAllLPB(int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return DYN_create_bool(false);

  return API_getAllLPB(wblock->block);
}


/***************** BPMs *************************/

void setMainBPM(int bpm_value){
  if (bpm_value <=1)
    return;
  if (bpm_value == root->tempo)
    return;

  struct Tracker_Windows *window = root->song->tracker_windows;
  struct WBlocks *wblock = window->wblock;
  
  ADD_UNDO(MainTempo(window,wblock));

  PC_Pause();{
    root->tempo=bpm_value;
    TIME_global_tempos_have_changed();
  }PC_StopPause(window);
      
  R_ASSERT(wblock->block->beats != NULL);

  window->must_redraw = true;
}

int getMainBPM(void){
  return root->tempo;
}

int numBPMs(int blocknum, int windownum){
  struct WBlocks *wblock=getWBlockFromNum(windownum,blocknum);
  if(wblock==NULL)
    return 0;

  return ListFindNumElements3((struct ListHeader3*)wblock->block->tempos);
}

void setBPMs(dynvec_t bpms, int blocknum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock=getWBlockFromNumA(-1,&window,blocknum);
  if(wblock==NULL)
    return;

  Tempos *tempos = NULL;

  for(dyn_t dynhash : bpms){
    
    if (dynhash.type != HASH_TYPE){
      handleError("setBPMs: An element is not a hash table. Found %s instead", DYN_type_name(dynhash.type));
      return;
    }
    
    hash_t *hash = dynhash.hash;
    
    if (!HASH_has_key(hash, ":place")){
      handleError("setBPMs: An element does not contain :place");
      return;
    }

    dyn_t dynplace = HASH_get_dyn(hash, ":place");

    if (!DYN_is_ratio(dynplace)){
      handleError("setBPMs: :place must either be INT_TYPE or RATIO_TYPE, found %s", DYN_type_name(dynplace.type));
      return;
    }
    
    if (!HASH_has_key(hash, ":bpm")){
      handleError("setBPMs: An element does not contain :bpm");
      return;
    }

    dyn_t dynbpm = HASH_get_dyn(hash, ":bpm");

    if (dynbpm.type != INT_TYPE){
      handleError("setBPMs: :bpm must be INT_TYPE, found %s", DYN_type_name(dynbpm.type));
      return;
    }
    
    int logtype = LOGTYPE_HOLD;
    
    if (HASH_has_key(hash, ":logtype")){
      
      dyn_t dynlogtype = HASH_get_dyn(hash, ":logtype");
      
      if (dynlogtype.type != INT_TYPE){
        handleError("setBPMs: :logtype must be INT_TYPE, found %s", DYN_type_name(dynbpm.type));
        return;
      }

      logtype = dynlogtype.int_number;
    }

    struct Tempos *tempo=(struct Tempos*)talloc(sizeof(struct Tempos));
    tempo->l.p = DYN_get_place(dynplace);
    tempo->tempo = (int)dynbpm.int_number;
    tempo->logtype = logtype;

    if (!PlaceLegal(wblock->block, &tempo->l.p)) {
      handleError("setBPMs: Place %s is not legal", PlaceToString(&tempo->l.p));
      return;
    }

    if (tempo->tempo < 1){
      handleError("setBPMs: BPM must be 1 or higher, found %d", tempo->tempo);
      return;
    }
    
    ListAddElement3(&tempos,&tempo->l);
  }
  
  ADD_UNDO(Tempos_CurrPos(window));

  SetTempos(wblock->block, tempos);

  window->must_redraw=true;
}

int addBPM(int bpm,
           Place place,
           int blocknum)
{
  struct Tracker_Windows *window;
  struct WBlocks *wblock=getWBlockFromNumA(-1,&window,blocknum);
  if(wblock==NULL)
    return -1;

  if (!PlaceLegal(wblock->block, &place)) {
    handleError("Place %s is not legal", PlaceToString(&place));
    return -1;
  }

  if (bpm < 1){
    handleError("addBPM: bpm must be 1 or higher. Found %d", bpm);
    return -1;
  }
  
  ADD_UNDO(Tempos_CurrPos(window));

  struct Tempos *tempo = SetTempo(wblock->block,&place,bpm,LOGTYPE_HOLD);

  window->must_redraw=true;

  return ListFindElementPos3(&wblock->block->tempos->l,&tempo->l);
}

int addBPM3(int bpm,
            int line,int counter,int dividor,
            int blocknum
            )
{
  Place place = {line, (uint32_t)counter, (uint32_t)dividor};

  return addBPM(bpm, place, blocknum);
}
           
int getBPM(int num, int blocknum, int windownum){
  struct BPMs *bpm = getBPMFromNum(windownum, blocknum, num);
  if (bpm==NULL)
    return -1;
  else
    return bpm->tempo;
}

Place getBPMPlace(int num, int blocknum, int windownum){
  struct BPMs *bpm = getBPMFromNum(windownum, blocknum, num);
  if (bpm==NULL)
    return p_Create(0,0,1);
  else
    return bpm->l.p;
}

dyn_t API_getAllBPM(const struct Blocks *block){
  dynvec_t ret = {};
  struct Tempos *tempo = block->tempos;

  while(tempo != NULL){
    hash_t *bpm = HASH_create(3);

    HASH_put_dyn(bpm, ":place", DYN_create_place(tempo->l.p));
    HASH_put_int(bpm, ":bpm", tempo->tempo);
    HASH_put_int(bpm, ":logtype", tempo->logtype);
    //HASH_put_int(bpm, "logtype", tempo->logtype);

    DYNVEC_push_back(&ret, DYN_create_hash(bpm));

    tempo = NextTempo(tempo);
  }

  return DYN_create_array(ret);  
}
  
dyn_t getAllBPM(int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL){
    return DYN_create_bool(false);
  }

  return API_getAllBPM(wblock->block);
}



/************* Block tempo automation ************************/

Place getTemponodePlace(int num, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return p_Create(0,0,1);

  struct Blocks *block = wblock->block;
  struct TempoNodes *temponode = (struct TempoNodes *)ListFindElement3_num(&block->temponodes->l, num);
  if (temponode==NULL) {
    handleError("No temponode %d in block %d%s",num,blocknum,blocknum==-1?" (i.e. current block)":"");
    return p_Create(0,0,1);
  }

  if (temponode->l.next==NULL)  // don't expose weirdness to the scripting interface.
    return p_Create(block->num_lines,0,1);
  
  return temponode->l.p;
}

float getTemponodeValue(int num, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return 0.0;

  struct Blocks *block = wblock->block;
  struct TempoNodes *temponode = (struct TempoNodes *)ListFindElement3_num(&block->temponodes->l, num);
  if (temponode==NULL) {
    handleError("No temponode %d in block %d%s",num,blocknum,blocknum==-1?" (i.e. current block)":"");
    return 0.0;
  }

  return temponode->reltempo;
}

void undoTemponodes(void){
  struct Tracker_Windows *window = root->song->tracker_windows;
  ADD_UNDO(TempoNodes_CurrPos(window));
}

void setTemponode(int num, double value, Place place, int blocknum, int windownum){

  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return;
  
  if(value < -1000000 || value > 1000000){
    handleError("setTemponode: value too extreme: %f", value);
    return;
  }
    
  //printf("Set temponode. value: %f. Place: %f\n",value,floatplace);
  
  struct Blocks *block = wblock->block;

  struct TempoNodes *temponode;

  const vector_t *tempo_nodes = GetTempoNodes(window, wblock);

  if (num==0)
    temponode = block->temponodes; // don't want to set placement for the first node. It's always at top.
  
  else if (num==tempo_nodes->num_elements-1)
    temponode = (struct TempoNodes *)ListLast3(&block->temponodes->l); // don't want to set placement for the last node. It's always at bottom.

  else if (num < 0 || num>=tempo_nodes->num_elements) {
    handleError("No temponode %d in block %d%s",num,blocknum,blocknum==-1?" (i.e. current block)":"");
    return;
    
  } else if (p_is_same_place(place)) {    
    temponode = (struct TempoNodes *)ListFindElement3_num(&block->temponodes->l, num);

  } else {
    if(place.line < 0){handleError("Negative place");}
    temponode = (struct TempoNodes *)ListMoveElement3_FromNum_ns(&block->temponodes, num, &place, NULL, NULL);
    R_ASSERT_RETURN_IF_FALSE(temponode!=NULL);
  }
  
  adjust_reltempomax(wblock, value);
  
  temponode->reltempo = value;
  TIME_block_tempos_have_changed(wblock->block);
  
  //printf("before: %f, now: %f\n",floatplace, GetfloatFromPlace(&temponode->l.p));

  window->must_redraw_editor = true;
}

int getNumTemponodes(int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return 0;
  
  const vector_t *tempo_nodes = GetTempoNodes(window, wblock);
  return tempo_nodes->num_elements;
}

void deleteTemponode(int num, int blocknum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(-1, &window, blocknum);
  if (wblock==NULL)
    return;

  const vector_t *tempo_nodes = GetTempoNodes(window, wblock);

  if (num < 0 || num >= tempo_nodes->num_elements){
    handleError("deleteTemponode: No temponode %d in block %d",num,blocknum);
    return;
  }

  if (num==0){
    wblock->block->temponodes->reltempo = 0.0f;
  } else if (num==tempo_nodes->num_elements-1) {
    struct TempoNodes *last = (struct TempoNodes *)ListLast3(&wblock->block->temponodes->l);
    last->reltempo = 0.0f;
  } else {
    ListRemoveElement3_fromNum(&wblock->block->temponodes,num);
  }

  TIME_block_tempos_have_changed(wblock->block);
    
  window->must_redraw_editor = true;
}

int addTemponode(float value, Place place, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL){
    handleError("addTemponode: No block %d in window %d",blocknum,windownum);
    return -1;
  }
  
  struct Blocks *block = wblock->block;
  
  if (place.line < 0)
    return -1;

  if (place.line >= block->num_lines)
    return -1;

  adjust_reltempomax(wblock, value);

  struct TempoNodes *temponode = AddTempoNode(window,wblock,&place,value); // addtemponode pauses player

  if (temponode==NULL)
    return -1;
  
  //GFX_SetChangeFloat(window,wblock,"Reltempo",RelTempo2RealRelTempo(Gfx2RelTempo(wblock,dx)));
  //UpdateSTimes(wblock->block);
  //GFX_DrawStatusBar(window,wblock);

  window->must_redraw_editor = true;

  return ListFindElementPos3(&block->temponodes->l, &temponode->l);
}

dyn_t API_getAllTemponodes(const struct Blocks *block){
  dynvec_t ret = {};
  struct TempoNodes *temponode = block->temponodes;

  while(temponode != NULL){
    hash_t *hash = HASH_create(3);

    const Place p = temponode->l.next==NULL ? p_Create(block->num_lines,0,1) : temponode->l.p; // don't expose weirdness to the scripting interface.
    
    HASH_put_dyn(hash, ":place", DYN_create_place(p));
    HASH_put_float(hash, ":tempo-multiplier", RelTempo2RealRelTempo(temponode->reltempo));
    HASH_put_int(hash, ":logtype", LOGTYPE_LINEAR);
    //HASH_put_int(bpm, "logtype", tempo->logtype);

    DYNVEC_push_back(&ret, DYN_create_hash(hash));

    temponode = NextTempoNode(temponode);
  }

  return DYN_create_array(ret);
}
  
dyn_t getAllTemponodes(int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return DYN_create_bool(false);

  return API_getAllTemponodes(wblock->block);
}



/************* Beats ************************/

dyn_t API_getAllBeats(const struct Beats *beats){
  dynvec_t ret = {};
  const struct Beats *beat = beats;

  while(beat != NULL){
    hash_t *hash = HASH_create(3);

    HASH_put_dyn(hash, ":place", DYN_create_place(beat->l.p));
    HASH_put_int(hash, ":barnum", beat->bar_num);
    HASH_put_int(hash, ":beatnum", beat->beat_num);

    //printf("API_getAllBeats: %s\n", p_ToString(beat->l.p));
    
    DYNVEC_push_back(&ret, DYN_create_hash(hash));

    beat = NextBeat(beat);
  }

  return DYN_create_array(ret);
}
  
dyn_t getAllBeats(int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return DYN_create_bool(false);

  return API_getAllBeats(wblock->block->beats);
}



/************* Swing ************************/

static dyn_t create_swing(const Place place, int weight, int logtype){
  dynvec_t swing = {};
  DYNVEC_push_back(&swing, DYN_create_place(place));
  DYNVEC_push_back(&swing, DYN_create_int(weight));
  DYNVEC_push_back(&swing, DYN_create_int(logtype));
  return DYN_create_array(swing);
}

dyn_t API_getAllBlockSwings(const struct Blocks *block){
  dynvec_t ret = {};

  struct Swing *swing = block->swings;

  while(swing != NULL){
    DYNVEC_push_back(&ret, create_swing(swing->l.p, swing->weight, swing->logtype));
    swing = NextSwing(swing);
  }
  
  return DYN_create_array(ret);
}
  
dyn_t getAllBlockSwings(int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock = getWBlockFromNumA(windownum, &window, blocknum);
  if (wblock==NULL)
    return DYN_create_bool(false);

  return API_getAllBlockSwings(wblock->block);
}

dyn_t API_getAllTrackSwings(const struct Tracks *track){
  dynvec_t ret = {};

  struct Swing *swing = track->swings;

  while(swing != NULL){
    DYNVEC_push_back(&ret, create_swing(swing->l.p, swing->weight, swing->logtype));
    swing = NextSwing(swing);
  }
  
  return DYN_create_array(ret);
}
  
dyn_t getTrackSwings(int tracknum, int blocknum, int windownum){
  struct WTracks *wtrack = getWTrackFromNum(windownum, blocknum, tracknum);
  if (wtrack==NULL)
    return DYN_create_bool(false);

  return API_getAllTrackSwings(wtrack->track);
}

template<class T>
static T *getList3FromDynvec(struct Blocks *block,
                             dynvec_t dynvec,
                             std::function<T*(const dyn_t&)> get_T)
{
  T *ret = NULL;
  
  for(dyn_t dyn : dynvec){
    T *t = get_T(dyn);
    if (t==NULL)
      return NULL;

    if (!PlaceLegal(block, &t->l.p)){
      handleError("Illegal position: %s", PlaceToString(&t->l.p));
      return NULL;
    }
    
    T *already = (struct Swing*)ListFindElement3(LCAST(ret),&t->l.p);
    if (already!=NULL && p_Equal(already->l.p, t->l.p)){
      handleError("Two elements are placed on the same position");
      return NULL;
    }

    ListAddElement3(&ret, LCAST(t));
  }

  return ret;
}

void setTrackSwings(dynvec_t swings, int tracknum, int blocknum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack=getWTrackFromNumA(-1,&window, blocknum, &wblock, tracknum);
  if(wtrack==NULL)
    return;

  struct Blocks *block = wblock->block;
  struct Tracks *track = wtrack->track;
  
  Swing *swing = getList3FromDynvec<Swing>(wblock->block, swings, [](const dyn_t &dyn){
      if (dyn.type != ARRAY_TYPE){
        handleError("setTrackSwings: An element is not an array");
        return static_cast<Swing*>(nullptr);
      }
      
      dynvec_t *s = dyn.array;
      
      if (s->num_elements != 3){
        handleError("setTrackSwings: An element does not contain 3 elements, but %d", s->num_elements);
        return static_cast<Swing*>(nullptr);
      }
      
      if (!DYN_is_ratio(s->elements[0])){
        handleError("setTrackSwings: Place of an element is not a place, but a %s", DYN_type_name(s->elements[0]));
        return static_cast<Swing*>(nullptr);
      }
          
      if (s->elements[1].type != INT_TYPE){
        handleError("setTrackSwings: Weight of an element is not an integer, but a %s", DYN_type_name(s->elements[1]));
        return static_cast<Swing*>(nullptr);
      }
          
      if (s->elements[2].type != INT_TYPE){
        handleError("setTrackSwings: Logtype of an element is not an integer, but a %s", DYN_type_name(s->elements[2]));
        return static_cast<Swing*>(nullptr);
      }
          
      struct Swing *swing = (struct Swing*)talloc(sizeof(struct Swing));
      
      swing->l.p = DYN_get_place(s->elements[0]);
      swing->weight = (int)s->elements[1].int_number;
      swing->logtype = (int)s->elements[2].int_number;

      if (swing->weight <= 0){
        handleError("setTrackSwings: Illegal swing weight: %d", swing->weight);
        return static_cast<Swing*>(nullptr);
      }
      
      return swing;
    });

  if (swing != NULL){
    ADD_UNDO(Swings_CurrPos(window, track));
    track->swings = swing;
    TIME_block_swings_have_changed(block);
  }
}

/*
#include "../embedded_scheme/scheme_proc.h"
dyn_t testsomething(dyn_t arg){
#if 0
  SCHEME_throw("testsomething-symbol", "testsomething-message");
  printf(" !!!!!!!!!!!!!!!!!   IM HERE !!!!!!!!!!!!!!!!!!\n");
  return DYN_create_int(0);
#endif
  return SCHEME_get_history();
}
*/




/*
dyn_t testsomething(dyn_t arg){
  hash_t *hash = HASH_create(5);
  HASH_put_int(hash,":a", 1);
  HASH_put_int(hash,":b", 2);

  hash_t *hash2 = HASH_create(2);
  HASH_put_int(hash2, ":d", 3);

  HASH_put_hash(hash, ":c", hash2);

  dynvec_t dynvec = {};
  DYNVEC_push_back(&dynvec, DYN_create_int(8));
  DYNVEC_push_back(&dynvec, DYN_create_int(9));

  HASH_put_array(hash, ":e", dynvec);

  HASH_put_dyn(hash, ":dyn", arg);

  return DYN_create_hash(hash);
}
*/


