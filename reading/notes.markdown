# Test anchor

## Sequences
- Four 1080p sequences chosen. (BQTerrace, etc.)
- Downscale to 720p, 536p and 360p.
- Encode with QP-hqs [22, 27, 32, 37]
- Then again with QP-lqs as [qp-hq, qp-hq + 2, ...]


# Modular transcoding chain

## File handling
- Each script will create its own tmp directory structure.
- Before each step, check if the corresponding file exists on storage.
- If not, create it. Then check so that no other script created it in the meantime.
- Move it over to the storage.

## Folders
- "output-folder/storage/seq/qp-hq/size/qp-lq" for final storage.
- "output-folder/tmp/tmp-seq-qp-size-qp/seq/qp-hq/size/qp-lq" for creating the files.
- Move file after running a command, creating folders in storage as needed.


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
