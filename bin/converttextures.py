import glob
import os

files = glob.glob('*.jpg')
for i in files:
	os.system('"' + os.environ['DXSDK_DIR'] + 'Utilities/bin/x64/texconv" -f DXT1 -if TRIANGLE -m -1 -ft DDS -nologo -srgbi ' + i)