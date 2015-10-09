# Create test anchor

## Sequences
- Four 1080p sequences chosen. (BQTerrace, etc.)
- Downscale to 720p, 536p and 360p.
- Encode with QP-hqs [22, 27, 32, 37]
- Then again with QP-lqs as [qp-hq, qp-hq + 2, ...]

## Theory
- PSNR calculated for each transcoded video by comparing to downscaled original.
- Filesize of pruned bitstream is compared to downscaled encoded original.

## Test data
- Get this data into Excel somehow.
- This will form the base data on the BD diagram.
- BD (Two coder configs at four quantizer settings)
- Any improvements on the encoder will be measured against this.

## VideoSimScript
- RunEvalGeneric, ExcelChart, ExcelPlot, BDelta, psnr
- See if you can find any hooks for PSNR data.
- Seems very difficult since I have no config files, no RunEncodingGeneric output.
- How does PSNRStatic differ from Ericsson PSNR calculation?


# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.
- Mejla Mike Doggett


# For v1.1

## Config
- Move entire "configuration" to bsub-transcoding.py.
- cfg_file, preserve_framerate, new_framerate.
- Move all_frames to new constants file.

## Fixes for Modular transcoding chain
- Will break os.path.exits() checks so 14-hour chain will have to be rerun
- Rename files like "downscaled-file" for clarity.
- Restructure chain: First the "originals" branch.
- Then the two branches that lead to the reconstruction.
