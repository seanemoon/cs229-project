class WebcamMetadataManager(object):
  """Manages webcam metadata.

  Supports iteratation on persisted metadata.

  Also supports querying and automatic persistance of metadata from sources
  which implement AbstractWebcamMetadataScraper.

  Usage Example:
    metadata_manager = WebcamMetadataManager(database_file)

    for webcam_metadata in metadata_manager:
      ...

    metadata_manager.set_scraper(WebcamMetadataScraper)
    metadata = metadata_manager.get('11232')
  """


  def __init__(self, database_file):
    """Initializes a WebcamMetadataManager object

    Args:
      database_file (string): The path to the database file backing this manager.
    """
    self._database_file = database_file
    # TODO(seanrafferty): rehydrate this from the database.
    self._metadata = {}


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
        self._add(metadata)
        return metadata
    else:
      return None


  def set_scraper(self, scraper):
    """Sets the current scraper for the manager.

    The scraper is only used when querying for metadata.

    Args:
      scraper (WebcamMetadataScraper): The scraper to use.
    """
    self._scraper = scraper


  def _add(self, metadata):
    """Adds the metadata to the manager.

    Args:
      metadata (WebcamMetadata): The metadata to add.
    """
    key = (metadata.source, metadata.identifier)
    self._metadata[key] = metadata
    # TODO(seanrafferty): Persist.
