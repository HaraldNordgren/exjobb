# Transmission

## Sender
- Original video in encoded and transmitted. This is the HQ bitstream.
- HQ bitstream is decoded, downscaled, reencoded with RDOQ=0 and pruned, then transmitted.

## Reciever
- HQ bitstream is decoded in dec order and downscaled.
- Dec order video together with pruned version are used to reconstruct the video.


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
