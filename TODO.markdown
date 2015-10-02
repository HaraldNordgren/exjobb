# Modular transcoding chain

## Race condition
- Map out direct dependecies in gtm.
- Restructure file accordlingly.

## Cfg file
- Send information about mode, frames, framerate through bsub too

## File handling
- Each script will create its own tmp directory structure.
- Before each step, check if the corresponding file exists on storage.
- If not, create it. Then check so that no other script created it in the meantime.
- Move it over to the storage.

## Folders
- "output-folder/storage/seq/qp-hq/size/qp-lq" for final storage.
- "output-folder/tmp/tmp-seq-qp-size-qp/seq/qp-hq/size/qp-lq" for script tmp.
- Move subdirectory structure and then file after running a command.

# Ask Kennth
- Mario's code, what does it do?

# Downscaling problems
- Find out why double downscaling fails. (Creeping colors)
- 1080p downscaled to 264p causes errors. [0,0]
- 240p downscaled to 24p causes errors. Maybe expected for such a small size?
- Create a stringent test.
- "software\d65_gt\tools\downScale\bin\Release\downScale.exe 832 480 sample_videos\MPEG_CfP_seqs\orig-draft-cfp_2009-07-23\BasketballDrill_832x480_50.yuv test_data\hej.yuv 0 1" fails in ConEmu

# Bonus
- Indendation-level in call-indented

# Create test anchor

## Mock test
- Create downscaled originals in loop.
- Calculate PSNR for each transcoded video by comparing to same-size original.
- Can different length/framerate videos be compared this way?
- Otherwise transcoding needs to encode all frames!
- Filesize of transcoded file (bin) compared to what?

## Real test
- Choose 1080p test sequences: BQTerrace, etc.
- BD (Two coder configs at four quantizer settings)


# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.
