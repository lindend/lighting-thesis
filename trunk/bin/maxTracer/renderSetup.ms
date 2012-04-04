--define an input file name
--open the file for reading
--read data from file
--for every index in array
--		assign camera corresponding values at a timeframe corresponding to iterator index
--close text file

in_file = openFile "c:\Jakob\cameraValues.txt" 
if in_file != undefined then
(
	num_lines = readValue in_file

	for t = 1 to num_lines do
	(
		
		at time t
		(
			cam = execute ("$'"+"Camera001" + "'")
			select cam
			selection[1].position = [readValue in_file, readValue in_file, readValue in_file]

			target = execute ("$'"+"Camera001.Target" + "'")	
			select target
			selection[1] .position = [readValue in_file, readValue in_file, readValue in_file]
		)
	)
	close in_file
)
