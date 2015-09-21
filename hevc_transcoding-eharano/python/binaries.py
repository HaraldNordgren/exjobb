import os

if 0:

	hm_encoder				= os.path.abspath("bin/windows/HM-16.6/TAppEncoder.exe")
	hm_decoder				= os.path.abspath("bin/windows/HM-16.6/TAppDecoder.exe")

	downscaler				= os.path.abspath("bin/windows/downScale/downScale.exe")

	d65_gt_pruning			= os.path.abspath("d65_gt/bin/d65_dec_pruning.exe")
	d65_gt_dec_order		= os.path.abspath("d65_gt/bin/d65_dec_dec_order.exe")
	d65_gt_res_reconstruct	= os.path.abspath("d65_gt/bin/d65_dec_residual_reconstruct.exe")

	#d65_gt_pruning			= os.path.abspath("bin/windows/d65_gt/d65_dec_pruning.exe")
	#d65_gt_dec_order		= os.path.abspath("bin/windows/d65_gt/d65_dec_dec_order.exe")
	#d65_gt_res_reconstruct	= os.path.abspath("bin/windows/d65_gt/d65_dec_residual_reconstruct.exe")


else:

	hm_encoder				= "echo hm_encoder"
	hm_decoder				= "echo hm_decoder"

	downscaler				= "echo downscaler"

	d65_gt_pruning			= "echo d65_gt_pruning"
	d65_gt_dec_order		= "echo d65_gt_dec_order"
	d65_gt_res_reconstruct	= "echo d65_gt_res_reconstruct"