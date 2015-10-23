#! /bin/python3.5

import webcam.metadata.manager
import webcam.webcam

import argparse
import collections
import datetime
import logging
import queue
import time
import threading


def import_scraper_module(source):
  """Imports the scraper module.

  Args:
    source (str): The source to scrape from. This is expected to be the name of
        a scraper module in webcam.metadata.scraper.

  Returns:
    webcam.metadata.scraper.source: The scraper module. Returns None if no such
        module exists.
  """
  try:
    scrapers = "webcam.metadata.scraper"
    scraper_module = __import__("%s.%s" % (scrapers, source), fromlist=[scrapers])
  except ImportError:
    print("Could not import scraper %s." % source)
    return None
  return scraper_module


def scraper_thread_fn(webcam_queue, duration):
  """Main function for a scraper (consumer) thread.

  A `scraper` is a consumer of the `webcam_queue`. While there is work to be
  done, (i.e. there are webcams in the queue), this thread will dequeue a
  webcam, fetch a frame from that webcam, persist the fetched frame, and repeat
  with a new webcam.

  This thread blocks when there are no webcams to be consumed.

  This thread terminates after `duration`.

  Args:
    webcam_queue (queue.Queue (webcam.webcam.Webcam)): The queue of webcams
        that must be scraped. This queue is shared between the one dispatcher
        and the many scrapers.
    duration (datetime.timedelta): The duration to scrape for.
  """
  time_started = datetime.datetime.now()
  failures = collections.defaultdict(int)
  attempts = collections.defaultdict(int)
  while datetime.datetime.now() < time_started + duration:
    cam = webcam_queue.get(block=True, timeout=None)
    cam.fetch_current_frame()


def dispatcher_thread_fn(webcams, webcam_queue, period, duration):
  """Main function for the dispatcher (producer) thread.

  A `dispatcher` is a producer for the `webcam_queue`. Periodically, with
  period `period`, this thread enqueues every webcam in `webcams` onto
  `webcam_queue`. To remove a webcam from the queue, a consumer must attempt to
  fetch and persist the latest frame from the webcam.

  This thread blocks until `period` has passed since the last time it has
  produced for the queue.

  This thread terminates after `duration`.

  Args:
    webcams (list (webcam.webcam.Webcam)): A list of live webcams to scrape
        frames from.
    webcam_queue (queue.Queue (webcam.webcam.Webcam)): The queue of webcams
        that must be scraped. This queue is shared between the one dispatcher
        and the many scrapers.
    period (datetime.timedelta): The duration to wait between frames.
    duration (datetime.timedelta): The duration to scrape for.
  """
  logger = logging.getLogger('main.dispatcher')
  logger.info("Scraping frames for %d webcams.", len(webcams))
  time_started = datetime.datetime.now()
  while datetime.datetime.now() < time_started + duration:
    time_last_scraped = datetime.datetime.now()
    logger.info("Dispatching scrape requests.")
    if not webcam_queue.empty():
      logger.error("Taking too long to consume scrape requests.")
    for cam in webcams:
      webcam_queue.put(cam, block=True, timeout=None)
    elapsed = datetime.datetime.now() - time_last_scraped
    remaining = period - elapsed
    if remaining.seconds > 0:
      time.sleep(remaining.seconds)
    else:
      logger.error("Taking too long to dispatch scrape requests.")


def scrape_frames(source, identifiers, period, duration, num_scrapers):
  """Scrapes frames in parallel.

  Args:
    source (str): The source to scrape from.
    identifiers (list (str)): A list of identifiers which uniquely identify a
        webcam for the source.
    period (datetime.timedelta): The period  between frames.
    duration (datetime.timedelta): The duration to scrape for.
    num_scrapers (int): The number of scraper threads.
  """
  # Setup the manager.
  manager = webcam.metadata.manager.Manager()

  # Populate a list of live webcams to scrape.
  webcams = []
  for identifier in identifiers:
    cam = webcam.webcam.Webcam(manager.get(identifier, source))
    if cam.is_live():
      webcams.append(cam)

  # Delegate the work to a dispatcher (producer) and scrapers (consumers).
  webcam_queue = queue.Queue()
  dispatcher = threading.Thread(target=dispatcher_thread_fn, args=(webcams,
      webcam_queue, period, duration))
  scrapers = []
  for i in range(num_scrapers):
    scrapers.append(threading.Thread(target=scraper_thread_fn,
        args=(webcam_queue, duration)))

  # Start the threads.
  dispatcher.start()
  for scraper in scrapers:
    scraper.start()

  # Wait for the threads to finish.
  dispatcher.join()
  for scraper in scrapers:
    scraper.join()


def main():
  """Scrapes frames from the webcam.

  Usage Example:
    scrape_frames.py --source=opentopia --identifiers=1:17000
  """
  # Retrieve command-line arguments.
  parser = argparse.ArgumentParser(prog='scrape_frames')
  parser.add_argument('-s', '--source', nargs=1, required=False,
      default=["opentopia"], help='The name of the source of the webcam.')
  parser.add_argument('-i', '--identifiers', nargs=1, required=False,
    default=["1:17000"],
        help='A colon separated range of the form [START:END].')
  parser.add_argument('-f', '--period', nargs=1, required=False,
      default=["5"], help='The period between frames (in Minutes).')
  parser.add_argument('-d', '--duration', nargs=1, required=False,
      default=["7"], help='The duration to scrape for (in Days).')
  parser.add_argument('-t', '--threads', nargs=1, required=False,
      default=["100"], help='The number of scraping threads.')
  args = parser.parse_args()

  # Parse command-line arguments.
  source = args.source[0]
  identifiers = args.identifiers[0].split(':')
  identifiers = [str(i) for i in range(int(identifiers[0]),
      int(identifiers[1]))]
  period = datetime.timedelta(minutes=int(args.period[0]))
  duration = datetime.timedelta(days=int(args.duration[0]))
  num_threads = int(args.threads[0])

  # Set up logging.
  logging.basicConfig(filename='scrape_frames.log', filemode='a',
      format='%(asctime)s,%(msecs)03d %(name)s %(levelname)s %(message)s',
          datefmt='%H:%M:%S', level=logging.INFO)

  # Scrape frames.
  scrape_frames(source, identifiers, period, duration, num_threads)


if __name__ == "__main__":
  main()
