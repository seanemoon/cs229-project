#! /bin/python3.5

import argparse
import logging

import webcam.metadata.manager


def list_metadata(must_be_live):
  """Lists metadata that we have persisted locally.

  Args:
    must_be_live (bool): When true, the metadata we print must contain live
        links which we can get frames from.
  """
  manager = webcam.metadata.manager.Manager()
  if must_be_live:
    metadata = manager.get_live_webcam_metadata()
  else:
    metadata = manager.get_webcam_metadata()
  for m in sorted(metadata, key=lambda m: int(m.identifier)):
    if m.is_live:
      live_text = "LIVE"
    else:
      live_text = "down"
    print("%s %08d (%s): %s" % (m.source, int(m.identifier), live_text,
      m.livestill_url))


def main():
  """Lists metadata that we have persisted locally.

  Usage Example:
    list_metadata.py --live
  """
  parser = argparse.ArgumentParser(prog='list_metadata')
  parser.add_argument('-l', '--live', action='store_true', required=False,
      help='Whether you require the webcams to be live.')
  args = parser.parse_args()

  logging.basicConfig(level=logging.INFO)
  list_metadata(args.live)


if __name__ == "__main__":
  main()
