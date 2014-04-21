<CsoundSynthesizer>
<CsOptions>
; -L stdin -odac           -iadc     -dm6    ;;;RT audio I/O
-L stdin -odac           -iadc     -dm6  -+rtaudio=jack -+jack_client=csoundGrain  -b 1024 -B 2048   ;;;RT audio I/O
; For Non-realtime ouput leave only the line below:
; -o grain3.wav -W ;;; for file output any platform
</CsOptions>
<CsInstruments>

sr	=  44100
;kr      =  100
ksmps   =  16
nchnls	=  1

;;;#define SIZE #268435456#
#define SIZE #4194304#


maxalloc 556, 600; Limit instances

/* f#  time  size  1  filcod  skiptime  format  channel */
;;; giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/long-recording-of-chinese-new-year.WAV", 0, 0, 0	
;;;;giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/crunchyglasswalking.wav", 0, 0, 0	
;;;;;giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/toilet-flush.wav", 0, 0, 0	
;;;;;;giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/firecrackers.wav", 0, 0, 0	
;;;;;;giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/lrt-crossing-bridge.WAV", 0, 0, 0	
;;;;giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/chinese-cymbals.wav", 0, 0, 0 ;; pretty good
;;giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/Sabatini-Scaramouche-01-01.wav", 0, 0, 0	
;;;giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/Music/20140420/ferry-sounds3-good.WAV", 0, 0, 0	
;;;giImpulse01     ftgen   101, 0, $SIZE, 1, "./palette/some-jam.wav", 0, 0, 0	
giImpulse01     ftgen   101, 0, $SIZE, 1, "/opt/hindle1/hdprojects/granular/videogames.wav", 0, 0, 0	
giImpulse01     ftgen   101, 0, $SIZE, 1, "goldberg-aria-da-capo.wav", 0, 0, 0	



gkFreq init 1
gkFreqRand init 0.001
gkAmp init 1000
gkDens init 20
gkDur init 0.1
gkPhase init 0
gkPhaseMix init 1

giEXP = -1
giLINEAR = 0

FLcolor	180,200,199
FLpanel 	"Granular",275,100
    istarttim = 0
    idropi = 666
    idur = 1
    ibox0  FLbox  "Grain", 1, 6, 12, 300, 20, 0, 0
    ;FLsetFont   7, ibox0
                
;    gkFreq,    iknob1 FLknob  "Freq", 0.00001, 2, -1,1, -1, 50, 0,0
    gkFreq,    iknob1 FLknob  "Freq", 0.01, 5, giLINEAR ,1, -1, 50, 0,25
;    gkFreqRand,    iknob2 FLknob  "FreqRand", 0.00001, 0.1, -1,1, -1, 50, 50,0
    gkAmp,    iknob3 FLknob  "Amp", 0.0001, 10000, -1,1, -1, 50, 100,25
;    gkDens,    iknob4 FLknob  "Dens", 1, 300, -1,1, -1, 50, 150,0
    gkDur,    iknob5 FLknob  "Dur", 0.01, 1.0, -1,1, -1, 50, 200,25
    ;kout, ihandle FLslider "label", imin, imax, iexp, itype, idisp, iwidth, \
    ;  iheight, ix, iy
;    gkPhase, islider1 FLslider "Phase", 0, 1, 0, 1, -1, 250, 50, 0, 100
    ;kout, ihandle FLknob "label", imin, imax, iexp, itype, idisp, iwidth, \
    ;  ix, iy [, icursorsize]
;    gkPhaseMix,    iknob6 FLknob  "PhaseMix", 0, 1.0, 0,1, -1, 50, 200,150
    FLsetVal_i   1.0, iknob1
;    FLsetVal_i   0.001, iknob2
    FLsetVal_i   1000, iknob3
;    FLsetVal_i   20, iknob4
    FLsetVal_i   0.1, iknob5
;    FLsetVal_i   0.0, islider1
;    FLsetVal_i   1.0, iknob6

;    gkIComp01,     gicompslider01 FLslider "01", 0, 1, 0, 1, -1, 250, 30, 255, (0)
    ion = 0
    ioff = 0
    ix = 0
    iy = 150
    iopcode = 0 ; i 


    
FLpanel_end	;***** end of container

FLrun		;***** runs the widget thread 





/* Bartlett window */
itmp	ftgen 1, 0, 16384, 20, 3, 1
/* sawtooth wave */
itmp	ftgen 2, 0, 16384, 7, 1, 16384, -1
/* sine */
itmp	ftgen 4, 0, 1024, 10, 1




; the grain
        instr 556
        idur = p3
        iamp = p4
        ipitch = (p5 / ($SIZE / 44100)) ; 16 1.352e-12 ; 2 is 2X 0.5 is 1/2
        iphase = p6
        iamp01 =  p7
aenv    oscili iamp, 1/idur, 1        
aa01     poscil iamp01, ipitch, 101, iphase
        out aenv*(aa01)

        endin

; the global grain
        instr 550
        idur = i(gkDur)
        p3 = idur
        iamp = i(gkAmp)
        ipitch = (i(gkFreq) / ($SIZE / 44100)) ; 16 1.352e-12 ; 2 is 2X 0.5 is 1/2
        iphase = p4
aenv    oscili iamp, 1/idur, 1        
aa      oscili 1.0, ipitch, 101, iphase
        out aenv*(aa)
        endin




        instr 557
        idur = p3
        iamp = p4
        ipitch = p5 * (1.0 / ($SIZE / 44100)) ; 16 1.352e-12 ; 2 is 2X 0.5 is 1/2
        iphase = p6
aa      oscili iamp, ipitch, 101, iphase
        out aa
        endin





</CsInstruments>
<CsScore>

t 0 60

f 0 3600

i556 0 0.1 11111 4.0 0.52 1.0
i556 0.1 0.1 11111 4.0 0.53 1.0
i556 0.2 0.1 11111 4.0 0.54 1.0
i556 0.3 0.1 11111 4.0 0.55 1.0
i556 0.4 0.1 11111 4.0 0.56 1.0
i556 0.5 0.1 11111 4.0 0.57 1.0
i556 0 0.1 11111 4.0 0.42 1.0
i556 0.1 0.1 11111 4.0 0.43 1.0
i556 0.2 0.1 11111 4.0 0.44 1.0
i556 0.3 0.1 11111 4.0 0.45 1.0
i556 0.4 0.1 11111 4.0 0.46 1.0
i556 0.5 0.1 11111 4.0 0.47 1.0


</CsScore>
</CsoundSynthesizer>
