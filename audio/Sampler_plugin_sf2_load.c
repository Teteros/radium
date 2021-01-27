
#include "SoundFonts_proc.h"

#include "../common/hashmap_proc.h"

static bool load_sf2_instrument(Data *data, filepath_t filename, int preset_bag_number, bool set_loop_on_off){
  EVENTLOG_add_event(talloc_format("load_sf2_instrument -%S-", filename.id));
    
  hash_t *info = SF2_get_info(filename);
  if(info==NULL){
    //GFX_Message(NULL, "Unable to open soundfont file \"%S\"\n", filename.id);
    return false;
  }

  //HASH_save(info,stdout);
  hash_t *sample_infos = HASH_get_hash(info,"samples");

  hash_t *presets = HASH_get_hash(info,"presets");
  hash_t *preset = HASH_get_hash_at(presets, "", preset_bag_number);
  if(preset==NULL){
    GFX_Message(NULL, "No such preset number %d in instrument \"%S\"\n", preset_bag_number, filename.id);
    return false;
  }

  int bank_num = HASH_get_int32(preset,"bank");

  hash_t *instrument = NULL;

  // Try to find an instrument from the region. A preset may use several instruments, but that's not supported yet. We just use the first and best/worst.
  {
    hash_t *instruments = HASH_get_hash(info,"instruments");
    hash_t *regions = HASH_get_hash(preset,"regions");

    int i;
    for(i=0;i<HASH_get_array_size(regions, "");i++){
      hash_t *region = HASH_get_hash_at(regions,"",i);

      if(HASH_has_key(region,"instrument")==true){
        instrument = HASH_get_hash(instruments,HASH_get_chars(region,"instrument"));
        break;
      }
    }
  }

  if(instrument==NULL){
    GFX_Message(NULL, "load_sf2_instrument: Preset \"%S\" (bank %d / preset %d) in \"%S\" doesn't point to an instrument\n",
                HASH_get_string(preset,"name"), bank_num, HASH_get_int32(preset,"num"), filename.id
                );
    return false;
  }

  hash_t *regions = HASH_get_hash(instrument, "regions");

  for(int i=0;i<128;i++){
    Note *note = new Note;
    data->notes[i] = note;
    data->note_storage.push_back(note);
  }
  
  int num_samples=0;

  int i;
  for(i=0;i<HASH_get_array_size(regions, "");i++){
    hash_t *region = HASH_get_hash_at(regions,"",i);
    
    const char *sample_name = HASH_get_chars(region, "sample_name");
    if(strcmp(sample_name,"<no sample!>")){
      hash_t *sample_info = HASH_get_hash(sample_infos, sample_name);
      int sample_num = HASH_get_int32(sample_info,"num");

      Sample &sample = data->samples[num_samples++];
      new (&sample) Sample();
        
      sample.data = data;

      sample.volume = 1.0f;
      sample.num_frames = HASH_get_int(sample_info,"num_frames");

      set_legal_loop_points(sample,-1,-1, set_loop_on_off); // By default, loop all.
      set_legal_loop_points(sample,
                            HASH_get_int(sample_info,"loop start"),
                            HASH_get_int(sample_info,"loop end"),
                            set_loop_on_off
                            );

      printf("Loop start / end: %d %d\n",(int)sample.loop_start,(int)sample.loop_end);
      
      {
        sample.ch = -1;
        const char *type = HASH_get_chars(sample_info,"type");
        if(!strcmp(type,"Left Sample") || !strcmp(type,"ROM Left Sample"))
          sample.ch = 0;
        if(!strcmp(type,"Right Sample") || !strcmp(type,"ROM Right Sample"))
          sample.ch = 1;
      }

      sample.sound = SF2_load_sample(filename, sample_num);

      int root_key = HASH_get_int32(region, "root key");
      int coarsetune = HASH_get_int32(region, "coarse tune");
      int finetune = HASH_get_int32(region, "fine tune");

      printf("root: %d, coarse: %d, fine: %d, sample pitch: %d\n",root_key,coarsetune,finetune,(int)HASH_get_int(sample_info,"pitch"));

      int note;
      for(note=0;note<128;note++)
        if(HASH_get_int(sample_info,"pitch")==255)
          sample.frequency_table[note] = HASH_get_int(sample_info, "samplerate");
        else
          sample.frequency_table[note] = HASH_get_int(sample_info, "samplerate") * midi_to_hz(note+coarsetune+(float)finetune/100.0) / midi_to_hz(root_key);

      int note_num;
      for(note_num=HASH_get_int32(region,"key start");note_num<=HASH_get_int32(region,"key end");note_num++){
        Note *note = const_cast<Note*>(data->notes[note_num]);
        note->samples.push_back(&sample);
        
        //printf("%d: %f. middle_note: %d, finetune: %f. Sample: %p. Frequency: %f\n",i,dest->ratio,dest->middle_note,dest->finetune,dest->interleaved_samples,get_frequency(i,dest->finetune));
      }
    }
  }

  // Optimize data->notes so that as few Note objects as possible are used (better for cache)
  for(int i=0;i<128;i++){

    Note *old_note = const_cast<Note*>(data->notes[i]);
    old_note->sort_samples();
    
    for(int i2=0;i2<i;i2++){
      if(data->notes[i]->is_equal(data->notes[i2])){
        
        data->notes[i] = data->notes[i2];

        data->note_storage.remove(old_note);
        delete old_note;

        printf("   Load SF2: Move notes %d to %d. Size of note_storage: %d\n", i, i2, data->note_storage.size());

        break;
      }
    }
    
  }

    

  return true;
}
