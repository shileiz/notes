from subprocess import Popen
import os
import os.path

file_list = [os.path.join(r"D:\video",f) for f in os.listdir(r"D:\video") if not f.endswith(".csv")]

for f in file_list:
    Popen(["Tutorial05.01.pts_dts.exe",f]).wait()
