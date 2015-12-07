##### Webcam Object for object detection and tracking #####
### Class constructor ###
# Webcam(online,path,resize = None,BSHistory = 50, BSThreshold = 15, minBlobAreaRatio = 0.0003, maxBlobAreaRatio = 0.2)
# ----- Arguments -----
# online (required)          : input True if image source is an online webcam, input False if image source is images in a folder
# path   (required)          : query URL if image source is an online webcam, image directory path is image source is images in a folder
# resize (optional)          : input tuple of (newYresolution, newXresolution) if resizing of image is desired. By default, no resizing occurs
# BSHistory (optional)       : Controls how long the background subtractor remembers previous frames for. By default, set to 50
# BSThreshold (optional)     : Controls the threshold above which the background subtractor classifies a pixel as foreground. By default, set to 15
# minBlobAreaRatio (optional): Minimum percentage of picture area a blob must be to be classified as foreground. By default, set to 0.0003
# maxBlobAreaRatio (optional): Maximum percentage of picture area a blob must be to be classified as background. By default, set to 0.15
#
### Instance Methods ###
# update()                  : Call this function to get a new image from image source and process it.
# score()                   : Call this function to get the current image score for the webcam.
# image()                   : Returns most recent image of webcam (in the form of a numpy array)
# overlaidImage()           : Returns most recent image of webcam overlaid with object detection contours and bounding boxes
# foreground()              : Returns binary foreground mask of webcam from background detector
# background()              : Returns most recent background image of webcam from background detector
# patches(squareSize, num)  : Returns list of images from bounding boxes of detected objects. Returns an empty list if no objects are detected.
#               If the optional squareSize argument is provided, images are resized to be of size squareSize x squareSize. By default, images are not resized
#               If the optional num argument is provided, up to num number of images are returned. By default, all images are returned in the list.
#

import cv2
import numpy as np
import time
import urllib2
import httplib
from socket import timeout
from skimage import io
import os

class Webcam:
  def __init__(self,online, path ,resize = None,BSHistory = 50, BSThreshold = 15, minBlobAreaRatio = 0.0003, maxBlobAreaRatio = 0.15):
    self.online = online
    self.backgroundMOG = cv2.createBackgroundSubtractorMOG2(history = BSHistory, varThreshold = BSThreshold, detectShadows = False)
    self.minBlobRatio = minBlobAreaRatio
    self.maxBlobRatio = maxBlobAreaRatio
    self.bb = []
    readImg = None;
    self.img = None;

    if online:
      self.path = path
      self.connected = False
      self.respTime = 99999

      try:
        startTime = time.clock()

        resource = urllib2.urlopen(self.path,timeout = 10)
        readImg = cv2.imdecode(np.asarray(bytearray(resource.read()),dtype = "uint8"),cv2.IMREAD_COLOR)

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
      frame_filenames = os.listdir(path)
      self.frame_paths = [os.path.join(path, fn) for fn in sorted(frame_filenames)]
      self.imgIdx = 0

      readImg = cv2.imread(self.frame_paths[self.imgIdx], cv2.IMREAD_COLOR)

    self.contours = [];
    self.contourArea = 0.0;

    if resize:
      self.size = resize
      self.numPixels = self.size[0]*self.size[1]
      self.img = np.zeros(resize+tuple([3]))
      self.overlaid = self.img
    elif readImg is not None:
      self.img = readImg
      self.size = self.img.shape[0:2]
      self.numPixels = self.size[0]*self.size[1]

    self.hasImg = False
    if readImg is not None:
      self.hasImg = True
      self.img = cv2.resize(readImg,(self.size[1],self.size[0]))
      #blurimg = cv2.GaussianBlur(self.img,(5,5),0)
      self.foremask = self.backgroundMOG.apply(self.img)

  def update(self):
    readImg = None
    if self.online:
      try:
        resource = urllib2.urlopen(self.path,timeout = 10)
        readImg = cv2.imdecode(np.asarray(bytearray(resource.read()),dtype = "uint8"),cv2.IMREAD_COLOR)
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
      if self.imgIdx < len(self.frame_paths):
        readImg = cv2.imread(self.frame_paths[self.imgIdx],cv2.IMREAD_COLOR)
        self.imgIdx = self.imgIdx+1

    self.hasImg = False
    if readImg is not None:
      self.hasImg = True
      self.img = cv2.resize(readImg,(self.size[1],self.size[0]))
      self.overlaid = np.copy(self.img)
      #blurimg = cv2.GaussianBlur(self.img,(5,5),0)
      self.foremask = self.backgroundMOG.apply(self.img)
      self.foremask = cv2.morphologyEx(self.foremask,cv2.MORPH_CLOSE,np.ones((2,2),np.uint8))

      contourCpy = np.copy(self.foremask)
      self.contours = cv2.findContours(contourCpy,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_NONE)[1]
      self.contourArea = 0.0;
      self.contourArcLength = 0.0;

      if self.contours:
        filteredContours = [];
        for contour in self.contours:
          cA = cv2.contourArea(contour)

          if cA > self.minBlobRatio*self.numPixels and cA < self.maxBlobRatio*self.numPixels:
            self.contourArea = self.contourArea + cA
            self.contourArcLength = self.contourArcLength + cv2.arcLength(contour,True)
            filteredContours.append(contour)

        self.contours = sorted(filteredContours, key = cv2.contourArea, reverse = True)
        self.bb = [cv2.boundingRect(cntr) for cntr in self.contours]

        cv2.drawContours(self.overlaid,self.contours,-1,(0,255,0), thickness = 1)
        for rect in self.bb:
          cv2.rectangle(self.overlaid,(rect[0],rect[1]),(rect[0]+rect[2],rect[1]+rect[3]),(0,0,255),thickness = 1)

  def filtered_overlay(self, classifier, squareSize=227):
    red = (0, 0, 255)
    green = (0, 255, 0)
    blue = (255, 0, 0)

    if self.img is None:
      return None

    overlay = np.copy(self.img)
    cv2.drawContours(overlay, self.contours, -1, green, thickness = 1)

    for i, patch in enumerate(self.patches(squareSize=squareSize)):
      rect = self.bb[i]
      a = (rect[0], rect[1])
      b = (rect[0] + rect[2], rect[1] + rect[3])
      rankings = classifier.rankings(patch)
      labels = set(('human', 'noise', 'animal', 'vehicle'))
      probs  = {}
      for label, idx, prob in rankings:
        if label in labels:
          probs[label] = prob

      if probs['noise'] < 1.0 - 0.03:
        if probs['human'] > probs['vehicle']:
          cv2.rectangle(overlay, a, b, blue, thickness = 1)
        else:
          cv2.rectangle(overlay, a, b, red, thickness = 1)

    return overlay

  def score(self):
    if self.hasImg and self.contourArcLength!= 0:
      return self.contourArea*1.0/(self.numPixels*self.contourArcLength)
    else:
      return 0

  def image(self):
    return self.img

  def overlaidImage(self):
    return self.overlaid

  def foreground(self):
    return self.foremask

  def background(self):
    return self.backgroundMOG.getBackgroundImage()

  def patches(self, squareSize = 0, num = float('Inf')):
    num = len(self.bb) if num == 0 else min(num, len(self.bb))
    queryBB = self.bb[:num]

    bbPatches = [self.img[rect[1]:rect[1]+rect[3],rect[0]:rect[0]+rect[2]] for rect in queryBB]
    if squareSize > 0:
      bbPatches = map(lambda x : cv2.resize(x,(squareSize,squareSize)),bbPatches)

    return bbPatches



