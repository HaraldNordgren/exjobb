import sys, os, subprocess
import downscaling, filenames, paths, raw_video
import encode_original_module

import definitions.config       as config
import definitions.debug        as debug
import definitions.directories  as directories
import definitions.binaries     as binaries


def transcode(hq_bitstream):

    hq_bitstream_basename = os.path.basename(hq_bitstream)
    hq_bitstream_shortpath = os.path.splitext(hq_bitstream_basename)[0]

    (width, height) = filenames.extract_dimensions(hq_bitstream_basename)

    """
    if height != 1080:
    	raise Exception("Expected 1080p video")
    """

    # Create folders and logs

    sequence_folder = "%s/%s" % (directories.output_folder, hq_bitstream_shortpath)
    paths.remove_and_recreate_directory(sequence_folder)

    err_log_path = "%s/err.txt" % sequence_folder
    err_log = open(err_log_path, 'a+')


    # Preprocessing

    ## Decode HQ bitstream (Sender side)

    hq_bitstream_decoded_shortpath = "%s_dec" % hq_bitstream_shortpath
    hq_bitstream_decoded = "%s/%s.yuv" % (sequence_folder, hq_bitstream_decoded_shortpath)

    hq_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, hq_bitstream, hq_bitstream_decoded)
    subprocess.call(hq_decode_cmd, shell=True, stderr=err_log)

    raw_video.mux(hq_bitstream_decoded)


    ## Decode HQ bitstream in decoding order (Receiver side)

    hq_bitstream_decoded_dec_order_shortpath = "%s_dec_dec_order" % hq_bitstream_shortpath
    hq_bitstream_decoded_dec_order = "%s/%s.yuv" % (sequence_folder, hq_bitstream_decoded_dec_order_shortpath)

    dec_order_cmd = "%s -i %s -o %s" % (binaries.d65_gt_dec_order, hq_bitstream, hq_bitstream_decoded_dec_order)
    subprocess.call(dec_order_cmd, shell=True, stderr=err_log)

    raw_video.mux(hq_bitstream_decoded_dec_order)


    # Downscaling and transcoding loop

    """
    [1], [0], [1,0] means two-thirds, half and one-third downscaling.
    This corresponds to 720p, 536p and 360p for a 1080p video.
    """

    #downscale_parameter_list = [[1], [0], [1,0]]
    downscale_parameter_list = [[0], [0,0]]

    for downscale_parameters in downscale_parameter_list:

        # Generate downscaled dimensions and create folders
        
        (downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
        downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)

        downscale_folder = "%s/%d" % (sequence_folder, downscaled_height)
        paths.create_if_needed(downscale_folder)


        # Branch 1
        
        ## Downscale decoded HQ bistream (Sender side)
        
        downscaled_file_shortpath = filenames.replace_dimensions(hq_bitstream_decoded_shortpath, downscaled_width, downscaled_height)
        downscaled_file = "%s/%s.yuv" % (downscale_folder, downscaled_file_shortpath)

        downscaling.perform_downscaling(width, height, hq_bitstream_decoded, downscaled_file, downscale_parameters)

        raw_video.mux(downscaled_file)


        ## Re-encode with RDOQ=0 (Sender side)

        rdoq_0_file_shortpath = "%s_rdoq_0" % (downscaled_file_shortpath)
        rdoq_0_file = "%s/%s.bin" % (downscale_folder, rdoq_0_file_shortpath)

        rdoq_0_cmd = "%s -c %s -i %s -b %s -fr %d -f %d -wdt %d -hgt %d --RDOQ=0 -SBH 0 --RDOQTS=0" % (binaries.hm_encoder, config.cfg_file, 
            downscaled_file, rdoq_0_file, config.framerate, config.all_frames, downscaled_width, downscaled_height)
        subprocess.call(rdoq_0_cmd, shell=True, stderr=err_log)


        ## Prune (Sender side)
        ## This is the bitstream to transmit alongside hq_bitstream.

        pruned_file_shortpath = "%s_pruned" % rdoq_0_file_shortpath
        pruned_file = "%s/%s.bin" % (downscale_folder, pruned_file_shortpath)

        prune_cmd = "%s -i %s -n %s" % (binaries.d65_gt_pruning, rdoq_0_file, pruned_file)
        subprocess.call(prune_cmd, shell=True, stderr=err_log)


        if debug.debug_1:
            # Decode pruned bitstream

            pruned_file_decoded_shortpath = "%s_dec" % pruned_file_shortpath
            pruned_file_decoded = "%s/%s.yuv" % (downscale_folder, pruned_file_decoded_shortpath)

            prune_decoding_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, pruned_file, pruned_file_decoded)
            subprocess.call(prune_decoding_cmd, shell=True, stderr=err_log)

            raw_video.mux(pruned_file_decoded)


        # Branch 2

        ## Downscale decoding order HQ bitstream (Receiver side)

        hq_bitstream_decoded_dec_order_downscaled_shortpath = \
            filenames.replace_dimensions(hq_bitstream_decoded_dec_order_shortpath, downscaled_width, downscaled_height)
        hq_bitstream_decoded_dec_order_downscaled = "%s/%s.yuv" % (downscale_folder, hq_bitstream_decoded_dec_order_downscaled_shortpath)

        downscaling.perform_downscaling(width, height, hq_bitstream_decoded_dec_order, hq_bitstream_decoded_dec_order_downscaled, downscale_parameters)

        raw_video.mux(hq_bitstream_decoded_dec_order_downscaled)


        # Put together the branches

        ## Reconstruct residual (Receiver side)

        reconstructed_file_shortpath = "%s_transcoded" % filenames.replace_dimensions(hq_bitstream_shortpath, downscaled_width, downscaled_height)
        reconstructed_file = "%s/%s.bin" % (downscale_folder, reconstructed_file_shortpath)

        res_reconstruct_cmd = "%s -i %s -u %s -n %s" % (binaries.d65_gt_res_reconstruct, pruned_file, 
            hq_bitstream_decoded_dec_order_downscaled, reconstructed_file)
        subprocess.call(res_reconstruct_cmd, shell=True, stderr=err_log)


        ## Decode transcoded video (Receiver side)

        reconstructed_file_decoded_shortpath = "%s_dec" % reconstructed_file_shortpath
        reconstructed_file_decoded = "%s/%s.yuv" % (downscale_folder, reconstructed_file_decoded_shortpath)

        res_reconstruct_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, reconstructed_file, reconstructed_file_decoded)
        subprocess.call(res_reconstruct_decode_cmd, shell=True, stderr=err_log)

        raw_video.mux(reconstructed_file_decoded)

    err_log.close()

def iterate():

    sequences = ['sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQSquare_416x240_60.yuv']
    
    QP_hq = [22, 27, 32, 37]
    QP_lq = [qp + 2 for qp in QP_hq]

    for sequence in sequences:

        original_file_basename = os.path.basename(original_file)
        original_file_shortpath = os.path.splitext(original_file_basename)[0]

        (width, height)  = filenames.extract_dimensions(original_file_shortpath)

        cfg_file_basename = os.path.basename(cfg_file)
        cfg_mode = filenames.extract_cfg_mode(cfg_file_basename)

        output_file_framerate_replaced = filenames.replace_framerate(original_file_shortpath, config.framerate)
        output_file_info_added = "%s_%s_%d-frames" % (output_file_framerate_replaced, cfg_mode, config.frames)

        output_folder = "%s/%s" % (directories.bitstream_folder, output_file_info_added)
        paths.create_if_needed(output_folder)

        for qp_hq in QP_hq:

            # Output file

            output_file_shortpath = "%s_qp-%d" % (output_file_info_added, qp)
            output_file = "%s/%s.bin" % (output_folder, output_file_shortpath)

            encode_cmd = "%s -c %s -i %s -b %s -fr %s -f %s -hgt %s -wdt %s -SBH 1" % \
                (binaries.hm_encoder, cfg_file, original_file, output_file, config.framerate, config.frames, height, width)
            subprocess.call(encode_cmd, shell=True)
            #subprocess.call("touch " + output_file, shell=True)

        #for s in [720, 536, 360]:
        #    for qp_lq in QP_lq:

                #print "%s_%d_%s_%d" % (sequence, qp_hq, s, qp_lq)

                #Reencode