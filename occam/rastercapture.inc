#INCLUDE "file.module"
#INCLUDE "raster.module"
#INCLUDE "rasterio.module"
#INCLUDE "rastergraphics.module"
#INCLUDE "sdlraster.module"
#INCLUDE "useful.module"

--{{{  PROTOCOL RASTER.FILTER.TOGGLE
--* Control protocol used internally in filters.
-- @private
PROTOCOL RASTER.FILTER.TOGGLE
  CASE
    enable; BOOL
    quit
:
--}}}
--{{{  PROC raster.filter.control
--* Detect filter-related key events.
-- When Ctrl and [@code key] is pressed, send an enable message; upon shutdown,
-- send a quit message and pass the shutdown through; else pass events through.
-- @private
PROC raster.filter.control (VAL INT key, CHAN RASTER.EVENT in?, out!, CHAN RASTER.FILTER.TOGGLE ctl!)
  INITIAL BOOL enabled IS FALSE:

  INITIAL BOOL running IS TRUE:
  WHILE running
    RASTER.EVENT ev:
    SEQ
      in ? ev
      IF
        (ev[type] = RASTER.ET.KEYDOWN) AND ((ev[mod] = RASTER.MOD.LCTRL) AND (ev[id] = key))
          --{{{  Ctrl and key: toggle
          SEQ
            enabled := NOT enabled
            ctl ! enable; enabled
          --}}}
        ev[type] = RASTER.ET.SHUTDOWN
          --{{{  quit
          SEQ
            out ! ev
            ctl ! quit
            running := FALSE
          --}}}
        TRUE
          out ! ev
:
--}}}
--{{{  PROC raster.capture.filter
--* Filter to capture frames of video, toggled by Ctrl-R.
PROC raster.capture.filter (CHAN RASTER in?, out!, CHAN RASTER.EVENT events.in?, events.out!)
  --{{{  PROC capture
  PROC capture (CHAN RASTER in?, out!, CHAN RASTER.FILTER.TOGGLE ctl?)
    INITIAL BOOL enabled IS FALSE:
    INITIAL INT sequence IS 0:

    INITIAL BOOL ring.exit IS FALSE:
    INITIAL BOOL control.exit IS FALSE:
    WHILE NOT (ring.exit AND control.exit)
      ALT
        RASTER r:
        in ? r
          --{{{
          SEQ
            IF
              (SIZE r) = 0
                --{{{  no more rasters coming
                ring.exit := TRUE
                --}}}
              enabled
                SEQ
                  --{{{  capture frame
                  MOBILE []BYTE num, padded, filename:
                  SEQ
                    -- FIXME: This is awkward -- I'd really like a "z" format type that took a padding width...
                    format.i (sequence, num)
                    format.ss ("00000000", num, padded)
                    format.sss ("capture/frame", [padded FROM (SIZE padded) - 8], ".ppm", filename)
                    write.raster (filename, r)
                  sequence := sequence + 1
                  --}}}
                  --{{{  draw "viewfinder" graphics
                  VAL INT width IS SIZE r[0]:
                  VAL INT gap IS 20:
                  VAL INT radius IS 30:
                  fill.circle (width - (radius + gap), (radius + gap), radius, COLOUR.RED, r)
                  --}}}
              TRUE
                SKIP
            out ! r
          --}}}
        ctl ? CASE
          enable; enabled
            --{{{
            SEQ
              trace.sin ("capture: ", INT enabled)
              sequence := 0
              INT result:
              file.mkdir ("capture", S.I755, result)
            --}}}
          quit
            --{{{
            control.exit := TRUE
            --}}}
  :
  --}}}

  CHAN RASTER.FILTER.TOGGLE ctl:
  PAR
    capture (in?, out!, ctl?)
    raster.filter.control ('r', events.in?, events.out!, ctl!)
:
--}}}
--{{{  PROC raster.pause.filter
--* Filter to pause the raster display, toggled by Ctrl-P.
-- (If you're using multiple filters, you probably want this one first in the
-- chain to avoid blocking later filters.)
PROC raster.pause.filter (CHAN RASTER in?, out!, CHAN RASTER.EVENT events.in?, events.out!)
  --{{{  PROC pause
  PROC pause (CHAN RASTER in?, out!, CHAN RASTER.FILTER.TOGGLE ctl?)
    INITIAL BOOL pausing IS FALSE:
    INITIAL BOOL paused IS FALSE:

    INITIAL BOOL ring.exit IS FALSE:
    INITIAL BOOL control.exit IS FALSE:
    WHILE NOT (ring.exit AND control.exit)
      ALT
        RASTER r:
        (NOT paused) & in ? r
          --{{{
          SEQ
            IF
              (SIZE r) = 0
                --{{{  no more rasters coming
                ring.exit := TRUE
                --}}}
              pausing
                SEQ
                  --{{{  draw "viewfinder" graphics
                  VAL INT width IS SIZE r[0]:
                  VAL INT bar IS 20:
                  SEQ i = 2 FOR 2 STEP 2
                    fill.rectangle (width - (i * bar), bar, bar, 3 * bar, COLOUR.WHITE, r)
                  --}}}
                  --{{{  pause from now on
                  paused := TRUE
                  --}}}
              TRUE
                SKIP
            out ! r
          --}}}
        ctl ? CASE
          enable; pausing
            --{{{
            paused := FALSE
            --}}}
          quit
            --{{{
            control.exit := TRUE
            --}}}
  :
  --}}}

  CHAN RASTER.FILTER.TOGGLE ctl:
  PAR
    pause (in?, out!, ctl?)
    raster.filter.control ('p', events.in?, events.out!, ctl!)
:
--}}}
--{{{  PROC raster.capture
--* Raster display with extra features.
-- For the meaning of the parameters, see [@ref raster.display].
--
-- The extra features:
--
-- [@item] Ctrl-P will toggle pause mode -- it won't accept any more rasters on
-- its input until it is unpaused. Depending on how the rest of your program is
-- constructed, this may or may not work as a general-purpose pause button.
-- [@item] Ctrl-R will toggle record mode, in which passing rasters will be
-- saved as PPM image files in a [@code capture] subdirectory. These can be
-- encoded into a video file later using tools like [@text mencoder].
PROC raster.capture (VAL []BYTE title, VAL INT width, height, num.rasters, CHAN RASTER in?, out!, CHAN RASTER.EVENT events!)
  [2]CHAN RASTER thru:
  [2]CHAN RASTER.EVENT events.thru:
  PAR
    raster.pause.filter (in?, thru[0]!, events.thru[0]?, events.thru[1]!)
    raster.capture.filter (thru[0]?, thru[1]!, events.thru[1]?, events!)
    raster.display (title, width, height, num.rasters, thru[1]?, out!, events.thru[0]!)
:
--}}}
--{{{  PROC raster.capture.simple
--* Simple raster display with extra features.
-- For the meaning of the parameters, see [@ref raster.display.simple].
-- For the extra features, see [@ref raster.capture].
PROC raster.capture.simple (VAL []BYTE title, VAL INT width, height, num.rasters, CHAN RASTER in?, out!)
  CHAN RASTER.EVENT events:
  CHAN RASTER thru:
  PAR
    raster.capture (title, width, height, num.rasters, in?, thru!, events!)
    raster.autoclose (thru?, out!, events?)
:
--}}}
