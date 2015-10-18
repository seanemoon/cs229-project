import webcam.metadata.manager

import argparse
import logging
import signal
import sys
import time


def scrape(source, identifiers):
  """Scrape webcam metadata.

  Args:
    scraper (webcam.metadata.scraper.Abstract): The scraper to use.
    identifiers (list (string)): A list of identifiers uniquely identifying
        webcams to scrape.
  """
  try:
    scrapers = "webcam.metadata.scraper"
    scraper_module = __import__("%s.%s" % (scrapers, source), fromlist=[scrapers])
  except ImportError:
    print("Could not import scraper %s." % source)
    return
  with webcam.metadata.manager.Manager() as manager:
    manager.set_scraper(scraper_module.Scraper)
    for identifier in identifiers:
      manager.get(identifier)
      time.sleep(1)


def main():
  """Scrapes metadata from the source.

  Usage Example:
    scrape_metadata.py --source=opentopia --identifiers=1:17000
  """
  parser = argparse.ArgumentParser(prog='scrape_metadata')
  parser.add_argument('-s', '--source', nargs=1, required=True,
      help='The name of the source to scrape from.')
  parser.add_argument('-i', '--identifiers', nargs=1, required=True,
      help='A colon separated range of the form [START:END].')
  args = parser.parse_args()

  source = args.source[0]
  identifiers = args.identifiers[0].split(':')
  identifiers = [str(i) for i in range(int(identifiers[0]),
      int(identifiers[1]))]

  logging.basicConfig(level=logging.INFO)
  scrape(source, identifiers)


if __name__ == "__main__":
  main()
