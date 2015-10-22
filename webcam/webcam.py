import datetime
import glob
import logging
import os
import socket
import urllib
import urllib.error
import urllib.request


class Webcam(object):
  """Manages webcams.

  Supports fetching the current frame and iterating over stored frames.

  Usage Example:
    metadata_manager = metadata.Manager()
    metadata = metadata_manager.get('opentopia', '11008')
    webcam = Webcam(metadata)

    while True:
      webcam.fetch_current_frame()
      time.sleep(60)

    for frame in webcam.frames():
      use(frame)
  """
  def __init__(self, metadata):
    """Initializes a Webcam object.

    Args:
      metadata (metadata.scraper.Metadata): The metadata which uniquely
          identifies the webcam.
    """
    self._metadata = metadata
    self._logger = logging.getLogger('webcam.webcam.Webcam')


  def _frame_directory(self):
    """The directory in which frames are stored.

    Returns:
      string: The directory in which frames are stored.
    """
    return "%s/frames/%s_%08d/" % (os.path.dirname(os.path.realpath(__file__)),
        self._metadata.source, int(self._metadata.identifier))


  def fetch_current_frame(self, timeout=10):
    """Fetches the current frame from the webcam.

    Constructs and sends an HTTP request to the webcam. Saves the response to
    self._frame_directory().

    Args:
      timeout (int, default 10): The maximum time to block on a connection.

    Returns:
      bool: True if and only if a frame was succesfully stored.
    """
    if not self._metadata.livestill_url:
      self._logger.error('No webcam URL found for (%s, %s).' %
          (self._metadata.source, self._metadata.identifier))
      return False

    try:
      response = urllib.request.urlopen(self._metadata.livestill_url,
          timeout=timeout)
    except (urllib.error.URLError, socket.timeout) as error:
      self._logger.error('Failed to fetch current frame for (%s, %s).' %
          (self._metadata.source, self._metadata.identifier))
      self._logger.error(error)
      return False

    now = datetime.datetime.today()
    filename = "%04d_%02d_%02d_%02d_%02d_%02d" % (now.year, now.month, now.day,
        now.hour, now.minute, now.second)
    filepath = "%s%s.jpg" % (self._frame_directory(), filename)
    try:
      os.makedirs(os.path.dirname(filepath), exist_ok=True)
      with open(filepath, 'wb') as f:
        self._logger.info('Succesfully saved frame for %s from %s.' %
            (self._metadata.identifier, self._metadata.source))
        f.write(response.read())
      return True
    except IOError as error:
      self._logger.error('failed to save current frame for (%s, %s).' %
          (self._metadata.source, self._metadata.identifier))
      self._logger.error(error)
      return False


  def frames(self):
    """An unsorted iterator for the filepaths for all the frames.

    Yields:
      string: The filename for the next frame chronologically.
    """
    for filename in glob.glob("%s*.jpg" % self._frame_directory()):
      yield filename


  def sorted_frames(self):
    """A sorted iterator for the filepaths for all the frames.

    Yields:
      string: The filename for the next frame chronologically.
    """
    print("%s*.jpg", self._frame_directory())
    for filename in sorted(glob.glob("%s*.jpg" % self._frame_directory())):
      yield filename


  def is_live(self):
    """Indicates whether we can get live frames from this webcam.

    Returns:
      bool: True if and only if, to the best of our knowledge, we can get live
          frames from this webcam.
    """
    return hasattr(self._metadata, 'is_live') and self._metadata.is_live


  def identifier(self):
    """The identifier for this webcam.

    Returns:
      str: The unique identifier for this webcam.
    """
    return self._metadata.identifier
