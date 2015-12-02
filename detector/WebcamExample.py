import numpy as np 
import cv2
from Webcam import Webcam


#webcamPath = "C:\Users\G\Desktop\CS229Webcams\ians\opentopia_00005326"   #Folder path here
webcamPath = "http://vso.aa0.netvolante.jp/record/current.jpg"

webcam = Webcam(online = True, path = webcamPath)

while(True):
	webcam.update()

	patches = webcam.patches(squareSize = 224,num = 5)
	if len(patches)>0:
		cv2.imshow('Patches',np.hstack(patches))

	cv2.imshow("Background + Overlays",np.concatenate((webcam.background(),webcam.overlaidImage()),axis = 1))
	cv2.imshow("Foreground Mask",webcam.foreground())

	cv2.waitKey(10)
