# Kista

## Wednesday
- Move svn repo to cluster.
- Ask Ruoyang about the Assert failures in the decoder.
- Get the transcoding to work on the cluster

## QPs
- Iterate over different QPs, create folders for each and store output.
- Where should QP be changed (-q option), only for coder or decoder as well?

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
- Decode to get reconstructed samples using HM or d65mt_dev.
- Optionally downscale to a smaller resolution.

## Step 2
- Transcode by encoding to derive LQ bitstream with coefficients using HM with RDOQ=0 (since the guided transcoder don’t deploy RDOQ).
- Then remove the coefficients using one of the compiled versions of d65_gt (prune).

## Step 3
- Next decode the HQ bitstream with another version of d65_gt (output in decoding order).
- Optionally downscale to a smaller resolution.

## Step 4
- Then perform guided transcoding using d65_gt (generate residual) with pruned bitstream and downscaled HQ in decoding order.
- Then that bitstream now is ready for transmission to the end device and can be decoding of d65mt_dev or HM decoder.


# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.