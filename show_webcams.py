#### Script to display top webcams #####
# Example usage - show_webcams.py C:\webcamPicturesFolderDir 350 -p 300 -d 10 
# 1st arg : Directory containing webcam image folders
# 2nd arg : Number of webcams to process (samples randomly among all webcam folders in Dir)
# 3rd arg -p (optional) : Side dimension of square images that are displayed. Default 200 
# 4th arg -d (optional) : Number of webcam images to display. Default 5

import numpy as np 
import cv2
import os
import sys
import random
import argparse

from detector.Webcam import Webcam

parser = argparse.ArgumentParser()
parser.add_argument("Dir",help = "Directory containing webcam image folders")
parser.add_argument("numWebcamsToSample",help = "Number of webcams to sample", type = int)
parser.add_argument("-p","--picSize",help = "Side dimension of square images to be displayed",type = int, default = 200)
parser.add_argument("-d","--numDisplays", help = "Number of top webcam images to display", type = int, default = 5)
args = parser.parse_args()

Dir = args.Dir
sampleNum = args.numWebcamsToSample
picSize = args.picSize
numDisplays = args.numDisplays

webcamList = os.listdir(Dir)
webcamList = map(lambda x: Dir+"\\"+x, webcamList)

sampleIdx = random.sample(xrange(0,len(webcamList)),sampleNum)
webcamList = [webcamList[i] for i in sampleIdx]

webcams = map(lambda x : Webcam(online = False,resize = (picSize,picSize),path = x),webcamList)

while True:
	map(lambda x: x.update(),webcams)

	webcams = sorted(webcams,key = lambda x: x.score(),reverse = True)

	overlays = webcams[0].overlaidImage()
	for i in range(1,numDisplays):
		overlays = np.hstack((overlays,webcams[i].overlaidImage()))

	backgrounds = webcams[0].background()
	for i in range(1,numDisplays):
		backgrounds = np.hstack((backgrounds,webcams[i].background()))	

	display = np.vstack((overlays,backgrounds))

	cv2.imshow("Display",display)

	cv2.waitKey(10)
