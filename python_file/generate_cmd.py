import os,sys

def read_path(dir_name):
	for root, dirs, files in os.walk(dir_name):
#		print('root_dir:', root)
#		print('sub_dirs:', dirs)
#	        print('files:', files)
		for lines in files:
                        cmd = "time ../convert_trace /mnt/ssd/lhf/" + lines + " /mnt/ssd/lhf/simpletrace/" + lines.replace(".trace",".simpletrace") + " > log."+lines + ";"
#			print(lines)
                        print(cmd)
#			print(lines.replace(".trace",".simpletrace"))
#			print("log." + lines)
		


dir_name = sys.argv[1]
read_path(dir_name)




