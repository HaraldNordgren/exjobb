import sys, os, subprocess
import downscaling, filenames, paths, raw_video, time_string, command_line
import encode_original_module

import definitions.config       as config
import definitions.debug        as debug
import definitions.directories  as directories
import definitions.binaries     as binaries


"""
[1], [0], [1,0] means two-thirds, half and one-third downscaling.
This corresponds to 720p, 536p and 360p for a 1080p video.
"""

#downscale_parameter_list = [[1], [0], [1,0]]
#downscale_parameter_list = [ [1,0,0], [1,0,0,0] ]
downscale_parameter_list = [ [0], [0,0] ]


def transcode(hq_bitstream):

    cfg_mode = filenames.extract_cfg_mode(config.cfg_file)

    if not cfg_mode in hq_bitstream:
        print 'Incompatible cfg modes: %s with "%s"' % (hq_bitstream, cfg_mode)
        sys.exit(1)


    print "Bitstream: %s\n" % hq_bitstream

    hq_bitstream_basename = os.path.basename(hq_bitstream)
    hq_bitstream_shortpath = os.path.splitext(hq_bitstream_basename)[0]

    (width, height) = filenames.extract_dimensions(hq_bitstream_basename)
    framerate = filenames.extract_framerate(hq_bitstream_basename)

    sequence_folder = "%s/%s_gt_%s" % (directories.output_folder, hq_bitstream_shortpath, time_string.current())
    paths.remove_and_recreate_directory(sequence_folder)

    err_log_path = "%s/error_log.txt" % sequence_folder


    # Preprocessing

    ## Decode HQ bitstream (Sender side)

    hq_bitstream_decoded_shortpath = "%s_dec" % hq_bitstream_shortpath
    hq_bitstream_decoded = "%s/%s.yuv" % (sequence_folder, hq_bitstream_decoded_shortpath)

    hq_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, hq_bitstream, hq_bitstream_decoded)
    command_line.call_indented(hq_decode_cmd, err_log_path=err_log_path)

    raw_video.mux(hq_bitstream_decoded, err_log_path=err_log_path)


    ## Decode HQ bitstream in decoding order (Receiver side)

    hq_bitstream_decoded_dec_order_shortpath = "%s_dec_dec_order" % hq_bitstream_shortpath
    hq_bitstream_decoded_dec_order = "%s/%s.yuv" % (sequence_folder, hq_bitstream_decoded_dec_order_shortpath)

    dec_order_cmd = "%s -i %s -o %s" % (binaries.d65_gt_dec_order, hq_bitstream, hq_bitstream_decoded_dec_order)
    command_line.call_indented(dec_order_cmd, err_log_path=err_log_path)

    raw_video.mux(hq_bitstream_decoded_dec_order, err_log_path=err_log_path)


    # Downscaling and transcoding loop

    for downscale_parameters in downscale_parameter_list:

        # Generate downscaled dimensions and create folders
        
        (downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
        downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)

        downscale_folder = "%s/%dp" % (sequence_folder, downscaled_height)
        paths.create_if_needed(downscale_folder)


        # Branch 1
        
        ## Downscale decoded HQ bistream (Sender side)
        
        downscaled_file_shortpath = filenames.replace_dimensions(hq_bitstream_decoded_shortpath, downscaled_width, downscaled_height)
        downscaled_file = "%s/%s.yuv" % (downscale_folder, downscaled_file_shortpath)

        downscaling.perform_downscaling(width, height, hq_bitstream_decoded, downscaled_file, downscale_parameters, err_log_path=err_log_path)

        raw_video.mux(downscaled_file, err_log_path=err_log_path)


        ## Re-encode with RDOQ=0 (Sender side)

        rdoq_0_file_shortpath = "%s_rdoq_0" % (downscaled_file_shortpath)
        rdoq_0_file = "%s/%s.bin" % (downscale_folder, rdoq_0_file_shortpath)

        rdoq_0_cmd = "%s -c %s -i %s -b %s -fr %d -f %d -wdt %d -hgt %d --RDOQ=0 -SBH 0 --RDOQTS=0 --SEIDecodedPictureHash=2" % \
            (binaries.hm_encoder, config.cfg_file, downscaled_file, rdoq_0_file, framerate, config.all_frames, downscaled_width, downscaled_height)
        command_line.call_indented(rdoq_0_cmd, err_log_path=err_log_path)


        ## Prune (Sender side)
        ## This is the bitstream to transmit alongside hq_bitstream.

        pruned_file_shortpath = "%s_pruned" % rdoq_0_file_shortpath
        pruned_file = "%s/%s.bin" % (downscale_folder, pruned_file_shortpath)

        prune_cmd = "%s -i %s -n %s" % (binaries.d65_gt_pruning, rdoq_0_file, pruned_file)
        command_line.call_indented(prune_cmd, err_log_path=err_log_path)


        if debug.debug_1:
            # Decode pruned bitstream

            pruned_file_decoded_shortpath = "%s_dec" % pruned_file_shortpath
            pruned_file_decoded = "%s/%s.yuv" % (downscale_folder, pruned_file_decoded_shortpath)

            prune_decoding_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, pruned_file, pruned_file_decoded)
            command_line.call_indented(prune_decoding_cmd, err_log_path=err_log_path)

            raw_video.mux(pruned_file_decoded, err_log_path=err_log_path)


        # Branch 2

        ## Downscale decoding order HQ bitstream (Receiver side)

        hq_bitstream_decoded_dec_order_downscaled_shortpath = \
            filenames.replace_dimensions(hq_bitstream_decoded_dec_order_shortpath, downscaled_width, downscaled_height)
        hq_bitstream_decoded_dec_order_downscaled = "%s/%s.yuv" % (downscale_folder, hq_bitstream_decoded_dec_order_downscaled_shortpath)

        downscaling.perform_downscaling(width, height, hq_bitstream_decoded_dec_order, 
            hq_bitstream_decoded_dec_order_downscaled, downscale_parameters, err_log_path=err_log_path)

        raw_video.mux(hq_bitstream_decoded_dec_order_downscaled, err_log_path=err_log_path)


        # Put together the branches

        ## Reconstruct residual (Receiver side)

        reconstructed_file_shortpath = "%s_transcoded" % filenames.replace_dimensions(hq_bitstream_shortpath, downscaled_width, downscaled_height)
        reconstructed_file = "%s/%s.bin" % (downscale_folder, reconstructed_file_shortpath)

        res_reconstruct_cmd = "%s -i %s -u %s -n %s" % (binaries.d65_gt_res_reconstruct, pruned_file, 
            hq_bitstream_decoded_dec_order_downscaled, reconstructed_file)
        command_line.call_indented(res_reconstruct_cmd, err_log_path=err_log_path)


        ## Decode transcoded video (Receiver side)

        reconstructed_file_decoded_shortpath = "%s_dec" % reconstructed_file_shortpath
        reconstructed_file_decoded = "%s/%s.yuv" % (downscale_folder, reconstructed_file_decoded_shortpath)

        res_reconstruct_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, reconstructed_file, reconstructed_file_decoded)
        command_line.call_indented(res_reconstruct_decode_cmd, err_log_path=err_log_path)

        raw_video.mux(reconstructed_file_decoded, err_log_path=err_log_path)

def iterate():

    orig_draft = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23"
    originals = [ orig_draft + "/BQSquare_416x240_60.yuv", orig_draft + "/RaceHorses_416x240_30.yuv"]
    
    QP_hq = [22, 27, 32, 37]
    QP_lq = [qp + 2 for qp in QP_hq]

    for original_file in originals:

        # Original sequence

        original_file_basename = os.path.basename(original_file)
        original_file_shortpath = os.path.splitext(original_file_basename)[0]

        (width, height)  = filenames.extract_dimensions(original_file_shortpath)

        cfg_mode = filenames.extract_cfg_mode(config.cfg_file)


        # Encoded files

        output_file_framerate_replaced = filenames.replace_framerate(original_file_shortpath, config.framerate)
        output_file_mode_info_added = "%s_%df_%s" % (output_file_framerate_replaced, config.frames, cfg_mode)

        sequence_folder = "%s/%s" % (directories.bitstream_folder, output_file_mode_info_added)
        paths.create_if_needed(sequence_folder)

        for qp_hq in QP_hq:

            # Output file

            output_file_qp_hq_shortpath = "%s_qp%d" % (output_file_mode_info_added, qp_hq)
            output_file_qp_hq = "%s/%s.bin" % (sequence_folder, output_file_qp_hq_shortpath)

            encode_cmd = "%s -c %s -i %s -b %s -fr %s -f %s -hgt %s -wdt %s -SBH 1 --SEIDecodedPictureHash=2" % \
                (binaries.hm_encoder, cfg_file, original_file, output_file_qp_hq, config.framerate, config.frames, height, width)
            command_line.call_indented(encode_cmd)
            subprocess.call(encode_cmd, shell=True)
            #subprocess.call("touch " + output_file, shell=True)

        #for s in [720, 536, 360]:
        #    for qp_lq in QP_lq:

                #print "%s_%d_%s_%d" % (sequence, qp_hq, s, qp_lq)

                #Reencode