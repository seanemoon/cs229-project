##### Webcam Object for object detection and tracking #####
### Class constructor ###
# Webcam(online,path,resize = None,BSHistory = 500, BSThreshold = 16, minBlobAreaRatio = 0.0002, maxBlobAreaRatio = 0.2) 
# ----- Arguments -----
# online (required)          : input True if image source is an online webcam, input False if image source is images in a folder
# path   (required)          : query URL if image source is an online webcam, image directory path is image source is images in a folder 
# resize (optional)          : input tuple of (newYresolution, newXresolution) if resizing of image is desired. By default, no resizing occurs
# BSHistory (optional)       : Controls how long the background subtractor remembers previous frames for. By default, set to 30
# BSThreshold (optional)     : Controls the threshold above which the background subtractor classifies a pixel as foreground. By default, set to 10
# minBlobAreaRatio (optional): Minimum percentage of picture area a blob must be to be classified as foreground. By default, set to 0.0002
# maxBlobAreaRatio (optional): Maximum percentage of picture area a blob must be to be classified as background. By default, set to 0.2
#
### Instance Methods ###
# update()                  : Call this function to get a new image from image source and process it. 
# score()                   : Call this function to get the current image score for the webcam. Currently returns blob area as percentage of total image area
# image()                   : Returns most recent image of webcam (in the form of a numpy array)
# overlaidImage()           : Returns most recent image of webcam overlaid with object detection contours and bounding boxes
# foreground()              : Returns binary foreground mask of webcam from background detector
# background()              : Returns most recent background image of webcam from background detector
# patches(squareSize, num)  : Returns list of images from bounding boxes of detected objects. Returns an empty list if no objects are detected.
#							  If the optional squareSize argument is provided, images are resized to be of size squareSize x squareSize. By default, images are not resized
#							  If the optional num argument is provided, up to num number of images are returned. By default, all images are returned in the list.	
#

import cv2
import numpy as np
import time
from socket import timeout
from skimage import io
import os

class Webcam:
	def __init__(self,online, path ,resize = None,BSHistory = 30, BSThreshold = 10, minBlobAreaRatio = 0.0002, maxBlobAreaRatio = 0.2):
		self.online = online
		self.backgroundMOG = cv2.createBackgroundSubtractorMOG2(history = BSHistory, varThreshold = BSThreshold, detectShadows = False)
		self.minBlobRatio = minBlobAreaRatio
		self.maxBlobRatio = maxBlobAreaRatio 
		self.bb = []
		readImg = None;

		if online:
			self.path = path		
			self.connected = False
			self.respTime = 99999

			try:
				startTime = time.clock()
				readImg = cv2.cvtColor(io.imread(self.path),cv2.COLOR_BGR2RGB)
				endTime = time.clock()
				self.respTime = endTime-startTime
				self.connected = True

			except urllib2.URLError:
				print "URLError : " + self.path
				pass
			except httplib.BadStatusLine:
				print "HTTPError : " + self.path
				pass
			except timeout:
				print "Timed out : " + self.path
			except:
				print "Some other Exception : " + self.path
				pass
		else :
			self.path = os.listdir(path)
			self.path = map(lambda x: path+"\\"+x,self.path)
			self.numImgs = len(self.path)
			self.imgIdx = 0
		
			readImg = cv2.imread(self.path[self.imgIdx],1)

		self.contours = [];
		self.contourArea = 0.0;

		if resize:
			self.size = resize
			self.numPixels = self.size[0]*self.size[1]
			self.img = np.zeros(resize+tuple([3]))
		else:
			self.img = readImg
			self.size = self.img.shape[0:2]
			self.numPixels = self.size[0]*self.size[1]

		self.foremask = np.zeros_like(self.img)
			
		if readImg is not None:
			self.img = cv2.resize(readImg,(self.size[1],self.size[0]))
			#blurimg = cv2.GaussianBlur(self.img,(5,5),0)
			self.foremask = self.backgroundMOG.apply(self.img)

	def update(self):
		readImg = None
		if self.online:
			try:
				readImg = cv2.cvtColor(io.imread(self.path),cv2.COLOR_BGR2RGB)
				self.connected = True

			except urllib2.URLError:
				self.connected = False
				pass
			except httplib.BadStatusLine:
				self.connected = False
				pass
			except timeout:
				self.connected = False
				pass
		else:
			if not self.imgIdx == self.numImgs:
				readImg = cv2.imread(self.path[self.imgIdx],1)
				self.imgIdx = self.imgIdx+1

		if readImg is not None:

			self.img = cv2.resize(readImg,(self.size[1],self.size[0]))
			self.overlaid = np.copy(self.img)
			#blurimg = cv2.GaussianBlur(self.img,(5,5),0)
			self.foremask = self.backgroundMOG.apply(self.img)
			self.foremask = cv2.morphologyEx(self.foremask,cv2.MORPH_CLOSE,np.ones((2,2),np.uint8))

			contourCpy = np.copy(self.foremask)
			self.contours = cv2.findContours(contourCpy,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_NONE)[1]
			self.contourArea = 0.0;
			if self.contours:
				filteredContours = [];
				for i,contour in enumerate(self.contours):
					cA = cv2.contourArea(contour)

					if cA > self.minBlobRatio*self.numPixels and cA < self.maxBlobRatio*self.numPixels:
						self.contourArea = self.contourArea + cA
						filteredContours.append(contour)
						
				self.contours = sorted(filteredContours, key = cv2.contourArea, reverse = True)
				self.bb = [cv2.boundingRect(cntr) for cntr in self.contours]

				cv2.drawContours(self.overlaid,self.contours,-1,(0,255,0), thickness = 1)
				for rect in self.bb:
					cv2.rectangle(self.overlaid,(rect[0],rect[1]),(rect[0]+rect[2],rect[1]+rect[3]),(0,0,255),thickness = 1)


	def score(self):
		return self.contourArea/self.numPixels

	def image(self):
		return self.img
	
	def overlaidImage(self):
		return self.overlaid

	def foreground(self):
		return self.foremask

	def background(self):
		return self.backgroundMOG.getBackgroundImage()

	def patches(self, squareSize = 0, num = 0):
		queryBB = self.bb[:max(0,min(num,len(self.bb)-1))]

		bbPatches = [self.img[rect[1]:rect[1]+rect[3],rect[0]:rect[0]+rect[2]] for rect in queryBB]
		if squareSize > 0:
			bbPatches = map(lambda x : cv2.resize(x,(squareSize,squareSize)),bbPatches)

		return bbPatches



