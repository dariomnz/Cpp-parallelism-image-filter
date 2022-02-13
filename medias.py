import sys

# print ('Number of arguments:', len(sys.argv), 'arguments.')
# print ('Argument List:', str(sys.argv))

if  len(sys.argv) != 2:
    exit()

file1 = open(sys.argv[1], 'r') 
count = 0

class file:
    def __init__(self,name):
        self.name = name
        self.total_time = 0
        self.load_time = 0
        self.gauss_time = 0
        self.sobel_time = 0
        self.store_time = 0
        self.count = 0

chica = file("chica.bmp")
luna = file("luna.bmp")
poema = file("poesia.bmp")

temp_load_time = 0
temp_gauss_time = 0
temp_sobel_time = 0
temp_store_time = 0
current_file : file = None
while True: 
    count += 1
  
    # Get next line from file 
    line = file1.readline() 
  
    # if line is empty 
    # end of file is reached 
    if not line: 
        break

    part_line = line.strip().split(": ")

    if "chica.bmp" in part_line[1]:
        current_file = chica
        current_file.total_time += int(part_line[2].strip(')'))
        current_file.count += 1
    elif "luna.bmp" in part_line[1]:
        current_file = luna
        current_file.total_time += int(part_line[2].strip(')'))
        current_file.count += 1
    elif "poesia.bmp" in part_line[1]:
        current_file = poema
        current_file.total_time += int(part_line[2].strip(')'))
        current_file.count += 1



    if part_line[0]=="Load time":
        current_file.load_time += int(part_line[1])
    elif part_line[0]=="Gauss time":
        current_file.gauss_time += int(part_line[1])
    elif part_line[0]=="Sobel time":
        current_file.sobel_time += int(part_line[1])
    elif part_line[0]=="Store time":
        current_file.store_time += int(part_line[1])


for file in [chica,luna,poema]:
    print("File: \"{}\"(time: {:.2f})".format(file.name, file.total_time/file.count)) 
    print("\tLoad time: {:.2f}".format(file.load_time/file.count)) 
    if file.gauss_time != 0:
        print("\tGauss time: {:.2f}".format(file.gauss_time/file.count)) 
    if file.sobel_time != 0:
        print("\tSobel time: {:.2f}".format(file.sobel_time/file.count)) 
    print("\tStore time: {:.2f}".format(file.store_time/file.count)) 
  
file1.close() 