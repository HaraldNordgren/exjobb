# Create test anchor

## Theory
- PSNR calculated for each transcoded video by comparing to downscaled original.
- Filesize of pruned bitstream is compared to downscaled encoded original.
- Also data final file, one is sent to hub, the other to end-user.

## Test data
- Get this data into Excel somehow.
- This will form the base data on the BD diagram.
- BD (Two coder configs at four quantizer settings)
- Any improvements on the encoder will be measured against this.


# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.


# For v1.1

## Test framerate
- Create yuv-video from Smala Sussie using ffmpeg.
- Do transcoding to test framerate, preserved and altered.
- Then scrap "preserve-framerate", assume it is preserved.

## Fixes for Modular transcoding chain
- Rename files like "downscaled-file" for clarity.
