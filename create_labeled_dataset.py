from detector.Webcam import Webcam
import os
import cv2
import sys

frames_dir = 'webcam/frames'
width = 224

wc_paths = [os.path.join(frames_dir, fn) for fn in os.listdir(frames_dir)]


QUIT = 0
ANIMAL = 1
NOISE = 2
HUMAN = 3
VEHICLE = 4
NEXT = 5

KEYS = {
    32: 'NEXT',
    113: 'QUIT',
    104: 'ANIMAL',
    106: 'NOISE',
    107: 'HUMAN',
    108: 'VEHICLE'
}

LABELS = {'ANIMAL', 'NOISE', 'HUMAN', 'VEHICLE'}

patch_count = 9780
def save_patch(patch, label):
  global patch_count
  cv2.imwrite(os.path.join('database', label.lower(), str(patch_count) + '.jpg'), patch)
  print patch_count
  patch_count += 1


def process_cam(cam):
  while True:
    cam.update()
    patches = cam.patches(squareSize=width)
    for patch in patches:
      if patch is not None:
        cv2.imshow('patch', patch)
        k = cv2.waitKey(0) & 0xFF
        if k in KEYS:
          command = KEYS[k]
          if command == 'QUIT':
            sys.exit(0)
          elif command == 'NEXT':
            print "PROCESSING NEXT WEBCAM"
            return
          elif command in LABELS:
            print command
            save_patch(patch, command)
          else:
            print 'Unrecognized command'
            return
        else:
          print 'Frame unlabeled.'


webcam_no = 241
wc_paths = sorted(wc_paths)
wc_paths = wc_paths[webcam_no:]
for wc_path in sorted(wc_paths):
  cam = Webcam(online=False, resize=(width, width), path=wc_path)
  cam.update()
  process_cam(cam)
  print "WEBCAM_NO", webcam_no
  webcam_no += 1
