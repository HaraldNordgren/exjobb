# Ask Kennth
- Mario's code, what does it do?

# Ask Ruoyang
- What are "--SEIDecodedPictureHash" and "--Level".
- They give warnings that clog stderr.

# Downscaling problems
- Find out why double downscaling fails. (Creeping colors)
- 1080p downscaled to 264p causes errors. [0,0]
- 240p downscaled to 24p causes errors. Maybe expected for such a small size?


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
