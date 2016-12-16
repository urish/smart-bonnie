import os

samples_cpp = open("../src/samples.h", "w")
for filename in os.listdir('.'):
	if filename.endswith('.raw'):
		samplename = filename.split('.')[0]
		print (samplename)
		file = open(filename, "rb")
		buffer = file.read()
		data = "{" + ", ".join(map(str, map(ord, buffer))) + "}"
		samples_cpp.write("static uint8_t sample_{0}[] = {1};\n".format(samplename, data))
		samples_cpp.write("static uint32_t sample_{0}_size = sizeof(sample_{0});\n".format(samplename))
samples_cpp.close()
