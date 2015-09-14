Dim arg, conf
If WScript.Arguments.Count = 0 Then
   WScript.Echo  "no argument on the command line. If *.xls is located in the same directory, put excel file name as argument. If *.xls is located in different directory, put in the relative path ahead of file name "
Else
   For each arg in WScript.Arguments  
      'WScript.Echo "arg : "  & arg 
      conf = arg
   Next

	Dim loopCount, directory, objExcel, workbook, worksheet, NumberOfSheets, counter
	Set objExcel = CreateObject("Excel.Application")
	'Gets the directory where our script is running from
	directory = CreateObject("Scripting.FileSystemObject").GetParentFolderName(Wscript.ScriptFullName)
	'Open XLS file	
	Set workbook = objExcel.Workbooks.Open(directory & "\" & conf)
	          
	NumberOfSheets = objExcel.ActiveWorkbook.Worksheets.Count
	For counter = 2 to NumberOfSheets
		Set worksheet = objExcel.ActiveWorkbook.Worksheets(counter)
		
		Dim rng
		set rng = worksheet.Range("E5:F8")
		Dim anchor_format_match, test_format_match
		anchor_format_match = worksheet.Cells(5, 5).Value = "" Or worksheet.Cells(6, 5).Value = "" Or worksheet.Cells(7, 5).Value = "" Or worksheet.Cells(8, 5).Value = "" Or worksheet.Cells(5, 6).Value = "" Or worksheet.Cells(6, 6).Value = "" Or worksheet.Cells(7, 6).Value = ""Or worksheet.Cells(8, 6).Value = ""
		test_format_match = worksheet.Cells(5, 11).Value = "" Or worksheet.Cells(6, 11).Value = "" Or worksheet.Cells(7, 11).Value = "" Or worksheet.Cells(8, 11).Value = "" Or worksheet.Cells(5, 12).Value = "" Or worksheet.Cells(6, 12).Value = "" Or worksheet.Cells(7, 12).Value = ""Or worksheet.Cells(8, 12).Value = ""
        If anchor_format_match <> 0 Then
        	WScript.Echo worksheet.Name & " sheet Anchor format not match"
        ElseIf test_format_match <> 0 Then
        	WScript.Echo worksheet.Name & " sheet Test format not match"
        Else
  
	'''''''''''''''''''''''''''''''''''Log chart	
			'Excel does not support long name in sheet title
			If Len(worksheet.Name) > 30 Then
				worksheet.Name = Left(worksheet.Name,30)
			End If
			objExcel.Charts.Add
			objExcel.ActiveChart.SetSourceData rng, 2
			objExcel.ActiveChart.ChartType = 72
			'objExcel.ActiveChart.SeriesCollection.NewSeries
			objExcel.ActiveChart.SeriesCollection(1).XValues = "=" & worksheet.Name & "!R5C5:R8C5"
			objExcel.ActiveChart.SeriesCollection(1).Values = "=" & worksheet.Name & "!R5C6:R8C6"
			objExcel.ActiveChart.SeriesCollection(1).Name = "=""Anchor"""
			
			'objExcel.ActiveChart.SeriesCollection.NewSeries
			objExcel.ActiveChart.SeriesCollection(2).XValues = "=" & worksheet.Name & "!R5C11:R8C11"
			objExcel.ActiveChart.SeriesCollection(2).Values = "=" & worksheet.Name & "!R5C12:R8C12"
			objExcel.ActiveChart.SeriesCollection(2).Name = "=""Test"""
			
			
			objExcel.ActiveChart.HasTitle = True
			objExcel.ActiveChart.ChartTitle.Characters.Text = worksheet.Name


			
			objExcel.ActiveChart.Location 2, worksheet.Name
			Dim psnr_high, psnr_low
			psnr_high = worksheet.Cells(8, 6).Value	
			if psnr_high < worksheet.Cells(8, 12).Value	Then
				psnr_high = worksheet.Cells(8, 12)
			End if
			psnr_low = worksheet.Cells(5, 6).Value	
			if psnr_low > worksheet.Cells(5, 12).Value	Then
			   psnr_low = worksheet.Cells(5, 12)
			End if
			
			Dim bitrate_low
			bitrate_low = worksheet.Cells(5,5).Value
			if bitrate_low > worksheet.Cells(5, 11).Value Then
				bitrate_low = worksheet.Cells(5, 11).Value
			End if
			
			
			objExcel.ActiveChart.Axes(2).Select
			With objExcel.ActiveChart.Axes(2)
			        .MinimumScale = Int(psnr_low-0.2)
			        .MaximumScale = Int(psnr_high+1.2)
			        .MinorUnitIsAuto = True
			        .MajorUnit = 0.5
			End With
			
			objExcel.ActiveChart.Axes(1).Select
			With objExcel.ActiveChart.Axes(1)
			        .MinimumScale = 10 ^ Int(Log(bitrate_low)/Log(10))
			        .MaximumScaleIsAuto = True
			        .MinorUnitIsAuto = True
			        .ScaleType = -4133
			        .MajorTickMark = 1
			        .MinorTickMark = 4
			        .TickLabelPosition = 4
			        
			End With
	
				objExcel.ActiveChart.ChartArea.Select
				With objExcel.ActiveChart.Parent
					.Height = 350
					.Width = 500
					.Top = 350
					.Left = 50
				End With
				'objExcel.ActiveSheet.Shapes("Chart 1").ScaleWidth 0.71 
				'objExcel.ActiveSheet.Shapes("Chart 1").IncrementLeft -300
				'objExcel.ActiveSheet.Shapes("Chart 1").IncrementTop 120

	
	''''''''''''''''''''''''''normal chart				
			objExcel.ActiveChart.Axes(1, 1).HasTitle = True
			objExcel.ActiveChart.Axes(1, 1).AxisTitle.Characters.Text = "bitrate [kbps]"
			objExcel.ActiveChart.Axes(2, 1).HasTitle = True
			objExcel.ActiveChart.Axes(2, 1).AxisTitle.Characters.Text = "PSNR"		
	
			
			objExcel.Charts.Add
			objExcel.ActiveChart.SetSourceData rng, 2
			objExcel.ActiveChart.ChartType = 72
			'objExcel.ActiveChart.SeriesCollection.NewSeries
			objExcel.ActiveChart.SeriesCollection(1).XValues = "=" & worksheet.Name & "!R5C5:R8C5"
			objExcel.ActiveChart.SeriesCollection(1).Values = "=" & worksheet.Name & "!R5C6:R8C6"
			objExcel.ActiveChart.SeriesCollection(1).Name = "=""Anchor"""
			
			'objExcel.ActiveChart.SeriesCollection.NewSeries
			objExcel.ActiveChart.SeriesCollection(2).XValues = "=" & worksheet.Name & "!R5C11:R8C11"
			objExcel.ActiveChart.SeriesCollection(2).Values = "=" & worksheet.Name & "!R5C12:R8C12"
			objExcel.ActiveChart.SeriesCollection(2).Name = "=""Test"""
			
			objExcel.ActiveChart.HasTitle = True
			objExcel.ActiveChart.ChartTitle.Characters.Text = worksheet.Name
			'Excel does not support long name in sheet title
			If Len(worksheet.Name) <= 31 Then
				objExcel.ActiveChart.Location 2, worksheet.Name
			End If
			
			
			objExcel.ActiveChart.Axes(2).Select
			With objExcel.ActiveChart.Axes(2)
			        .MinimumScale = Int(psnr_low-0.2)
			        .MaximumScale = Int(psnr_high+1.2)
			        .MinorUnitIsAuto = True
			        .MajorUnit = 0.5
			End With
			
			objExcel.ActiveChart.Axes(1, 1).HasTitle = True
			objExcel.ActiveChart.Axes(1, 1).AxisTitle.Characters.Text = "bitrate [kbps]"
			objExcel.ActiveChart.Axes(2, 1).HasTitle = True
			objExcel.ActiveChart.Axes(2, 1).AxisTitle.Characters.Text = "PSNR"
			
				objExcel.ActiveChart.ChartArea.Select
				'objExcel.ActiveSheet.DrawingObjects("Chart 2").Placement = 2
				'objExcel.ActiveSheet.DrawingObjects("Chart 2").PrintObject = True
				'objExcel.ActiveSheet.DrawingObjects("Chart 2").Locked = True
				With objExcel.ActiveChart.Parent
					.Height = 350
					.Width = 500
					.Top = 350
					.Left = 600
				End With		
    
	    End If	
	Next	
	'value = worksheet.Cells(loopCount, i).Value
	
	objExcel.Workbooks.Close
	objExcel.quit
	objExcel = Empty
	workbook = Empty
End If