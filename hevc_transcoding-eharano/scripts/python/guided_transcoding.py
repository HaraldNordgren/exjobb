import sys, os, subprocess, time
import downscaling, filenames, paths, raw_video, time_string, command_line
import encode_original_module

import definitions.config       as config
import definitions.debug        as debug
import definitions.directories  as directories
import definitions.binaries     as binaries


orig_draft = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23"
#originals = [ orig_draft + "/BQSquare_416x240_60.yuv", orig_draft + "/RaceHorses_416x240_30.yuv"]
originals = [ orig_draft + "/BQSquare_416x240_60.yuv" ]

# [1], [0], [1,0] means two-thirds, half and one-third downscaling.
# This corresponds to 720p, 536p and 360p for a 1080p video.
#downscale_parameter_list = [[1], [0], [1,0]]
#downscale_parameter_list = [ [1,0,0], [1,0,0,0] ]
#downscale_parameter_list = [ [0], [0,0] ]
downscale_parameter_list = [ [0] ]

#QP_hq = [22, 27, 32, 37]
QP_hq = [27]
QP_lq = [qp + 2 for qp in QP_hq]


#Downscaled originals, then compare to transcoded one

def transcode():

    start_time = time.time()

    transcoding_folder = "%s/transcoding_%s" % (directories.output_folder, time_string.current())
    err_log_path = "%s/error_log.txt" % transcoding_folder

    cfg_mode = filenames.extract_cfg_mode(config.cfg_file)

    for original_file in originals:

        # Original sequence

        original_file_basename = os.path.basename(original_file)
        original_file_shortpath = os.path.splitext(original_file_basename)[0]

        (width, height)  = filenames.extract_dimensions(original_file_shortpath)

        hq_bitstream_framerate_replaced = filenames.replace_framerate(original_file_shortpath, config.framerate)
        hq_bitstream_mode_info_added = "%s_%df_%s" % (hq_bitstream_framerate_replaced, config.frames, cfg_mode)

        sequence_folder = "%s/%s" % (transcoding_folder, hq_bitstream_mode_info_added)


        # Create downscaled originals

        downscaled_originals_folder = "%s/downscaled_originals" % sequence_folder
        paths.create_if_needed(downscaled_originals_folder)

        downscaled_dimensions   = []
        downscaled_originals    = []
        
        for downscale_parameters in downscale_parameter_list:
            
            (downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
            downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)
        
            downscaled_original_shortpath = filenames.replace_dimensions(original_file_shortpath, downscaled_width, downscaled_height)
            downscaled_original = "%s/%s.yuv" % (downscaled_originals_folder, downscaled_original_shortpath)

            downscaling.perform_downscaling(width, height, original_file, downscaled_original, downscale_parameters, err_log_path=err_log_path)

            raw_video.mux(downscaled_original, err_log_path=err_log_path)

            downscaled_dimensions.append((downscaled_width, downscaled_height))
            downscaled_originals.append(downscaled_original)


        for qp_hq in QP_hq:

            # Encode original sequence with given QP (Sender side)

            qp_hq_string = "qp%d" % qp_hq

            hq_bitstream_shortpath = "%s_%s" % (hq_bitstream_mode_info_added, qp_hq_string)

            hq_bitstream_folder = "%s/%s" % (sequence_folder, qp_hq_string)
            paths.create_if_needed(hq_bitstream_folder)

            hq_bitstream = "%s/%s.bin" % (hq_bitstream_folder, hq_bitstream_shortpath)

            encode_hq_cmd = "%s -c %s -i %s -b %s -q %d -fr %s -f %s -wdt %s -hgt %s -SBH 1 --SEIDecodedPictureHash=2" % \
                (binaries.hm_encoder, config.cfg_file, original_file, hq_bitstream, qp_hq, config.framerate, config.frames, width, height)
            command_line.call_indented(encode_hq_cmd, err_log_path=err_log_path)


            # Encode downscaled originals too

            downscaled_originals_encoded = []

            downscaled_originals_encoded_folder = "%s/downscaled_originals_encoded" % hq_bitstream_folder
            paths.create_if_needed(downscaled_originals_encoded_folder)

            for i in range(len(downscaled_originals)):
                
                downscaled_original = downscaled_originals[i]

                downscaled_original_basename = os.path.basename(downscaled_original)
                downscaled_original_shortpath = os.path.splitext(downscaled_original_basename)[0]
                downscaled_original_encoded = "%s/%s_%s.bin" % (downscaled_originals_encoded_folder, downscaled_original_shortpath, qp_hq_string)

                (downscaled_width, downscaled_height) = downscaled_dimensions[i]

                encode_downscaled_original_cmd = "%s -c %s -i %s -b %s -q %d -fr %s -f %s -wdt %s -hgt %s -SBH 1 --SEIDecodedPictureHash=2" % \
                    (binaries.hm_encoder, config.cfg_file, downscaled_original, downscaled_original_encoded, 
                    qp_hq, config.framerate, config.frames, downscaled_width, downscaled_height)
                command_line.call_indented(encode_downscaled_original_cmd, err_log_path=err_log_path)

                downscaled_originals_encoded.append(downscaled_original_encoded)


            ## Decode HQ bitstream (Sender side)

            hq_bitstream_decoded_shortpath = "%s_dec" % hq_bitstream_shortpath
            hq_bitstream_decoded = "%s/%s.yuv" % (hq_bitstream_folder, hq_bitstream_decoded_shortpath)

            hq_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, hq_bitstream, hq_bitstream_decoded)
            command_line.call_indented(hq_decode_cmd, err_log_path=err_log_path)

            raw_video.mux(hq_bitstream_decoded, err_log_path=err_log_path)


            ## Decode HQ bitstream in decoding order (Receiver side)

            hq_bitstream_decoded_dec_order_shortpath = "%s_dec_dec_order" % hq_bitstream_shortpath
            hq_bitstream_decoded_dec_order = "%s/%s.yuv" % (hq_bitstream_folder, hq_bitstream_decoded_dec_order_shortpath)

            dec_order_cmd = "%s -i %s -o %s" % (binaries.d65_gt_dec_order, hq_bitstream, hq_bitstream_decoded_dec_order)
            command_line.call_indented(dec_order_cmd, err_log_path=err_log_path)

            raw_video.mux(hq_bitstream_decoded_dec_order, err_log_path=err_log_path)


            # Downscaling and transcoding loop

            #for downscale_parameters in downscale_parameter_list:
            for i in range(len(downscale_parameter_list)):

                downscale_parameters = downscale_parameter_list[i]

                # Generate downscaled dimensions and create folders
                
                #(downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
                #downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)

                (downscaled_width, downscaled_height) = downscaled_dimensions[i]

                downscale_folder = "%s/%dp" % (hq_bitstream_folder, downscaled_height)
                paths.create_if_needed(downscale_folder)


                # Branch 1
                ## Downscale decoded HQ bistream (Sender side)
                
                downscaled_file_shortpath = filenames.replace_dimensions(hq_bitstream_decoded_shortpath, downscaled_width, downscaled_height)
                downscaled_file = "%s/%s.yuv" % (downscale_folder, downscaled_file_shortpath)

                downscaling.perform_downscaling(width, height, hq_bitstream_decoded, downscaled_file, downscale_parameters, err_log_path=err_log_path)

                raw_video.mux(downscaled_file, err_log_path=err_log_path)


                # Branch 2
                ## Downscale decoding order HQ bitstream (Receiver side)

                hq_bitstream_decoded_dec_order_downscaled_shortpath = \
                    filenames.replace_dimensions(hq_bitstream_decoded_dec_order_shortpath, downscaled_width, downscaled_height)
                hq_bitstream_decoded_dec_order_downscaled = "%s/%s.yuv" % \
                    (downscale_folder, hq_bitstream_decoded_dec_order_downscaled_shortpath)

                downscaling.perform_downscaling(width, height, hq_bitstream_decoded_dec_order, 
                    hq_bitstream_decoded_dec_order_downscaled, downscale_parameters, err_log_path=err_log_path)

                raw_video.mux(hq_bitstream_decoded_dec_order_downscaled, err_log_path=err_log_path)


                for qp_lq in QP_lq:

                    # Branch 1
                    ## Re-encode with RDOQ=0 (Sender side)

                    qp_lq_string = "qp%d" % qp_lq

                    lq_bitstream_folder = "%s/%s" % (downscale_folder, qp_lq_string)
                    paths.create_if_needed(lq_bitstream_folder)

                    rdoq_0_file_shortpath = "%s_rdoq_0_%s" % (downscaled_file_shortpath, qp_lq_string)
                    rdoq_0_file = "%s/%s.bin" % (lq_bitstream_folder, rdoq_0_file_shortpath)

                    rdoq_0_cmd = "%s -c %s -i %s -b %s -q %d -fr %d -f %d -wdt %d -hgt %d --RDOQ=0 -SBH 0 --RDOQTS=0 --SEIDecodedPictureHash=2" % \
                        (binaries.hm_encoder, config.cfg_file, downscaled_file, rdoq_0_file, qp_lq, 
                        config.framerate, config.all_frames, downscaled_width, downscaled_height)
                    command_line.call_indented(rdoq_0_cmd, err_log_path=err_log_path)


                    ## Prune (Sender side)
                    ## This is the bitstream to transmit alongside hq_bitstream.

                    pruned_file_shortpath = "%s_pruned" % rdoq_0_file_shortpath
                    pruned_file = "%s/%s.bin" % (lq_bitstream_folder, pruned_file_shortpath)

                    prune_cmd = "%s -i %s -n %s" % (binaries.d65_gt_pruning, rdoq_0_file, pruned_file)
                    command_line.call_indented(prune_cmd, err_log_path=err_log_path)


                    if debug.debug_1:
                        
                        # Decode pruned bitstream

                        pruned_file_decoded_shortpath = "%s_dec" % pruned_file_shortpath
                        pruned_file_decoded = "%s/%s.yuv" % (lq_bitstream_folder, pruned_file_decoded_shortpath)

                        prune_decoding_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, pruned_file, pruned_file_decoded)
                        command_line.call_indented(prune_decoding_cmd, err_log_path=err_log_path)

                        raw_video.mux(pruned_file_decoded, err_log_path=err_log_path)


                    # Put together the branches
                    ## Reconstruct residual (Receiver side)

                    reconstructed_file_shortpath = "%s_qp%d_transcoded" % \
                        (filenames.replace_dimensions(hq_bitstream_shortpath, downscaled_width, downscaled_height), qp_lq)
                    reconstructed_file = "%s/%s.bin" % (lq_bitstream_folder, reconstructed_file_shortpath)

                    res_reconstruct_cmd = "%s -i %s -u %s -n %s" % (binaries.d65_gt_res_reconstruct, pruned_file, 
                        hq_bitstream_decoded_dec_order_downscaled, reconstructed_file)
                    command_line.call_indented(res_reconstruct_cmd, err_log_path=err_log_path)


                    ## Decode transcoded video (Receiver side)

                    reconstructed_file_decoded_shortpath = "%s_dec" % reconstructed_file_shortpath
                    reconstructed_file_decoded = "%s/%s.yuv" % (lq_bitstream_folder, reconstructed_file_decoded_shortpath)

                    res_reconstruct_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, reconstructed_file, reconstructed_file_decoded)
                    command_line.call_indented(res_reconstruct_decode_cmd, err_log_path=err_log_path)

                    raw_video.mux(reconstructed_file_decoded, err_log_path=err_log_path)


                    ## Compare downscaled_originals[i] with reconstructed_file_decoded (with PSNRStatic.exe)
                    ## Compare file size of encoded downscaled_originals_encoded[i], with reconstructed_file (with os.path.getsize)

    end_time = time.time()
    print "total time: %.0fs" % (end_time - start_time)