import logging
import os
import pickle


class Manager(object):
  """Manages webcam metadata.

  Supports iteratation on persisted metadata.

  Also supports querying and automatic persistance of metadata from sources
  which implement scraper.abstract.AbstractScraper.

  Usage Example:
    metadata_manager = webcam.metadata.Manager(database_file)

    for webcam_metadata in metadata_manager:
      use(webcam_metadata)

    metadata_manager.set_scraper(scraper.opentopia.Scraper)
    metadata = metadata_manager.get('11232')
  """
  def __init__(self, metadata_path = None):
    """Initializes a Manager object.

    Args:
      metadata_path (string, optional): The path to the pickled metadata.
    """
    self._metadata_path = metadata_path or Manager._default_metadata_path()
    self._logger = logging.getLogger('webcam.manager.Manager')
    try:
      with open(self._metadata_path, 'rb') as f:
        try:
          self._metadata = pickle.load(f)
        except Exception as error:
          self._logger.fatal(
              'Error unpickling metadata. Please verify that %s is valid.' %
                  self._metadata_path)
          raise error
    except FileNotFoundError:
      self._metadata = {}
    except OSError as error:
      self._logger.fatal(
          'Error loading metadata. Please verify that %s is valid.' %
              self._metadata_path)
      raise error
    self._metadata_is_dirty = False


  def __enter__(self):
    """Called at the beginning of a with block."""
    return self


  def __exit__(self, type, value, traceback):
    """Called at the end of a with block."""
    self.persist_changes()


  @staticmethod
  def _default_metadata_path():
    """Returns the default metadata path.

    Returns:
      string: The default metadata path.
    """
    return "%s/%s" % (os.path.dirname(os.path.realpath(__file__)),
        "metadata.p")


  def persist_changes(self):
    """Persists the metadata changes.

    Pickles the metadata and saves it to the metadata path.
    """
    if self._metadata_is_dirty:
      try:
        with open(self._metadata_path, 'wb') as f:
          self._logger.info('Persisting metadata changes.')
          pickle.dump(self._metadata, f)
          self._metadata_is_dirty = False
      except OSError as error:
        self._logger.error('failed to persist webcam metadata.')
        self._logger.error(error)


  def get(self, identifier, source=None):
    """Gets metadata for a webcam.

    If neither as source is passed or a scraper is set, then this returns None.

    If a source is passed but no scraper is set, then the manager will return
    the stored metadata if it exists, and None otherwise.

    If a scraper is set, then the manager will first see if the metadata is
    persisted using the passed source, or the scraper's source if no source is
    passed. Otherwise, it will use the scraper to retrieve the metadata.

    Args:
      identifier (string): The unique identifier for the webcam for the given
          source or current scraper.
      source (string, optional): The source of webcam metadata.

    Returns:
      WebcamMetadata: The webcam's metadata.
    """
    if source or self._scraper:
      key = (source or self._scraper.source(), identifier)
      if key in self._metadata:
        return self._metadata[key]
      else:
        metadata = self._scraper.scrape(identifier)
        self._logger.info('Scraping %s from %s.' % (identifier, self._scraper.source()))
        self._add(metadata)
        return metadata
    else:
      return None


  def set_scraper(self, scraper):
    """Sets the current scraper for the manager.

    The scraper is only used when querying for metadata.

    Args:
      scraper (scraper.abstract.Scraper): The scraper to use.
    """
    self._scraper = scraper


  def get_webcam_metadata():
    """Returns a list of all known webcam metadata.

    Returns:
      list (scraper.metadata.Metadata): A list of all known webcam metadata.
    """
    return [metdata for key, metadata in self._metadata.iteritems()]


  def get_live_webcam_metadata(self):
    """Returns a list of all know live webcams.

    Returns:
      list (scraper.metadata.Metadata): A list of all known webcam metadata
          where metadata.is_live is True.
    """
    webcam_metadata = []
    for key, metadata in self._metadata.iteritems():
      if hasattr(metadata, 'is_live') and metadata.is_live
        webcam_metadata.append(metadata)
    return webcam_metadata


  def _add(self, metadata):
    """Adds the metadata to the manager.

    Args:
      metadata (scraper.metadata.Metadata): The metadata to add.
    """
    key = (metadata.source, metadata.identifier)
    self._metadata[key] = metadata
    self._metadata_is_dirty = True
