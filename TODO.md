# Kista

## Monday
- Email from Julian should contain a usable command-line video player, check that out.
- Change to "if not m: then Exception" in filenames.py for less indentation.
- Print filenames to see that they are corrects, especially the "_transcoded" ones.

## Python
- Iterate over different QPs, create folders for each and store output.
- Where should QP be changed (-q option), only for coder or decoder as well?
- Make it run on the cluster

## Ta fram ankare
- En koding per testsekvens, ska helst göras bara en gång. Ger HQ bit, som sedan avkodas.
- Generera nerskalade versioner. För S=832x480 (S*2/3=320...)
- (720, 540, 360)

## Testing
- Add more tests to RH832 or choose another set.
- Calculate PSNR for transcoded file.
- BD (Two coder configs at four quantizer settings)

## <s>Transcoding chain</s>

### Preprocessing
- Encode a test sequence to generate a HQ bitstream using HM.

### Step 1
- Decode to get reconstructed samples using HM or d65mt_dev.
- Optionally downscale to a smaller resolution.

### Step 2
- Transcode by encoding to derive LQ bitstream with coefficients using HM with RDOQ=0 (since the guided transcoder don’t deploy RDOQ).
- Then remove the coefficients using one of the compiled versions of d65_gt (prune).

### Step 3
- Next decode the HQ bitstream with another version of d65_gt (output in decoding order).
- Optionally downscale to a smaller resolution.

### Step 4
- Then perform guided transcoding using d65_gt (generate residual) with pruned bitstream and downscaled HQ in decoding order.
- Then that bitstream now is ready for transmission to the end device and can be decoding of d65mt_dev or HM decoder.


# Uppsatsen
- Läs gamla uppsatser.
- Försök formulera en snabb inledning, lite metod.
- Thomas Ruserts whiteboard-bild ger en bra struktur av arbetet.

# <s>SimSim workflow

## Step 1
- run.bat is called.
- RunSimHM16.6.py is called by run.bat.
- RunSimSamsung(RunSim) object is created, and its start() is called.

## Step 2
- Acquire jobCtrl object.
- Iterate over test set (RH832), and get job instances for qpSet (22, 27, 32, 37). Type is SimpleJob.
- Submit() is called to create script file at the predetermined location, and _submitScriptFile() calls subprocess.Popen() to execute simpleJobScript.bat.

## Step 3
- simpleJobScript.bat runs RunEncodingGeneric.py with config.txt.

## Step 4
- decFile is defined in RunEncodingGeneric.py
</s>
