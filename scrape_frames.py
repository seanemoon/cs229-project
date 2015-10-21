#! /bin/python3.5

import webcam.metadata.manager
import webcam.webcam

import argparse
import logging
import time


def scrape(source, identifier, frequency):
  """Scrapes frames from the webcam.

  Args:
    source (string): The source of the webcam.
    identifier (string): A unique identifier for the webcam.
    frequency (float): The number of frames to scrape per second.
  """
  try:
    scrapers = "webcam.metadata.scraper"
    scraper_module = __import__("%s.%s" % (scrapers, source), fromlist=[scrapers])
  except ImportError:
    print("Could not import scraper %s." % source)
    return
  with webcam.metadata.manager.Manager() as manager:
    manager.set_scraper(scraper_module.Scraper)
    cam = webcam.webcam.Webcam(manager.get(identifier, source))
    while cam.fetch_current_frame():
      time.sleep(1.0 / frequency)


def main():
  """Scrapes frames from the webcam.

  Usage Example:
    scrape_frames.py --source=opentopia --identifier=11008 --frequency=0.017
  """
  parser = argparse.ArgumentParser(prog='scrape_frames')
  parser.add_argument('-s', '--source', nargs=1, required=True,
      help='The name of the source of the webcam.')
  parser.add_argument('-i', '--identifier', nargs=1, required=True,
      help='An identifier that uniquely identifies the webcam.')
  parser.add_argument('-f', '--frequency', nargs=1, required=False,
      default=[str(1.0/60.0)], help='The frequency to scrape frames (in Hertz).')
  args = parser.parse_args()

  logging.basicConfig(level=logging.INFO)
  scrape(args.source[0], args.identifier[0], float(args.frequency[0]))


if __name__ == "__main__":
  main()
