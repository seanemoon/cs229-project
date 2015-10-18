import metadata.manager
import metadata.scraper.opentopia
import webcam

import logging


def main():
  logging.basicConfig(level=logging.DEBUG)
  with metadata.manager.Manager() as metadata_manager:
    metadata_manager.set_scraper(metadata.scraper.opentopia.Scraper)
    mdata = metadata_manager.get('11008')
    cam = webcam.Webcam(mdata)
    cam.fetch_current_frame()
    for frame in cam.sorted_frames():
      print(frame)


if __name__ == "__main__":
  main()
