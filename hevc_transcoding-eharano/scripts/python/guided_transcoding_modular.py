#!/usr/bin/env python

import sys, os, subprocess, time, ast, shutil
import downscaling, filenames, paths, raw_video, time_string, command_line

import definitions.config       as config
import definitions.debug        as debug
import definitions.directories  as directories
import definitions.binaries     as binaries


if len(sys.argv) != 5:
    print "Usage: %s <original-yuv> <qp_hq> <downscale-parameters> <qp_lq>" % os.path.basename(sys.argv[0])
    sys.exit(1)


start_time = time.time()

original_file           = sys.argv[1]
qp_hq                   = int(sys.argv[2])
downscale_parameters    = ast.literal_eval(sys.argv[3])
qp_lq                   = int(sys.argv[4])


#Loop level 1
#for original_file in originals:

original_file_basename = os.path.basename(original_file)
original_file_shortpath = os.path.splitext(original_file_basename)[0]

(width, height)  = filenames.extract_dimensions(original_file_shortpath)

(downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)

cfg_mode = filenames.extract_cfg_mode(config.cfg_file)

hq_bitstream_framerate_replaced = filenames.replace_framerate(original_file_shortpath, config.framerate)
hq_bitstream_mode_info = "%s_%df_%s" % (hq_bitstream_framerate_replaced, config.frames, cfg_mode)

script_id = "%s_%s_%s_%sp_%s" % (time_string.current(), hq_bitstream_mode_info, qp_hq, downscaled_height, qp_lq)

sequence_folder_modular = hq_bitstream_mode_info
tmp_directory = "%s/%s" % (directories.tmp_folder, script_id)

err_log_path = "%s/error_log.txt" % tmp_directory


#Loop level 2
#for downscale_parameters in downscale_parameter_list:

downscaled_originals_folder_modular         = "%s/downscaled_originals" % sequence_folder_modular

downscaled_original_shortpath               = filenames.replace_dimensions(original_file_shortpath, downscaled_width, downscaled_height)
downscaled_original_modular                 = "%s/%s.yuv" % (downscaled_originals_folder_modular, downscaled_original_shortpath)
downscaled_original                         = "%s/%s" % (directories.storage_folder, downscaled_original_modular)


if not os.path.isfile(downscaled_original):

    paths.create_full_directory(tmp_directory, downscaled_originals_folder_modular)
    downscaled_original_tmp = paths.get_full_file(tmp_directory, downscaled_original_modular)

    downscaling.perform_downscaling(width, height, original_file, downscaled_original_tmp, downscale_parameters, err_log_path=err_log_path)
    raw_video.mux(downscaled_original_tmp, err_log_path=err_log_path)

    if not os.path.isfile(downscaled_original):

        paths.create_full_directory(directories.storage_folder, downscaled_originals_folder_modular)
        shutil.copyfile(downscaled_original_tmp, downscaled_original)

#end "#for downscale_parameters in downscale_parameter_list:"


#Loop level 2
#for qp_hq in QP_hq:

qp_hq_string                                = "qp%d" % qp_hq
hq_bitstream_folder_modular                 = "%s/%s" % (sequence_folder_modular, qp_hq_string)

hq_bitstream_shortpath                      = "%s_%s" % (hq_bitstream_mode_info, qp_hq_string)
hq_bitstream_modular                        = "%s/%s.bin" % (hq_bitstream_folder_modular, hq_bitstream_shortpath)
hq_bitstream                                = "%s/%s" % (directories.storage_folder, hq_bitstream_modular)

hq_bitstream_decoded_shortpath              = "%s_dec" % hq_bitstream_shortpath
hq_bitstream_decoded_modular                = "%s/%s.yuv" % (hq_bitstream_folder_modular, hq_bitstream_decoded_shortpath)
hq_bitstream_decoded                        = "%s/%s" % (directories.storage_folder, hq_bitstream_decoded_modular)

hq_bitstream_decoded_dec_order_shortpath    = "%s_dec_dec_order" % hq_bitstream_shortpath
hq_bitstream_decoded_dec_order_modular      = "%s/%s.yuv" % (hq_bitstream_folder_modular, hq_bitstream_decoded_dec_order_shortpath)
hq_bitstream_decoded_dec_order              = "%s/%s" % (directories.storage_folder, hq_bitstream_decoded_dec_order_modular)


if not os.path.isfile(hq_bitstream):

    # Encode original sequence with given QP (Sender side)
    
    paths.create_full_directory(tmp_directory, hq_bitstream_folder_modular)
    hq_bitstream_tmp = paths.get_full_file(tmp_directory, hq_bitstream_modular)

    encode_hq_cmd = "%s -c %s -i %s -b %s -q %d -fr %s -f %s -wdt %s -hgt %s -SBH 1 --SEIDecodedPictureHash=2" % \
        (binaries.hm_encoder, config.cfg_file, original_file, hq_bitstream_tmp, qp_hq, config.framerate, config.frames, width, height)
    command_line.call_indented(encode_hq_cmd, err_log_path=err_log_path)

    if not os.path.isfile(hq_bitstream):

        paths.create_full_directory(directories.storage_folder, hq_bitstream_folder_modular)
        shutil.copyfile(hq_bitstream_tmp, hq_bitstream)
        
        ## Decode HQ bitstream (Sender side)

        hq_bitstream_decoded_tmp = paths.get_full_file(tmp_directory, hq_bitstream_decoded_modular)

        hq_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, hq_bitstream, hq_bitstream_decoded_tmp)
        command_line.call_indented(hq_decode_cmd, err_log_path=err_log_path)
        raw_video.mux(hq_bitstream_decoded_tmp, err_log_path=err_log_path)

        if not os.path.isfile(hq_bitstream_decoded):

            shutil.copyfile(hq_bitstream_decoded_tmp, hq_bitstream_decoded)

            ## Decode HQ bitstream in decoding order (Receiver side)

            hq_bitstream_decoded_dec_order_tmp = paths.get_full_file(tmp_directory, hq_bitstream_decoded_dec_order_modular)

            dec_order_cmd = "%s -i %s -o %s" % (binaries.d65_gt_dec_order, hq_bitstream, hq_bitstream_decoded_dec_order_tmp)
            command_line.call_indented(dec_order_cmd, err_log_path=err_log_path)
            raw_video.mux(hq_bitstream_decoded_dec_order_tmp, err_log_path=err_log_path)

            if not os.path.isfile(hq_bitstream_decoded_dec_order):

                shutil.copyfile(hq_bitstream_decoded_dec_order_tmp, hq_bitstream_decoded_dec_order)



#Loop level 3
#for i in range(len(downscaled_originals)):

downscaled_originals_encoded_folder_modular             = "%s/downscaled_originals_encoded" % hq_bitstream_folder_modular
downscale_folder_modular                                = "%s/%dp" % (hq_bitstream_folder_modular, downscaled_height)

downscaled_original_encoded_modular                     = "%s/%s_%s.bin" % \
    (downscaled_originals_encoded_folder_modular, downscaled_original_shortpath, qp_hq_string)
downscaled_original_encoded                             = "%s/%s" % (directories.storage_folder, downscaled_original_encoded_modular)

downscaled_file_shortpath                               = \
    filenames.replace_dimensions(hq_bitstream_decoded_shortpath, downscaled_width, downscaled_height)
downscaled_file_modular                                 = "%s/%s.yuv" % (downscale_folder_modular, downscaled_file_shortpath)
downscaled_file                                         = "%s/%s" % (directories.storage_folder, downscaled_file_modular)

hq_bitstream_decoded_dec_order_downscaled_shortpath     = \
    filenames.replace_dimensions(hq_bitstream_decoded_dec_order_shortpath, downscaled_width, downscaled_height)
hq_bitstream_decoded_dec_order_downscaled_modular       = "%s/%s.yuv" % \
    (downscale_folder_modular, hq_bitstream_decoded_dec_order_downscaled_shortpath)
hq_bitstream_decoded_dec_order_downscaled               = "%s/%s" % (directories.storage_folder, hq_bitstream_decoded_dec_order_downscaled_modular)


if not os.path.isfile(downscaled_original_encoded):

    # Encode downscaled originals

    paths.create_full_directory(tmp_directory, downscaled_originals_encoded_folder_modular)
    downscaled_original_encoded_tmp = paths.get_full_file(tmp_directory, downscaled_original_encoded_modular)

    encode_downscaled_original_cmd = "%s -c %s -i %s -b %s -q %d -fr %s -f %s -wdt %s -hgt %s -SBH 1 --SEIDecodedPictureHash=2" % \
        (binaries.hm_encoder, config.cfg_file, downscaled_original, downscaled_original_encoded_tmp, 
        qp_hq, config.framerate, config.frames, downscaled_width, downscaled_height)
    command_line.call_indented(encode_downscaled_original_cmd, err_log_path=err_log_path)

    if not os.path.isfile(downscaled_original_encoded):

        paths.create_full_directory(directories.storage_folder, downscaled_originals_encoded_folder_modular)
        shutil.copyfile(downscaled_original_encoded_tmp, downscaled_original_encoded)

        # Branch 1
        ## Downscale decoded HQ bistream (Sender side)

        paths.create_full_directory(tmp_directory, downscale_folder_modular)
        downscaled_file_tmp = paths.get_full_file(tmp_directory, downscaled_file_modular)

        downscaling.perform_downscaling(width, height, hq_bitstream_decoded, downscaled_file_tmp, downscale_parameters, err_log_path=err_log_path)
        raw_video.mux(downscaled_file_tmp, err_log_path=err_log_path)

        if not os.path.isfile(downscaled_file):

            paths.create_full_directory(directories.storage_folder, downscale_folder_modular)
            shutil.copyfile(downscaled_file_tmp, downscaled_file)

            # Branch 2
            ## Downscale decoding order HQ bitstream (Receiver side)

            hq_bitstream_decoded_dec_order_downscaled_tmp = paths.get_full_file(tmp_directory, hq_bitstream_decoded_dec_order_downscaled_modular)

            downscaling.perform_downscaling(width, height, hq_bitstream_decoded_dec_order, 
                hq_bitstream_decoded_dec_order_downscaled_tmp, downscale_parameters, err_log_path=err_log_path)
            raw_video.mux(hq_bitstream_decoded_dec_order_downscaled_tmp, err_log_path=err_log_path)

            if not os.path.isfile(hq_bitstream_decoded_dec_order_downscaled):

                shutil.copyfile(hq_bitstream_decoded_dec_order_downscaled_tmp, hq_bitstream_decoded_dec_order_downscaled)



#Loop level 4
#for qp_lq in QP_lq:

# Branch 1
## Re-encode with RDOQ=0 (Sender side)

qp_lq_string                                = "qp%d" % qp_lq
lq_bitstream_folder_modular                 = "%s/%s" % (downscale_folder_modular, qp_lq_string)

rdoq_0_file_shortpath                       = "%s_rdoq_0_%s" % (downscaled_file_shortpath, qp_lq_string)
rdoq_0_file_modular                         = "%s/%s.bin" % (lq_bitstream_folder_modular, rdoq_0_file_shortpath)
rdoq_0_file                                 = "%s/%s" % (directories.storage_folder, rdoq_0_file_modular)

pruned_file_shortpath                       = "%s_pruned" % rdoq_0_file_shortpath
pruned_file_modular                         = "%s/%s.bin" % (lq_bitstream_folder_modular, pruned_file_shortpath)
pruned_file                                 = "%s/%s" % (directories.storage_folder, pruned_file_modular)

if debug.debug_1:
    pruned_file_decoded_shortpath           = "%s_dec" % pruned_file_shortpath
    pruned_file_decoded_modular             = "%s/%s.yuv" % (lq_bitstream_folder_modular, pruned_file_decoded_shortpath)
    pruned_file_decoded                     = "%s/%s" % (directories.storage_folder, pruned_file_decoded_modular)

hq_bitstream_dimensions_replaced            = filenames.replace_dimensions(hq_bitstream_shortpath, downscaled_width, downscaled_height)
reconstructed_file_shortpath                = "%s_qp%d_transcoded" % (hq_bitstream_dimensions_replaced, qp_lq)
reconstructed_file_modular                  = "%s/%s.bin" % (lq_bitstream_folder_modular, reconstructed_file_shortpath)
reconstructed_file                          = "%s/%s" % (directories.storage_folder, reconstructed_file_modular)

reconstructed_file_decoded_shortpath        = "%s_dec" % reconstructed_file_shortpath
reconstructed_file_decoded_modular          = "%s/%s.yuv" % (lq_bitstream_folder_modular, reconstructed_file_decoded_shortpath)
reconstructed_file_decoded                  = "%s/%s" % (directories.storage_folder, reconstructed_file_decoded_modular)


if not os.path.isfile(rdoq_0_file):

    paths.create_full_directory(tmp_directory, lq_bitstream_folder_modular)
    rdoq_0_file_tmp = paths.get_full_file(tmp_directory, rdoq_0_file_modular)

    rdoq_0_cmd = "%s -c %s -i %s -b %s -q %d -fr %d -f %d -wdt %d -hgt %d --RDOQ=0 -SBH 0 --RDOQTS=0 --SEIDecodedPictureHash=2" % \
        (binaries.hm_encoder, config.cfg_file, downscaled_file, rdoq_0_file_tmp, qp_lq, 
        config.framerate, config.all_frames, downscaled_width, downscaled_height)
    command_line.call_indented(rdoq_0_cmd, err_log_path=err_log_path)

    if not os.path.isfile(rdoq_0_file):

        paths.create_full_directory(directories.storage_folder, lq_bitstream_folder_modular)
        shutil.copyfile(rdoq_0_file_tmp, rdoq_0_file)

        ## Prune (Sender side)
        ## This is the bitstream to transmit alongside hq_bitstream.

        pruned_file_tmp = paths.get_full_file(tmp_directory, pruned_file_modular)

        prune_cmd = "%s -i %s -n %s" % (binaries.d65_gt_pruning, rdoq_0_file, pruned_file_tmp)
        command_line.call_indented(prune_cmd, err_log_path=err_log_path)

        if not os.path.isfile(pruned_file):

            shutil.copyfile(pruned_file_tmp, pruned_file)

            # Decode pruned bitstream

            if debug.debug_1:

                pruned_file_decoded_tmp = paths.get_full_file(tmp_directory, pruned_file_decoded_modular)

                prune_decoding_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, pruned_file, pruned_file_decoded_tmp)
                command_line.call_indented(prune_decoding_cmd, err_log_path=err_log_path)
                raw_video.mux(pruned_file_decoded_tmp, err_log_path=err_log_path)


            # Put together the branches
            
            ## Reconstruct residual (Receiver side)

            reconstructed_file_tmp = paths.get_full_file(tmp_directory, reconstructed_file_modular)

            res_reconstruct_cmd = "%s -i %s -u %s -n %s" % (binaries.d65_gt_res_reconstruct, pruned_file, 
                hq_bitstream_decoded_dec_order_downscaled, reconstructed_file_tmp)
            command_line.call_indented(res_reconstruct_cmd, err_log_path=err_log_path)

            if not os.path.isfile(reconstructed_file):

                shutil.copyfile(reconstructed_file_tmp, reconstructed_file)

                ## Decode transcoded video (Receiver side)

                reconstructed_file_decoded_tmp = paths.get_full_file(tmp_directory, reconstructed_file_modular)

                res_reconstruct_decode_cmd = "%s -b %s -o %s" % (binaries.hm_decoder, reconstructed_file, reconstructed_file_decoded_tmp)
                command_line.call_indented(res_reconstruct_decode_cmd, err_log_path=err_log_path)
                raw_video.mux(reconstructed_file_decoded_tmp, err_log_path=err_log_path)

                if not os.path.isfile(reconstructed_file_decoded):

                    shutil.copyfile(reconstructed_file_decoded_tmp, reconstructed_file_decoded)



## Compare downscaled_originals[i] with reconstructed_file_decoded (with PSNRStatic.exe)
## Compare file size of encoded downscaled_originals_encoded[i], with reconstructed_file (with os.path.getsize)

end_time = time.time()
print "total time: %.0fs" % (end_time - start_time)