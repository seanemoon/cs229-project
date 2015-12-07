import numpy as np
import cv2
import os
import random
import argparse
import time

from detector.Webcam import Webcam
from classify.classifier import ReferenceClassifier
from classify.classifier import CamClassifier

parser = argparse.ArgumentParser()
parser.add_argument("frames_dir",
    help = "Directory containing webcam image folders")
parser.add_argument("num_webcams",
    help = "Number of webcams to sample", type = int)
args = parser.parse_args()

frames_dir = args.frames_dir
num_webcams = args.num_webcams

wc_paths = [os.path.join(frames_dir, fn) for fn in os.listdir(frames_dir)]
wc_paths = random.sample(wc_paths, num_webcams)
# cams  = [Webcam(online=False, path=p) for p in wc_paths]

def classify_one(classifier, patches):
  random.shuffle(patches)
  for patch in patches:
    if patch is not None:
      print patch.shape
      #classification = classifier.classify(patch)
      cv2.imshow('patch', patch)
      #print classifier.label(classification)
      classifier.rankings(patch)
      cv2.waitKey(0)
      return

classifier = CamClassifier()
classifier.set_batch_size(1)

webcam = Webcam(online = True,
    path = "http://vso.aa0.netvolante.jp/record/current.jpg")

K_HUMAN_THRESH = 0.0002

def find_human_in_cam(classifier, cam):
  patches = webcam.patches(squareSize=227)
  random.shuffle(patches)
  for patch in patches:
    if patch is not None:
      rankings = classifier.rankings(patch)
      for ranking in rankings:
        label, idx, prob = ranking
        if label != 'noise' and prob > 0.5:
          if prob > K_HUMAN_THRESH:
            print rankings
            cv2.imshow('patch', patch)
            cv2.waitKey(0)
            return

def find_human(classifier, cams):
  for i, cam in enumerate(cams):
    print i
    find_human_in_cam(classifier, cam)

for i in range(5):
  # map(lambda c: c.update(), cams)
  webcam.update()

def find_interesting_object(classifier, wc_paths):
  for i, path in enumerate(wc_paths):
    print i
    cam = Webcam(online=False, path=path)
    for i in range(10):
      cam.update()
    for patch in cam.patches(squareSize=227):
      if patch is not None:
        rankings = classifier.rankings(patch)
        for label, idx, prob in rankings:
          if label == 'human' and prob > 0.015:
            print rankings
            cv2.imshow('patch', patch)
            cv2.waitKey(0)

while True:
  find_interesting_object(classifier, wc_paths)

while True:
  # cams.sort(key = lambda c: c.score(), reverse = True)
  #webcam.update()
  #patches = webcam.patches(squareSize=227)
  #classify_one(classifier, patches)

  # patches_list = [c.patches(img_w, 5) for c in cams]
  map(lambda c: c.update(), cams)
  find_human(classifier, cams)



