# Cluster (Bash)
- Useful aliases to add to bashrc at work
- alias ll='ls -alF'

# Create test anchor

## Theory
- PSNR calculated for each (decoded) transcoded video by comparing to same-size original.
- Filesize of transcoded bitstream is compared downscaled encoded original.

## Sequences
- Four 1080p sequences chosen.
- Downscaled to 720, 536p and 360p.
- Encoded with QP-hqs [22, 27, 32, 37]
- Then again with QP-lqs [24, 29, 34, 39]

## Test data
- PSNR data -> Excel
- Choose 1080p test sequences: BQTerrace, etc.
- BD (Two coder configs at four quantizer settings)

## Read files
- RunEvalGeneric.py, psnr.py, jobCtrl.py

# Fixes for Modular transcoding chain
- Rename files like "downscaled-file".
- Restructure chain, original branch, then two branches that lead to the reconstruction.
- Put all filename definitions on top of the file.
- Send information about mode, frames, framerate through bsub, avoid duplication of data.

# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.
