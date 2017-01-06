(provide 'instruments.scm)

#||
  * Sends should be slightly skewed to the left, so that we see that they go in parallel. Arrows indicating this might help too.

  * What if the last sound object in the sequence isn't connected to the main pipe? How should this be visualized?
    Ardour seems to visualize this with a button under the volume slider ("Master"). (that makes sense)

||#

(define *bus-effect-names* (list "System Reverb On/Off"
                                 "System Chorus On/Off"
                                 "System Aux 1 On/Off"
                                 "System Aux 2 On/Off"
                                 "System Aux 3 On/Off"))

(define (for-all-tracks func)
  (for-each (lambda (blocknum)
              (for-each (lambda (tracknum)
                          (func blocknum tracknum))
                        (iota (<ra> :get-num-tracks blocknum))))
            (iota (<ra> :get-num-blocks))))

(define (get-all-midi-instruments)
  (map (lambda (instrument-num)
         (<ra> :get-midi-instrument-id instrument-num))
       (iota (<ra> :get-num-midi-instruments))))
         
(define (get-all-audio-instruments)
  (map (lambda (instrument-num)
         (<ra> :get-audio-instrument-id instrument-num))
       (iota (<ra> :get-num-audio-instruments))))

(define (get-instrument-from-name name)
  (let loop ((instruments (append (get-all-midi-instruments)
                                  (get-all-audio-instruments))))
    (if (null? instruments)
        #f
        (if (string=? name (<ra> :get-instrument-name (car instruments)))
            (car instruments)
            (loop (cdr instruments))))))

#||
(for-each (lambda (i) (<ra> :set-instrument-effect i "System Solo On/Off" 0)) (get-all-audio-instruments))
||#

(define (get-buses-connecting-from-instrument id-instrument)
  (keep identity
        (map (lambda (bus-num effect-name)
               (if (>= (<ra> :get-instrument-effect id-instrument effect-name) 0.5)
                   (<ra> :get-audio-bus-id bus-num)
                   #f))
             (iota (length *bus-effect-names*))
             *bus-effect-names*)))

(define (get-instruments-connecting-to-instrument id-instrument)
  (map (lambda (in-connection)
         (<ra> :get-audio-connection-source-instrument in-connection id-instrument))
       (iota (<ra> :get-num-in-audio-connections id-instrument))))

(define (get-instruments-connecting-from-instrument id-instrument)
  (map (lambda (in-connection)
         (<ra> :get-audio-connection-dest-instrument in-connection id-instrument))
       (iota (<ra> :get-num-out-audio-connections id-instrument))))

(define (get-instruments-econnecting-to-instrument id-instrument)
  (map (lambda (in-connection)
         (<ra> :get-event-connection-source-instrument in-connection id-instrument))
       (iota (<ra> :get-num-in-event-connections id-instrument))))

(define (get-instruments-econnecting-from-instrument id-instrument)
  (map (lambda (in-connection)
         (<ra> :get-event-connection-dest-instrument in-connection id-instrument))
       (iota (<ra> :get-num-out-event-connections id-instrument))))

(define (duplicate-connections id-old-instrument id-new-instrument)
  ;; in audio
  (for-each (lambda (from-instrument)
              (<ra> :create-audio-connection
                    from-instrument
                    id-new-instrument))
            (get-instruments-connecting-to-instrument id-old-instrument))
  ;; out audio
  (for-each (lambda (to-instrument)
              (<ra> :create-audio-connection
                    id-new-instrument
                    to-instrument))
            (get-instruments-connecting-from-instrument id-old-instrument))
  ;; in event
  (for-each (lambda (from-instrument)
              (<ra> :create-event-connection
                    from-instrument
                    id-new-instrument))
            (get-instruments-econnecting-to-instrument id-old-instrument))
  ;; out event
  (for-each (lambda (to-instrument)
              (<ra> :create-event-connection
                    id-new-instrument
                    to-instrument))
            (get-instruments-econnecting-from-instrument id-old-instrument)))

(define (replace-instrument-in-all-tracks! id-old-instrument id-new-instrument)
  (for-all-tracks
   (lambda (blocknum tracknum)
     (if (= id-old-instrument (<ra> :get-instrument-for-track tracknum blocknum))
         (<ra> :set-instrument-for-track id-new-instrument tracknum blocknum)))))

(define (replace-instrument-in-mixer id-old-instrument id-new-instrument)
  (define x (<ra> :get-instrument-x id-old-instrument))
  (define y (<ra> :get-instrument-y id-old-instrument))
  (<ra> :delete-instrument id-old-instrument)
  (<ra> :set-instrument-position x y id-new-instrument)
  )

;; Called from the outside. 'instrument-description' can be false or empty string.
(define (replace-instrument id-old-instrument instrument-description)
  (if (<ra> :instrument-is-permanent id-old-instrument)
      (<ra> :show-message "Can not be replaced")
      (let ((instrument-description (if (or (not instrument-description)
                                            (string=? instrument-description ""))
                                        (<ra> :instrument-description-popup-menu)
                                        instrument-description)))
        (when (not (string=? "" instrument-description))
          (undo-block
           (lambda ()
             (define patch-name (if (<ra> :instrument-name-was-autogenerated id-old-instrument)
                                    ""
                                    (<ra> :get-instrument-name id-old-instrument)))
             (define id-new-instrument (<ra> :create-audio-instrument-from-description instrument-description patch-name))
             (when (not (= -1 id-new-instrument))
               (duplicate-connections id-old-instrument id-new-instrument)
               (replace-instrument-in-all-tracks! id-old-instrument id-new-instrument)
               (replace-instrument-in-mixer id-old-instrument id-new-instrument)
               )))))))

;; Called from the outside. 'instrument-description' can be false or empty string.
(define (load-instrument-preset id-instrument instrument-description)
  (if (<ra> :instrument-is-permanent id-instrument)
      (<ra> :show-message "Can not load preset for this instrument")
      (let ((instrument-description (if (or (not instrument-description)
                                            (string=? instrument-description ""))
                                        (<ra> :request-load-preset-instrument-description)
                                        instrument-description)))
        (if (not (string=? instrument-description ""))
            (replace-instrument id-instrument instrument-description)))))
        

#!!
(<ra> :get-num-audio-instruments)
(define id (<ra> :get-audio-instrument-id 7))
(<ra> :get-instrument-x 17)
(<ra> :get-instrument-y 17)
 
(<ra> :set-instrument-position -80 106 17)
(<ra> :connect-audio-instrument-to-main-pipe 17)
(<ra> :delete-instrument 22)

(let ((descr (<ra> :instrument-description-popup-menu)))
  (define id (<ra> :create-audio-instrument-from-description descr))
  ;;(<ra> :delete-instrument id)
  id
  )

(<ra> :delete-instrument 27)

(<ra> :get-instrument-for-track)

;; delete instrument in track 0:
(begin
  (<ra> :delete-instrument (<ra> :get-instrument-for-track)))

(<ra> :set-instrument-name "heprqereqp3" 25)

(let ((id-old-instrument (<ra> :get-instrument-for-track))
      (descr (<ra> :instrument-description-popup-menu)))
  (replace-instrument id-old-instrument descr)
  ;;(define id-new-instrument (<ra> :create-audio-instrument-from-description descr))
  ;;(<ra> :set-instrument-for-track id-new-instrument)
  id-old-instrument)

(<ra> :get-undo-history)


!!#





;; Called from the outside. if 'do-autoconnect' is true, instruments sending sound to id-instrument and instruments getting sound from id-instrument will be connected.
;;(define (delete-instrument id-instrument do-autoconnect)
;;  ...)


(define (select-track-instrument tracknum)
  (undo-block
   (lambda ()
     (define midi-instruments (get-all-midi-instruments))
     (define instruments-before (get-all-audio-instruments))
     (define id-instrument
       (popup-menu
        "<New MIDI Instrument>" (lambda ()
                                  (<ra> :create-midi-instrument "Unnamed"))
        "<New Sample Player>" (lambda ()
                                (<ra> :create-audio-instrument "Sample Player" "Sample Player"))
        "<New FluidSynth>" (lambda ()
                             (<ra> :create-audio-instrument "FluidSynth" "FluidSynth"))
        (if (<ra> :has-pure-data)
            (list "<New Pd Instrument>" (lambda ()
                                          (<ra> :create-audio-instrument "Pd" "Simple Midi Synth")))
            #f)
        "<New Audio Instrument>" (lambda ()
                                   (<ra> :create-audio-instrument-from-description 
                                         (<ra> :instrument-description-popup-menu)))
        "<Load New Preset>" (lambda ()
                              (<ra> :create-audio-instrument-from-description
                                    (<ra> :request-load-preset-instrument-description)))
        "----------"
        "Clone Audio Instrument" (map (lambda (num instrument-id)
                                        (if (<ra> :instrument-is-permanent instrument-id)
                                            #f
                                            (list (<-> num ". " (<ra> :get-instrument-name instrument-id))
                                                  (lambda ()
                                                    (<ra> :clone-audio-instrument instrument-id)))))
                                      (iota (length instruments-before))
                                      instruments-before)
        (and (> (length midi-instruments) 0)
             (list "----------"
                   (map (lambda (num instrument-id)
                          (list (<-> num ". " (<ra> :get-instrument-name instrument-id))                     
                                (lambda ()
                                  instrument-id)))
                        (iota (length midi-instruments))
                        midi-instruments)))
        "----------"
        (map (lambda (num instrument-id)
               (list (<-> num ". " (<ra> :get-instrument-name instrument-id))                     
                     (lambda ()
                       instrument-id)))
             (iota (length instruments-before))
             instruments-before)))
     
     (define (is-new-instrument? id-instrument)
       (and (not (member id-instrument instruments-before))
            (member id-instrument (get-all-audio-instruments))))

     (define (num-new-instruments)
       (- (length (get-all-audio-instruments))
          (length instruments-before)))
     
     (when (and (integer? id-instrument) (not (= -1 id-instrument)))
       (<ra> :set-instrument-for-track id-instrument tracknum)
       (when (and (is-new-instrument? id-instrument)
                  (= 1 (num-new-instruments)))
         (<ra> :autoposition-instrument id-instrument)
         (<ra> :connect-audio-instrument-to-main-pipe id-instrument))))))
#||
(select-track-instrument 0)
||#


