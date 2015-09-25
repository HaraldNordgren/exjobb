# Kista

## Reorganization
- Moved HM-16.6, d65-gt, d65mt-dev to software.
- Moved batch, shell, python to scripts
- Moved run.py out of python folder to scripts
- Moved binaries, definitions to new definitions folder.
- Split definitions.py into three files.

## Friday
- Maybe move svn repo to cluster.
- Ask Ruoyang about the Assert failures in the decoder.
- Get the transcoding to work on the cluster.
- "All" option for frames in hm_encoder?

## Transmission

### Sender
- Original video in encoded and transmitted. This is the HQ bitstream.
- HQ bitstream is decoded, downscaled, reencoded with RDOQ=0 and pruned, then transmitted.

### Reciever
- HQ bitstream is decoded in dec order and downscaled.
- Dec order, together with pruned version are used to reconstruct the video.

## QPs
- Iterate over different QPs, create folders for each and store output. Outside of current loop.
- (-q) option is set for hm-decoder (what numbers?)
- hm-decoder is only run twice: To encode original, and when encoded with RDOQ=0 before pruning.
- First one is QP-hq, and second one QP-lq (QP-hq + 2 ?).

## Create test anchor
- Choose 1080p test sequences: BQTerrace, etc.
- Downscale to create 720p, 540p and 360p "originals" of each sequence.
- Use these to calculate PSNR.
- BD (Two coder configs at four quantizer settings)

## Preprocessing
- Create HQ bitstreams for 1080p videos to speed up the transcoding chain.
- Preferably ~10 seconds with reasonable framerate.


# <s>Transcoding chain</s>

## Preprocessing
- Encode a test sequence to generate a HQ bitstream using HM.

## Step 1
- Decode to get reconstructed samples using HM or d65mt-dev.
- Optionally downscale to a smaller resolution.

## Step 2
- Transcode by encoding to derive LQ bitstream with coefficients using HM with RDOQ=0 (since the guided transcoder don’t deploy RDOQ).
- Then remove the coefficients using one of the compiled versions of d65-gt (prune).

## Step 3
- Next decode the HQ bitstream with another version of d65-gt (output in decoding order).
- Optionally downscale to a smaller resolution.

## Step 4
- Then perform guided transcoding using d65-gt (generate residual) with pruned bitstream and downscaled HQ in decoding order.
- Then that bitstream now is ready for transmission to the end device and can be decoding of d65mt-dev or HM decoder.


# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.
