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


classifier = CamClassifier()
classifier.set_batch_size(1)

while True:
  for path in wc_paths:
    cam = Webcam(online=False, path=path)
    for i in range(5):
      cam.update()
    overlay = cam.filtered_overlay(classifier, squareSize=227)
    if overlay is not None:
      cv2.imshow('overlay', overlay)
      cv2.waitKey(0)
