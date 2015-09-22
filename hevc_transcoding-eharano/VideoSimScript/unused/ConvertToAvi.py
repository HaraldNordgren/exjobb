import os, sys, math, string, re
import jobCtrl, cfgFile, testSeqs, shutil 

### configuration ###

def usage():
    print "ConvertToAvi.py [result_file] ([directory])"
    print "  results_file            input result text file, located in the Results/ directory"
    print "  directory               optional output/temp directory"
    sys.exit(2)


def Convert(sequence, width, height, fps_in, fps_out, bitstream_size):

    yuvcrop = r'c:\bin\rusert_yuvcrop.exe'

    tempfile = sequence[:-4] + '_temp.yuv'

    if(output_path != ''):
      p = re.compile(r'.*/');
      tempfile = p.sub('', tempfile)
      tempfile = output_path + '/' + tempfile



    size = os.path.getsize(sequence)
    picturesize = width*height*3/2
    nr_frames = size/picturesize

    assert (size % picturesize) == 0, "Internal error - file not an integer number of frames %s" % sequence
    assert fps_in == fps_out, "Internal error - frame skip not supported yet %i %i" % (fps_in, fps_out)

    if fps_in > 30:
       if (width <= 1280) and (height <= 720):
          yoffset = 0
          xoffset = 0
          cmd = "%s -p %i -l %i %i %i %i %i %s %s" % (yuvcrop, width, height, xoffset, yoffset, 1280, 720, sequence, tempfile)
          width = 1280
          height = 720
       else:
          yoffset = (height-720)/2
          xoffset = (width-1280)/2
          cmd = "%s -p %i -l %i %i %i %i %i %s %s" % (yuvcrop, width, height, xoffset, yoffset, 1280, 720, sequence, tempfile)
          width = 1280
          height = 720
    else:
       if (width <= 1920) and (height <= 1080):
          yoffset = 0
          xoffset = 0
          cmd = "%s -p %i -l %i %i %i %i %i %s %s" % (yuvcrop, width, height, xoffset, yoffset, 1920, 1080, sequence, tempfile)
          width = 1920
          height = 1080
       else:
          yoffset = (height-1080)/2
          xoffset = (width-1920)/2
          cmd = "%s -p %i -l %i %i %i %i %i %s %s" % (yuvcrop, width, height, xoffset, yoffset, 1920, 1080, sequence, tempfile)
          width = 1920
          height = 1080

    print cmd
    os.system(cmd)

    rate = int((8.0*bitstream_size*fps_in)/(nr_frames*1000.0))

    avifile = '%s_%ip%i_%ikbps.avi' % (sequence[:-4], height, fps_in, rate)
    if(output_path != ''):
      p = re.compile(r'.*/');
      avifile = p.sub('', avifile)
      avifile = output_path + '/' + avifile

    cmd = r'c:\easyview\yuvconvert %s %s 0 0 %i %i 0 -1 %i "%i kbps"' % (tempfile, avifile, width, height, fps_in, rate)
    print cmd
    os.system(cmd)

    os.remove(tempfile)

    return



if (len(sys.argv) < 2) | (len(sys.argv) > 3):
  usage()

result_file = sys.argv[1]
output_path = ''

assert os.path.isfile(result_file), "could not find result file: %s" % result_file
if len(sys.argv) == 3:
  output_path = sys.argv[2]
  assert os.path.isdir(output_path), "could not find output path: %s" % output_path

conf1 = cfgFile.cfgFile(inputFile=result_file)

enc_dir = 'Results/dec/' + conf1['resultsSubDir'][3:]
dec_dir = 'Results/dec/' + conf1['resultsSubDir'][3:]

assert os.path.isdir(enc_dir), "Internal error - could not find path: %s" % enc_dir
assert os.path.isdir(dec_dir), "Internal error - could not find path: %s" % dec_dir

dec_files = []
dec_list = os.listdir(dec_dir)
for x in dec_list:
  if os.path.isfile(dec_dir+'/'+x):
    if(x[-4:] == '.yuv'):
      dec_files.append(dec_dir+'/'+x)

enc_files = []
enc_list = os.listdir(enc_dir)
for x in enc_list:
  if os.path.isfile(enc_dir+'/'+x):
    if(x[-4:] == '.yuv'):
      enc_files.append(dec_dir+'/'+x)

if len(enc_files) > len(dec_files):
   files = enc_files
else:
   files = dec_files

for file in files:                      # for each file
  for attr in conf1['testSequences']:   # find a match
    (seq, org, width, height, fps_in, fps_out, start, end) = attr
    if string.find(file, seq) != -1:
      p = re.compile(r'dec_|enc_');
      bitstream = p.sub('', file)
      p = re.compile(r'/dec/');
      bitstream = p.sub('/enc/', bitstream)
      p = re.compile(r'_L0.yuv$');
      bitstream = p.sub('.bin', bitstream)
      bitstream_size = os.path.getsize(bitstream)

#      print "%s %s %s %s %s %s" % (file, width, height, fps_in, fps_out, bitstream_size)
#      sys.exit()
      Convert(file, width, height, fps_in, fps_out, bitstream_size)
      break






