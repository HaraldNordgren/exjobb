# TODO
- Move PSNRStatic and link in binaries.py
- indendation-level in call-indented

# Ask Kennth
- Mario's code, what does it do?
- What are "--SEIDecodedPictureHash" and "--Level".
- They give warnings that clog stderr.
- Comment them in source!

# Downscaling problems
- Find out why double downscaling fails. (Creeping colors)
- 1080p downscaled to 264p causes errors. [0,0]
- 240p downscaled to 24p causes errors. Maybe expected for such a small size?
- Create a stringent test.
- "software\d65_gt\tools\downScale\bin\Release\downScale.exe 832 480 sample_videos\MPEG_CfP_seqs\orig-draft-cfp_2009-07-23\BasketballDrill_832x480_50.yuv test_data\hej.yuv 0 1" fails in ConEmu


# Create test anchor

## Mock test
- Create downscaled originals in loop.
- Calculate PSNR for each transcoded video by comparing to same-size original.
- Can different length/framerate videos be compared this way?
- Otherwise transcoding needs to encode all frames!
- Filesize of transcoded file (bin) compared to what?

## Real test
- Choose 1080p test sequences: BQTerrace, etc.
- Downscale to create 720p, 540p and 360p "originals" of each sequence.
- Use these to calculate PSNR.
- BD (Two coder configs at four quantizer settings)


# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.
